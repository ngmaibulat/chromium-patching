"use strict";
import { compare } from "./StringUtilities.js";
const defaultWarningMessages = {
  deprecratedSyntax() {
    return "Deprecated syntax found. Please use: <name>;dur=<duration>;desc=<description>";
  },
  duplicateParameter(parameter) {
    return `Duplicate parameter "${parameter}" ignored.`;
  },
  noValueFoundForParameter(parameter) {
    return `No value found for parameter "${parameter}".`;
  },
  unrecognizedParameter(parameter) {
    return `Unrecognized parameter "${parameter}".`;
  },
  extraneousTrailingCharacters() {
    return "Extraneous trailing characters.";
  },
  unableToParseValue(parameter, value) {
    return `Unable to parse "${parameter}" value "${value}".`;
  }
};
export class ServerTiming {
  metric;
  value;
  description;
  start;
  constructor(metric, value, description, start) {
    this.metric = metric;
    this.value = value;
    this.description = description;
    this.start = start;
  }
  static parseHeaders(headers, warningMessages = defaultWarningMessages) {
    const rawServerTimingHeaders = headers.filter((item) => item.name.toLowerCase() === "server-timing");
    if (!rawServerTimingHeaders.length) {
      return null;
    }
    const serverTimings = rawServerTimingHeaders.reduce((memo, header) => {
      const timing = this.createFromHeaderValue(header.value, warningMessages);
      memo.push(...timing.map(function(entry) {
        return new ServerTiming(
          entry.name,
          entry.hasOwnProperty("dur") ? entry.dur : null,
          entry.hasOwnProperty("desc") ? entry.desc : "",
          entry.hasOwnProperty("start") ? entry.start : null
        );
      }));
      return memo;
    }, []);
    serverTimings.sort((a, b) => compare(a.metric.toLowerCase(), b.metric.toLowerCase()));
    return serverTimings;
  }
  /**
   * TODO(crbug.com/1011811): Instead of using !Object<string, *> we should have a proper type
   *                          with #name, desc and dur properties.
   */
  static createFromHeaderValue(valueString, warningMessages = defaultWarningMessages) {
    function trimLeadingWhiteSpace() {
      valueString = valueString.replace(/^\s*/, "");
    }
    function consumeDelimiter(char) {
      console.assert(char.length === 1);
      trimLeadingWhiteSpace();
      if (valueString.charAt(0) !== char) {
        return false;
      }
      valueString = valueString.substring(1);
      return true;
    }
    function consumeToken() {
      const result2 = /^(?:\s*)([\w!#$%&'*+\-.^`|~]+)(?:\s*)(.*)/.exec(valueString);
      if (!result2) {
        return null;
      }
      valueString = result2[2];
      return result2[1];
    }
    function consumeTokenOrQuotedString() {
      trimLeadingWhiteSpace();
      if (valueString.charAt(0) === '"') {
        return consumeQuotedString();
      }
      return consumeToken();
    }
    function consumeQuotedString() {
      console.assert(valueString.charAt(0) === '"');
      valueString = valueString.substring(1);
      let value = "";
      while (valueString.length) {
        const result2 = /^([^"\\]*)(.*)/.exec(valueString);
        if (!result2) {
          return null;
        }
        value += result2[1];
        if (result2[2].charAt(0) === '"') {
          valueString = result2[2].substring(1);
          return value;
        }
        console.assert(result2[2].charAt(0) === "\\");
        value += result2[2].charAt(1);
        valueString = result2[2].substring(2);
      }
      return null;
    }
    function consumeExtraneous() {
      const result2 = /([,;].*)/.exec(valueString);
      if (result2) {
        valueString = result2[1];
      }
    }
    const result = [];
    let name;
    while ((name = consumeToken()) !== null) {
      const entry = { name };
      if (valueString.charAt(0) === "=") {
        this.showWarning(warningMessages["deprecratedSyntax"]());
      }
      while (consumeDelimiter(";")) {
        let paramName;
        if ((paramName = consumeToken()) === null) {
          continue;
        }
        paramName = paramName.toLowerCase();
        const parseParameter = this.getParserForParameter(paramName, warningMessages);
        let paramValue = null;
        if (consumeDelimiter("=")) {
          paramValue = consumeTokenOrQuotedString();
          consumeExtraneous();
        }
        if (parseParameter) {
          if (entry.hasOwnProperty(paramName)) {
            this.showWarning(warningMessages["duplicateParameter"](paramName));
            continue;
          }
          if (paramValue === null) {
            this.showWarning(warningMessages["noValueFoundForParameter"](paramName));
          }
          parseParameter.call(this, entry, paramValue);
        } else {
          this.showWarning(warningMessages["unrecognizedParameter"](paramName));
        }
      }
      result.push(entry);
      if (!consumeDelimiter(",")) {
        break;
      }
    }
    if (valueString.length) {
      this.showWarning(warningMessages["extraneousTrailingCharacters"]());
    }
    return result;
  }
  static getParserForParameter(paramName, warningMessages) {
    switch (paramName) {
      case "dur": {
        let durParser2 = function(entry, paramValue) {
          entry.dur = 0;
          if (paramValue !== null) {
            const duration = parseFloat(paramValue);
            if (isNaN(duration)) {
              ServerTiming.showWarning(warningMessages["unableToParseValue"](paramName, paramValue));
              return;
            }
            entry.dur = duration;
          }
        };
        var durParser = durParser2;
        return durParser2;
      }
      case "start": {
        let startParser2 = function(entry, paramValue) {
          entry.start = null;
          if (paramValue !== null) {
            const start = parseFloat(paramValue);
            if (isNaN(start)) {
              ServerTiming.showWarning(warningMessages["unableToParseValue"](paramName, paramValue));
              return;
            }
            entry.start = start;
          }
        };
        var startParser = startParser2;
        return startParser2;
      }
      case "desc": {
        let descParser2 = function(entry, paramValue) {
          entry.desc = paramValue || "";
        };
        var descParser = descParser2;
        return descParser2;
      }
      default: {
        return null;
      }
    }
  }
  static showWarning(msg) {
    console.warn(`ServerTiming: ${msg}`);
  }
}
//# sourceMappingURL=ServerTiming.js.map
