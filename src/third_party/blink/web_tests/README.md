The documentation for this directory is at:
- [Web Tests](/docs/testing/web_tests.md)
- [Writing Web Tests](/docs/testing/writing_web_tests.md)

## WPT Configuration

There are some special files under this directory for adapting [web platform
tests](https://chromium.googlesource.com/chromium/src/+/HEAD/docs/testing/web_platform_tests.md)
for Blink:
* `web_tests/external/wpt/.config.json`: `wptserve` configuration file for
  overriding default routes and ports. Note that filesystem paths are relative
  to `blink/web_tests` and will be changed to absolute path at run time for
  `wptserve` to consume. When changing the ports (HTTP/S, WS/S), make sure to also:
  * Update `WPT_HOST_AND_PORTS` in
    `//third_party/blink/tools/blinkpy/web_tests/port/driver.py`
  * Update `WebTestContentBrowserClient::GetOriginsRequiringDedicatedProcess`
