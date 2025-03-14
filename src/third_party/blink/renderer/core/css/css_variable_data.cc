// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/css/css_variable_data.h"

#include <algorithm>

#include "base/compiler_specific.h"
#include "third_party/blink/renderer/core/css/css_syntax_definition.h"
#include "third_party/blink/renderer/core/css/parser/css_parser_context.h"
#include "third_party/blink/renderer/core/css/parser/css_parser_token_stream.h"
#include "third_party/blink/renderer/core/css/parser/css_tokenizer.h"
#include "third_party/blink/renderer/core/css/properties/css_parsing_utils.h"
#include "third_party/blink/renderer/core/html/parser/input_stream_preprocessor.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"
#include "third_party/blink/renderer/platform/wtf/text/string_builder.h"
#include "third_party/blink/renderer/platform/wtf/text/string_view.h"

namespace blink {

static bool IsFontUnitToken(CSSParserToken token) {
  if (token.GetType() != kDimensionToken) {
    return false;
  }
  switch (token.GetUnitType()) {
    case CSSPrimitiveValue::UnitType::kEms:
    case CSSPrimitiveValue::UnitType::kChs:
    case CSSPrimitiveValue::UnitType::kExs:
    case CSSPrimitiveValue::UnitType::kIcs:
    case CSSPrimitiveValue::UnitType::kCaps:
      return true;
    default:
      return false;
  }
}

static bool IsRootFontUnitToken(CSSParserToken token) {
  if (token.GetType() != kDimensionToken) {
    return false;
  }
  switch (token.GetUnitType()) {
    case CSSPrimitiveValue::UnitType::kRems:
    case CSSPrimitiveValue::UnitType::kRexs:
    case CSSPrimitiveValue::UnitType::kRchs:
    case CSSPrimitiveValue::UnitType::kRics:
    case CSSPrimitiveValue::UnitType::kRlhs:
    case CSSPrimitiveValue::UnitType::kRcaps:
      return true;
    default:
      return false;
  }
}

static bool IsLineHeightUnitToken(CSSParserToken token) {
  return token.GetType() == kDimensionToken &&
         token.GetUnitType() == CSSPrimitiveValue::UnitType::kLhs;
}

void CSSVariableData::ExtractFeatures(const CSSParserToken& token,
                                      bool& has_font_units,
                                      bool& has_root_font_units,
                                      bool& has_line_height_units,
                                      bool& has_dashed_functions) {
  has_font_units |= IsFontUnitToken(token);
  has_root_font_units |= IsRootFontUnitToken(token);
  has_line_height_units |= IsLineHeightUnitToken(token);
  has_dashed_functions |= css_parsing_utils::IsDashedFunctionName(token);
}

CSSVariableData* CSSVariableData::Create(const String& original_text,
                                         bool is_animation_tainted,
                                         bool is_attr_tainted,
                                         bool needs_variable_resolution) {
  bool has_font_units = false;
  bool has_root_font_units = false;
  bool has_line_height_units = false;
  bool has_dashed_functions = false;
  CSSParserTokenStream stream(original_text);
  while (!stream.AtEnd()) {
    ExtractFeatures(stream.ConsumeRaw(), has_font_units, has_root_font_units,
                    has_line_height_units, has_dashed_functions);
  }
  return Create(original_text, is_animation_tainted, is_attr_tainted,
                needs_variable_resolution, has_font_units, has_root_font_units,
                has_line_height_units, has_dashed_functions);
}

String CSSVariableData::Serialize() const {
  if (length_ > 0 && OriginalText()[length_ - 1] == '\\') {
    // https://drafts.csswg.org/css-syntax/#consume-escaped-code-point
    // '\' followed by EOF is consumed as U+FFFD.
    // https://drafts.csswg.org/css-syntax/#consume-string-token
    // '\' followed by EOF in a string token is ignored.
    //
    // The tokenizer handles both of these cases when returning tokens, but
    // since we're working with the original string, we need to deal with them
    // ourselves.
    StringBuilder serialized_text;
    serialized_text.Append(OriginalText());
    serialized_text.Resize(serialized_text.length() - 1);

    CSSParserTokenStream stream(OriginalText());
    CSSParserTokenType last_token_type = kEOFToken;
    for (;;) {
      CSSParserTokenType token_type = stream.ConsumeRaw().GetType();
      if (token_type == kEOFToken) {
        break;
      }
      last_token_type = token_type;
    }

    if (last_token_type != kStringToken) {
      serialized_text.Append(kReplacementCharacter);
    }

    // Certain token types implicitly include terminators when serialized.
    // https://drafts.csswg.org/cssom/#common-serializing-idioms
    if (last_token_type == kStringToken) {
      serialized_text.Append('"');
    }
    if (last_token_type == kUrlToken) {
      serialized_text.Append(')');
    }

    return serialized_text.ReleaseString();
  }

  return OriginalText().ToString();
}

bool CSSVariableData::EqualsIgnoringAttrTainting(
    const CSSVariableData& other) const {
  return OriginalText() == other.OriginalText();
}

bool CSSVariableData::operator==(const CSSVariableData& other) const {
  return OriginalText() == other.OriginalText() &&
         IsAttrTainted() == other.IsAttrTainted();
}

CSSVariableData::CSSVariableData(PassKey,
                                 StringView original_text,
                                 bool is_animation_tainted,
                                 bool is_attr_tainted,
                                 bool needs_variable_resolution,
                                 bool has_font_units,
                                 bool has_root_font_units,
                                 bool has_line_height_units,
                                 bool has_dashed_functions)
    : length_(original_text.length()),
      is_animation_tainted_(is_animation_tainted),
      is_attr_tainted_(is_attr_tainted),
      needs_variable_resolution_(needs_variable_resolution),
      is_8bit_(original_text.Is8Bit()),
      has_font_units_(has_font_units),
      has_root_font_units_(has_root_font_units),
      has_line_height_units_(has_line_height_units),
      has_dashed_functions_(has_dashed_functions) {
  if (is_8bit_) {
    std::ranges::copy(original_text.Span8(),
                      UNSAFE_TODO(reinterpret_cast<LChar*>(this + 1)));
  } else {
    std::ranges::copy(original_text.Span16(),
                      UNSAFE_TODO(reinterpret_cast<UChar*>(this + 1)));
  }
}

const CSSValue* CSSVariableData::ParseForSyntax(
    const CSSSyntaxDefinition& syntax,
    SecureContextMode secure_context_mode) const {
  DCHECK(!NeedsVariableResolution());
  // TODO(timloh): This probably needs a proper parser context for
  // relative URL resolution.
  return syntax.Parse(OriginalText(),
                      *StrictCSSParserContext(secure_context_mode),
                      is_animation_tainted_, is_attr_tainted_);
}

}  // namespace blink
