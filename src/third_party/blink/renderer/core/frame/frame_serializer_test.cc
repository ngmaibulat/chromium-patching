/*
 * Copyright (c) 2013, Opera Software ASA. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Opera Software ASA nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "third_party/blink/renderer/core/frame/frame_serializer.h"

#include <string>

#include "base/run_loop.h"
#include "base/test/bind.h"
#include "base/test/scoped_feature_list.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/common/features.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/platform/web_url.h"
#include "third_party/blink/public/platform/web_url_request.h"
#include "third_party/blink/public/platform/web_url_response.h"
#include "third_party/blink/public/web/web_settings.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_binding_for_core.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_binding_for_testing.h"
#include "third_party/blink/renderer/core/dom/element.h"
#include "third_party/blink/renderer/core/exported/web_view_impl.h"
#include "third_party/blink/renderer/core/frame/frame_test_helpers.h"
#include "third_party/blink/renderer/core/frame/web_local_frame_impl.h"
#include "third_party/blink/renderer/platform/heap/thread_state.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource_error.h"
#include "third_party/blink/renderer/platform/mhtml/serialized_resource.h"
#include "third_party/blink/renderer/platform/scheduler/public/thread.h"
#include "third_party/blink/renderer/platform/testing/task_environment.h"
#include "third_party/blink/renderer/platform/testing/testing_platform_support.h"
#include "third_party/blink/renderer/platform/testing/unit_test_helpers.h"
#include "third_party/blink/renderer/platform/testing/url_loader_mock_factory.h"
#include "third_party/blink/renderer/platform/testing/url_test_helpers.h"
#include "third_party/blink/renderer/platform/wtf/deque.h"
#include "third_party/blink/renderer/platform/wtf/text/string_builder.h"
#include "third_party/blink/renderer/platform/wtf/vector.h"

namespace blink {
namespace {
using testing::Eq;
using testing::Pair;

class FrameSerializerTest
    : public testing::Test,
      public WebFrameSerializer::MHTMLPartsGenerationDelegate {
 public:
  FrameSerializerTest()
      : folder_("frameserializer/"),
        base_url_(url_test_helpers::ToKURL("http://www.test.com")) {}

  ~FrameSerializerTest() override {
    ThreadState::Current()->CollectAllGarbageForTesting();
  }

 protected:
  void SetUp() override {
    // We want the images to load.
    helper_.InitializeWithSettings(&ConfigureSettings);
  }

  void TearDown() override {
    URLLoaderMockFactory::GetSingletonInstance()
        ->UnregisterAllURLsAndClearMemoryCache();
    helper_.Reset();
  }

  void SetBaseFolder(const char* folder) { folder_ = folder; }

  void RegisterURL(const KURL& url, const char* file, const char* mime_type) {
    url_test_helpers::RegisterMockedURLLoad(
        url, test::CoreTestDataPath(WebString::FromUTF8(folder_ + file)),
        WebString::FromUTF8(mime_type));
  }

  void RegisterURL(const char* url, const char* file, const char* mime_type) {
    RegisterURL(KURL(base_url_, url), file, mime_type);
  }

  void RegisterURL(const char* file, const char* mime_type) {
    RegisterURL(file, file, mime_type);
  }

  void RegisterErrorURL(const char* file, int status_code) {
    ResourceError error = ResourceError::Failure(NullURL());

    WebURLResponse response;
    response.SetMimeType("text/html");
    response.SetHttpStatusCode(status_code);

    URLLoaderMockFactory::GetSingletonInstance()->RegisterErrorURL(
        KURL(base_url_, file), response, WebURLError(error));
  }

  void RegisterSkipURL(const char* url) {
    skip_urls_.insert(KURL(base_url_, url));
  }

  void Serialize(const char* url) {
    frame_test_helpers::LoadFrame(
        helper_.GetWebView()->MainFrameImpl(),
        KURL(base_url_, url).GetString().Utf8().c_str());
    // Sometimes we have iframes created in "onload" handler - wait for them to
    // load.
    frame_test_helpers::PumpPendingRequestsForFrameToLoad(
        helper_.GetWebView()->MainFrameImpl());
    Frame* frame = helper_.LocalMainFrame()->GetFrame();
    for (; frame; frame = frame->Tree().TraverseNext()) {
      // This is safe, because tests do not do cross-site navigation
      // (and therefore don't have remote frames).
      base::RunLoop run_loop;
      FrameSerializer::SerializeFrame(
          *this, *To<LocalFrame>(frame),
          base::BindLambdaForTesting([&](Deque<SerializedResource> resources) {
            for (auto& res : resources) {
              resources_.push_back(res);
              // Don't serialize the same resource on subsequent frames. This
              // mimics how FrameSerializer is actually used.
              skip_urls_.insert(res.url);
            }
            run_loop.Quit();
          }));
      URLLoaderMockFactory::GetSingletonInstance()->ServeAsynchronousRequests();
      run_loop.Run();
    }
  }

  Deque<SerializedResource>& GetResources() { return resources_; }
  Vector<std::pair<KURL, String>> GetResourceUrlAndMimeTypes() const {
    Vector<std::pair<KURL, String>> result;
    for (const SerializedResource& r : resources_) {
      result.emplace_back(r.url, r.mime_type);
    }
    return result;
  }
  const SerializedResource* GetResource(const KURL& url,
                                        const char* mime_type) {
    String mime(mime_type);
    for (const SerializedResource& resource : resources_) {
      if (resource.url == url && !resource.data->empty() &&
          (mime.IsNull() || EqualIgnoringASCIICase(resource.mime_type, mime)))
        return &resource;
    }
    return nullptr;
  }

  const SerializedResource* GetResource(const char* url_string,
                                        const char* mime_type) {
    return GetResource(ResourceURL(url_string), mime_type);
  }

  bool IsSerialized(const char* url, const char* mime_type = nullptr) {
    return GetResource(url, mime_type);
  }

  String GetSerializedData(const char* url, const char* mime_type = nullptr) {
    const SerializedResource* resource = GetResource(url, mime_type);
    if (resource) {
      const Vector<char> data = resource->data->CopyAs<Vector<char>>();
      return String(data);
    }
    return String();
  }

  KURL ResourceURL(const String& resource_name) const {
    return KURL(base_url_, resource_name);
  }

 private:
  static void ConfigureSettings(WebSettings* settings) {
    settings->SetImagesEnabled(true);
    settings->SetLoadsImagesAutomatically(true);
    settings->SetJavaScriptEnabled(true);
  }

  // WebFrameSerializer::MHTMLPartsGenerationDelegate impl.
  bool ShouldSkipResource(const WebURL& url) override {
    return skip_urls_.Contains(url.GetString());
  }
  bool UseBinaryEncoding() override { return false; }

  bool RemovePopupOverlay() override { return false; }

  test::TaskEnvironment task_environment_;
  ScopedTestingPlatformSupport<TestingPlatformSupport> platform_;
  frame_test_helpers::WebViewHelper helper_;
  std::string folder_;
  KURL base_url_;
  Deque<SerializedResource> resources_;
  HashSet<String> skip_urls_;
};

TEST_F(FrameSerializerTest, HTMLElements) {
  SetBaseFolder("frameserializer/elements/");

  RegisterURL("elements.html", "text/html");
  RegisterURL("style.css", "style.css", "text/css");
  RegisterErrorURL("style_network_error.css", 404);
  RegisterURL("copyright.html", "empty.txt", "text/html");
  RegisterURL("script.js", "empty.txt", "text/javascript");

  RegisterURL("bodyBackground.png", "image.png", "image/png");

  RegisterURL("imageSrc.png", "image.png", "image/png");

  RegisterURL("inputImage.png", "image.png", "image/png");

  RegisterURL("tableBackground.png", "image.png", "image/png");
  RegisterURL("trBackground.png", "image.png", "image/png");
  RegisterURL("tdBackground.png", "image.png", "image/png");

  RegisterURL("blockquoteCite.html", "empty.txt", "text/html");
  RegisterURL("qCite.html", "empty.txt", "text/html");
  RegisterURL("delCite.html", "empty.txt", "text/html");
  RegisterURL("insCite.html", "empty.txt", "text/html");

  RegisterErrorURL("nonExisting.png", 404);

  Serialize("elements.html");
  EXPECT_THAT(GetResourceUrlAndMimeTypes(),
              testing::UnorderedElementsAre(
                  Pair(ResourceURL("elements.html"), "text/html"),
                  Pair(ResourceURL("style.css"), "text/css"),
                  Pair(ResourceURL("bodyBackground.png"), "image/png"),
                  Pair(ResourceURL("imageSrc.png"), "image/png"),
                  Pair(ResourceURL("inputImage.png"), "image/png"),
                  Pair(ResourceURL("tableBackground.png"), "image/png"),
                  Pair(ResourceURL("trBackground.png"), "image/png"),
                  Pair(ResourceURL("tdBackground.png"), "image/png"),
                  Pair(ResourceURL("style_network_error.css"), "text/css")));
}

TEST_F(FrameSerializerTest, MHTMLImprovedHTMLElements) {
  base::test::ScopedFeatureList features(
      {blink::features::kMHTML_Improvements});
  SetBaseFolder("frameserializer/elements/");

  RegisterURL("elements.html", "text/html");
  RegisterURL("style.css", "style.css", "text/css");
  RegisterErrorURL("style_network_error.css", 404);
  RegisterURL("copyright.html", "empty.txt", "text/html");
  RegisterURL("script.js", "empty.txt", "text/javascript");

  RegisterURL("bodyBackground.png", "image.png", "image/png");

  RegisterURL("imageSrc.png", "image.png", "image/png");

  RegisterURL("inputImage.png", "image.png", "image/png");

  RegisterURL("tableBackground.png", "image.png", "image/png");
  RegisterURL("trBackground.png", "image.png", "image/png");
  RegisterURL("tdBackground.png", "image.png", "image/png");

  RegisterURL("blockquoteCite.html", "empty.txt", "text/html");
  RegisterURL("qCite.html", "empty.txt", "text/html");
  RegisterURL("delCite.html", "empty.txt", "text/html");
  RegisterURL("insCite.html", "empty.txt", "text/html");

  RegisterErrorURL("nonExisting.png", 404);

  Serialize("elements.html");

  EXPECT_THAT(GetResourceUrlAndMimeTypes(),
              testing::UnorderedElementsAre(
                  Pair(ResourceURL("elements.html"), "text/html"),
                  Pair(ResourceURL("style.css"), "text/css"),
                  Pair(ResourceURL("bodyBackground.png"), "image/png"),
                  Pair(ResourceURL("imageSrc.png"), "image/png"),
                  Pair(ResourceURL("inputImage.png"), "image/png"),
                  Pair(ResourceURL("tableBackground.png"), "image/png"),
                  Pair(ResourceURL("trBackground.png"), "image/png"),
                  Pair(ResourceURL("tdBackground.png"), "image/png"),
                  Pair(testing::Property(&KURL::IsValid, Eq(true)),
                       "text/javascript")));
}

TEST_F(FrameSerializerTest, Frames) {
  SetBaseFolder("frameserializer/frames/");

  RegisterURL("simple_frames.html", "text/html");
  RegisterURL("simple_frames_top.html", "text/html");
  RegisterURL("simple_frames_1.html", "text/html");
  RegisterURL("simple_frames_3.html", "text/html");

  RegisterURL("frame_1.png", "image.png", "image/png");
  RegisterURL("frame_2.png", "image.png", "image/png");
  RegisterURL("frame_3.png", "image.png", "image/png");
  RegisterURL("frame_4.png", "image.png", "image/png");

  Serialize("simple_frames.html");

  EXPECT_EQ(8U, GetResources().size());

  EXPECT_TRUE(IsSerialized("simple_frames.html", "text/html"));
  EXPECT_TRUE(IsSerialized("simple_frames_top.html", "text/html"));
  EXPECT_TRUE(IsSerialized("simple_frames_1.html", "text/html"));
  EXPECT_TRUE(IsSerialized("simple_frames_3.html", "text/html"));

  EXPECT_TRUE(IsSerialized("frame_1.png", "image/png"));
  EXPECT_TRUE(IsSerialized("frame_2.png", "image/png"));
  EXPECT_TRUE(IsSerialized("frame_3.png", "image/png"));
  EXPECT_TRUE(IsSerialized("frame_4.png", "image/png"));

  // Verify all 3 frame src are rewritten to Content ID URLs.
  Vector<String> split_string;
  GetSerializedData("simple_frames.html", "text/html")
      .Split("<frame src=\"cid:", split_string);
  EXPECT_EQ(split_string.size(), 4u);
}

TEST_F(FrameSerializerTest, IFrames) {
  SetBaseFolder("frameserializer/frames/");

  RegisterURL("top_frame.html", "text/html");
  RegisterURL("simple_iframe.html", "text/html");
  RegisterURL("object_iframe.html", "text/html");
  RegisterURL("embed_iframe.html", "text/html");
  RegisterURL("encoded_iframe.html", "text/html");

  RegisterURL("top.png", "image.png", "image/png");
  RegisterURL("simple.png", "image.png", "image/png");
  RegisterURL("object.png", "image.png", "image/png");
  RegisterURL("embed.png", "image.png", "image/png");

  Serialize("top_frame.html");

  EXPECT_EQ(10U, GetResources().size());

  EXPECT_TRUE(IsSerialized("top_frame.html", "text/html"));
  EXPECT_TRUE(IsSerialized("simple_iframe.html", "text/html"));  // Twice.
  EXPECT_TRUE(IsSerialized("object_iframe.html", "text/html"));
  EXPECT_TRUE(IsSerialized("embed_iframe.html", "text/html"));
  EXPECT_TRUE(IsSerialized("encoded_iframe.html", "text/html"));

  EXPECT_TRUE(IsSerialized("top.png", "image/png"));
  EXPECT_TRUE(IsSerialized("simple.png", "image/png"));
  EXPECT_TRUE(IsSerialized("object.png", "image/png"));
  EXPECT_TRUE(IsSerialized("embed.png", "image/png"));

  // Ensure that frame contents are not NFC-normalized before encoding.
  String expected_meta_charset =
      "<meta http-equiv=\"Content-Type\" content=\"text/html; "
      "charset=EUC-KR\">";
  EXPECT_TRUE(GetSerializedData("encoded_iframe.html", "text/html")
                  .Contains(expected_meta_charset));
  EXPECT_TRUE(GetSerializedData("encoded_iframe.html", "text/html")
                  .Contains("\xE4\xC5\xD1\xE2"));
  EXPECT_FALSE(GetSerializedData("encoded_iframe.html", "text/html")
                   .Contains("\xE4\xC5\xE4\xC5"));
}

// Tests that when serializing a page with blank frames these are reported with
// their resources.
TEST_F(FrameSerializerTest, BlankFrames) {
  SetBaseFolder("frameserializer/frames/");

  RegisterURL("blank_frames.html", "text/html");
  RegisterURL("red_background.png", "image.png", "image/png");
  RegisterURL("orange_background.png", "image.png", "image/png");
  RegisterURL("blue_background.png", "image.png", "image/png");

  Serialize("blank_frames.html");

  EXPECT_EQ(7U, GetResources().size());

  EXPECT_TRUE(
      IsSerialized("http://www.test.com/red_background.png", "image/png"));
  EXPECT_TRUE(
      IsSerialized("http://www.test.com/orange_background.png", "image/png"));
  EXPECT_TRUE(
      IsSerialized("http://www.test.com/blue_background.png", "image/png"));

  // The blank frames no longer get magic URL (i.e. wyciwyg://frame/0), so we
  // can't really assert their presence via URL.  We also can't use content-id
  // in assertions (since it is not deterministic).  Therefore we need to rely
  // on getResources().size() assertion above and on browser-level tests
  // (i.e. SavePageMultiFrameBrowserTest.AboutBlank).
}

TEST_F(FrameSerializerTest, CSS) {
  SetBaseFolder("frameserializer/css/");

  RegisterURL("css_test_page.html", "text/html");
  RegisterURL("link_styles.css", "text/css");
  RegisterURL("encoding.css", "text/css");
  RegisterURL("import_style_from_link.css", "text/css");
  RegisterURL("import_styles.css", "text/css");
  RegisterURL("do_not_serialize.png", "image.png", "image/png");
  RegisterURL("red_background.png", "image.png", "image/png");
  RegisterURL("orange_background.png", "image.png", "image/png");
  RegisterURL("yellow_background.png", "image.png", "image/png");
  RegisterURL("green_background.png", "image.png", "image/png");
  RegisterURL("blue_background.png", "image.png", "image/png");
  RegisterURL("purple_background.png", "image.png", "image/png");
  RegisterURL("pink_background.png", "image.png", "image/png");
  RegisterURL("brown_background.png", "image.png", "image/png");
  RegisterURL("ul-dot.png", "image.png", "image/png");
  RegisterURL("ol-dot.png", "image.png", "image/png");

  const KURL image_url_from_data_url(
      url_test_helpers::ToKURL("http://www.dataurl.com"),
      "fuchsia_background.png");
  RegisterURL(image_url_from_data_url, "image.png", "image/png");

  RegisterURL("included_in_another_frame.css", "text/css");
  RegisterSkipURL("included_in_another_frame.css");

  Serialize("css_test_page.html");

  // 16 resoucres added by RegisterURL + 3 resources added due to converting
  // style elements to link elements.
  EXPECT_EQ(19U, GetResources().size());

  EXPECT_FALSE(IsSerialized("do_not_serialize.png", "image/png"));
  EXPECT_FALSE(IsSerialized("included_in_another_frame.css", "text/css"));

  EXPECT_TRUE(IsSerialized("css_test_page.html", "text/html"));
  EXPECT_TRUE(IsSerialized("link_styles.css", "text/css"));
  EXPECT_TRUE(IsSerialized("encoding.css", "text/css"));
  EXPECT_TRUE(IsSerialized("import_styles.css", "text/css"));
  EXPECT_TRUE(IsSerialized("import_style_from_link.css", "text/css"));
  EXPECT_TRUE(IsSerialized("red_background.png", "image/png"));
  EXPECT_TRUE(IsSerialized("orange_background.png", "image/png"));
  EXPECT_TRUE(IsSerialized("yellow_background.png", "image/png"));
  EXPECT_TRUE(IsSerialized("green_background.png", "image/png"));
  EXPECT_TRUE(IsSerialized("blue_background.png", "image/png"));
  EXPECT_TRUE(IsSerialized("purple_background.png", "image/png"));
  EXPECT_TRUE(IsSerialized("pink_background.png", "image/png"));
  EXPECT_TRUE(IsSerialized("brown_background.png", "image/png"));
  EXPECT_TRUE(IsSerialized("ul-dot.png", "image/png"));
  EXPECT_TRUE(IsSerialized("ol-dot.png", "image/png"));

  EXPECT_TRUE(GetResource(image_url_from_data_url, "image/png"));

  // Ensure encodings are specified.
  EXPECT_TRUE(
      GetSerializedData("link_styles.css", "text/css").StartsWith("@charset"));
  EXPECT_TRUE(GetSerializedData("import_styles.css", "text/css")
                  .StartsWith("@charset"));
  EXPECT_TRUE(GetSerializedData("import_style_from_link.css", "text/css")
                  .StartsWith("@charset"));
  EXPECT_TRUE(GetSerializedData("encoding.css", "text/css")
                  .StartsWith("@charset \"euc-kr\";"));

  // Ensure that stylesheet contents are not NFC-normalized before encoding.
  EXPECT_TRUE(GetSerializedData("encoding.css", "text/css")
                  .Contains("\xE4\xC5\xD1\xE2"));
  EXPECT_FALSE(GetSerializedData("encoding.css", "text/css")
                   .Contains("\xE4\xC5\xE4\xC5"));
}

TEST_F(FrameSerializerTest, CSSImport) {
  SetBaseFolder("frameserializer/css/");

  RegisterURL("import.html", "text/html");
  RegisterURL("import/base.css", "text/css");
  RegisterURL("import/relative/red-background.css", "text/css");
  RegisterURL("import/absolute/green-header.css", "text/css");

  Serialize("import.html");

  EXPECT_TRUE(IsSerialized("import.html", "text/html"));
  EXPECT_TRUE(IsSerialized("import/base.css", "text/css"));
  EXPECT_TRUE(IsSerialized("import/relative/red-background.css", "text/css"));
  EXPECT_TRUE(IsSerialized("import/absolute/green-header.css", "text/css"));
}

TEST_F(FrameSerializerTest, XMLDeclaration) {
  V8TestingScope scope;
  SetBaseFolder("frameserializer/xml/");

  RegisterURL("xmldecl.xml", "text/xml");
  Serialize("xmldecl.xml");

  String expected_start("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  EXPECT_TRUE(GetSerializedData("xmldecl.xml").StartsWith(expected_start));
}

TEST_F(FrameSerializerTest, DTD) {
  SetBaseFolder("frameserializer/dtd/");

  RegisterURL("html5.html", "text/html");
  Serialize("html5.html");

  String expected_start("<!DOCTYPE html>");
  EXPECT_TRUE(GetSerializedData("html5.html").StartsWith(expected_start));
}

TEST_F(FrameSerializerTest, Font) {
  SetBaseFolder("frameserializer/font/");

  RegisterURL("font.html", "text/html");
  RegisterURL("font.ttf", "application/octet-stream");

  Serialize("font.html");

  EXPECT_TRUE(IsSerialized("font.ttf", "application/octet-stream"));
}

TEST_F(FrameSerializerTest, DataURI) {
  SetBaseFolder("frameserializer/datauri/");

  RegisterURL("page_with_data.html", "text/html");

  Serialize("page_with_data.html");

  EXPECT_EQ(1U, GetResources().size());
  EXPECT_TRUE(IsSerialized("page_with_data.html", "text/html"));
}

TEST_F(FrameSerializerTest, DataURIMorphing) {
  SetBaseFolder("frameserializer/datauri/");

  RegisterURL("page_with_morphing_data.html", "text/html");

  Serialize("page_with_morphing_data.html");

  EXPECT_EQ(2U, GetResources().size());
  EXPECT_TRUE(IsSerialized("page_with_morphing_data.html", "text/html"));
}

// Test that we don't regress https://bugs.webkit.org/show_bug.cgi?id=99105
TEST_F(FrameSerializerTest, SVGImageDontCrash) {
  SetBaseFolder("frameserializer/svg/");

  RegisterURL("page_with_svg_image.html", "text/html");
  RegisterURL("green_rectangle.svg", "image/svg+xml");

  Serialize("page_with_svg_image.html");

  EXPECT_EQ(2U, GetResources().size());

  EXPECT_TRUE(IsSerialized("green_rectangle.svg", "image/svg+xml"));
  EXPECT_GT(GetSerializedData("green_rectangle.svg", "image/svg+xml").length(),
            250U);
}

TEST_F(FrameSerializerTest, DontIncludeErrorImage) {
  SetBaseFolder("frameserializer/image/");

  RegisterURL("page_with_img_error.html", "text/html");
  RegisterURL("error_image.png", "image/png");

  Serialize("page_with_img_error.html");

  EXPECT_EQ(1U, GetResources().size());
  EXPECT_TRUE(IsSerialized("page_with_img_error.html", "text/html"));
  EXPECT_FALSE(IsSerialized("error_image.png", "image/png"));
}

TEST_F(FrameSerializerTest, NamespaceElementsDontCrash) {
  SetBaseFolder("frameserializer/namespace/");

  RegisterURL("namespace_element.html", "text/html");

  Serialize("namespace_element.html");

  EXPECT_EQ(1U, GetResources().size());
  EXPECT_TRUE(IsSerialized("namespace_element.html", "text/html"));
  EXPECT_GT(GetSerializedData("namespace_element.html", "text/html").length(),
            0U);
}

TEST_F(FrameSerializerTest, markOfTheWebDeclaration) {
  EXPECT_EQ("saved from url=(0015)http://foo.com/",
            FrameSerializer::MarkOfTheWebDeclaration(KURL("http://foo.com")));
  EXPECT_EQ("saved from url=(0015)http://f-o.com/",
            FrameSerializer::MarkOfTheWebDeclaration(KURL("http://f-o.com")));
  EXPECT_EQ("saved from url=(0019)http://foo.com-%2D/",
            FrameSerializer::MarkOfTheWebDeclaration(KURL("http://foo.com--")));
  EXPECT_EQ(
      "saved from url=(0024)http://f-%2D.com-%2D%3E/",
      FrameSerializer::MarkOfTheWebDeclaration(KURL("http://f--.com-->")));
  EXPECT_EQ(
      "saved from url=(0020)http://foo.com/?-%2D",
      FrameSerializer::MarkOfTheWebDeclaration(KURL("http://foo.com?--")));
  EXPECT_EQ(
      "saved from url=(0020)http://foo.com/#-%2D",
      FrameSerializer::MarkOfTheWebDeclaration(KURL("http://foo.com#--")));
  EXPECT_EQ("saved from url=(0026)http://foo.com/#bar-%2Dbaz",
            FrameSerializer::MarkOfTheWebDeclaration(
                KURL("http://foo.com#bar--baz")));
}

TEST_F(FrameSerializerTest, ReplaceAllCaseInsensitive) {
  auto transform = [](const String& from) { return String("</HI>"); };
  EXPECT_EQ(
      blink::internal::ReplaceAllCaseInsensitive("", "</style>", transform),
      "");
  EXPECT_EQ(
      blink::internal::ReplaceAllCaseInsensitive("test", "</style>", transform),
      "test");
  EXPECT_EQ(blink::internal::ReplaceAllCaseInsensitive("</Style>", "</style>",
                                                       transform),
            "</HI>");
  EXPECT_EQ(blink::internal::ReplaceAllCaseInsensitive("x</Style>", "</style>",
                                                       transform),
            "x</HI>");
  EXPECT_EQ(blink::internal::ReplaceAllCaseInsensitive("</Style>x", "</style>",
                                                       transform),
            "</HI>x");
  EXPECT_EQ(blink::internal::ReplaceAllCaseInsensitive(
                "test</Style>test</Style>testagain", "</style>", transform),
            "test</HI>test</HI>testagain");
}

}  // namespace
}  // namespace blink
