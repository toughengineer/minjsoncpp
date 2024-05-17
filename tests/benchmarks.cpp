#include <minjsoncpp.h>

#include "utils.h"

#include <catch2/catch_all.hpp>

#include <iostream>
#include <fstream>

#if !defined(_LIBCPP_VERSION) || _LIBCPP_VERSION >= 160000
#include <memory_resource>
#define POLYMORPHIC_ALLOCATOR_IS_SUPPORTED
#endif

using namespace std::string_view_literals;

namespace {
struct StringSink {
  StringSink() {
    str.reserve(9001);
  }
  std::string str;
  void operator()(std::string_view s) {
    str += s;
  }
};
struct DummySink {
  size_t size = 0;
  void operator()(std::string_view s) {
    size += s.size();
  }
};
using PolymorphicSink = std::function<void(std::string_view)>;
std::string loadFile(const std::string &filename) {
  std::ifstream file{ filename, std::ios_base::binary };
  if (!file.is_open())
    throw std::runtime_error{ "failed to open " + filename };
  return { std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
}
}  // namespace

TEST_CASE("escape benchmark (valid UTF-8 strings)", "[escape][!benchmark]") {
  const auto string =
    "some ASCII character to escape: \t \" \\ \n"
    "utf8 2 byte code point \xC2\xA3 3 byte code point \xE2\x82\xAC 4 byte code point \xF0\x9F\x98\x80 characters"sv;

  SECTION("default") {
    BENCHMARK("string") {
      return minjson::escape(string).size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::escape(sink, string, {}, {}, {});
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::escape(sink, string, {}, {}, {});
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::escape<PolymorphicSink>(sink, string, {}, {}, {});
      return sink.size;
    };
  }

  SECTION("default, validate UTF-8") {
    BENCHMARK("string") {
      return minjson::escape(string, {}, minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits).size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::escape(sink, string, {}, minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits, {});
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::escape(sink, string, {}, minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits, {});
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::escape<PolymorphicSink>(sink, string, {}, minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits, {});
      return sink.size;
    };
  }

  SECTION("non-ASCII") {
    BENCHMARK("string") {
      return minjson::escape(string, minjson::Escape::NonAscii).size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::escape(sink, string, minjson::Escape::NonAscii, {}, {});
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::escape(sink, string, minjson::Escape::NonAscii, {}, {});
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::escape<PolymorphicSink>(sink, string, minjson::Escape::NonAscii, {}, {});
      return sink.size;
    };
  }

  SECTION("non-ASCII, validate UTF-8") {
    BENCHMARK("string") {
      return minjson::escape(string, minjson::Escape::NonAscii, minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits)
        .size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::escape(sink, string, minjson::Escape::NonAscii,
                            minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits, {});
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::escape(sink, string, minjson::Escape::NonAscii,
                            minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits, {});
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::escape<PolymorphicSink>(sink, string, minjson::Escape::NonAscii,
                                             minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits, {});
      return sink.size;
    };
  }
}


TEST_CASE("serialization benchmark", "[serialization][!benchmark]") {
  // clang-format off
  const minjson::Value v = minjson::Object{
    { "null", minjson::Null{} },
    { "boolean", true },
    { "integer", 42 },
    { "double", 3.14 },
    { "string", "hello" },
    { "array", minjson::Array{ minjson::Null{}, false, 23, 0.5, "there" } },
    { "object", minjson::Object{
      { "nested null", minjson::Null{} },
      { "nested boolean", true },
      { "nested integer", 13 },
      { "nested double", 1.618 },
      { "nested string", "General Kenobi" }
    }}
  };
  // clang-format on

  static const auto dummy = std::invoke([&v] {
    std::cout << "\nserialization benchmark JSON:\n";
    minjson::SerializationOptions options;
    options.indent = 2;
    options.objectMemberSeparator = ": "sv;
    minjson::serializeToStream(std::cout, v, options);
    std::cout << "\n\n";
    return 42;
  });
  (void)dummy;

  SECTION("default") {
    BENCHMARK("string") {
      return minjson::serializeToString(v).size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::serialize(sink, v, {});
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::serialize(sink, v, {});
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::serialize<PolymorphicSink>(sink, v, {});
      return sink.size;
    };
  }

  minjson::SerializationOptions options;
  options.indent = 2;
  SECTION("indent 2 spaces") {
    BENCHMARK("string") {
      return minjson::serializeToString(v, options).size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::serialize(sink, v, options);
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::serialize(sink, v, options);
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::serialize<PolymorphicSink>(sink, v, options);
      return sink.size;
    };
  }

  options.indentationChar = minjson::SerializationOptions::IndentationChar::Tab;
  SECTION("indent 2 tabs") {
    BENCHMARK("string") {
      return minjson::serializeToString(v, options).size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::serialize(sink, v, options);
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::serialize(sink, v, options);
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::serialize<PolymorphicSink>(sink, v, options);
      return sink.size;
    };
  }

  options.newline = { {}, true, true, true, true, true, true, true, true, true, true, true };
  SECTION("all the newlines, indent 2 tabs") {
    BENCHMARK("string") {
      return minjson::serializeToString(v, options).size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::serialize(sink, v, options);
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::serialize(sink, v, options);
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::serialize(sink, v, options);
      return sink.size;
    };
  }
}


TEST_CASE("unescape benchmark (valid escapes)", "[unescape][!benchmark]") {
  const auto string =
    R"(some ASCII character escapes: \b \f \n \r \t \" \\ )"
    R"(some ASCII control character escapes: \u0000 \u000f )"
    R"(utf8 2 byte code point \u00a3 3 byte code point \u20ac 4 byte code point \ud83d\ude00 characters)"sv;

  SECTION("default") {
    BENCHMARK("string") {
      return minjson::unescape(string).size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::unescape(sink, string, {}, minjson::impl::DoNotReplaceSurrogates);
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::unescape(sink, string, {}, minjson::impl::DoNotReplaceSurrogates);
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::unescape<PolymorphicSink>(sink, string, {}, minjson::impl::DoNotReplaceSurrogates);
      return sink.size;
    };
  }

  SECTION("strict") {
    BENCHMARK("string") {
      return minjson::unescape(string, minjson::UnescapeMode::Strict).size();
    };
    BENCHMARK("string /w reserve") {
      StringSink sink{};
      minjson::impl::unescape(sink, string, minjson::UnescapeMode::Strict, minjson::impl::DoNotReplaceSurrogates);
      return sink.str.size();
    };
    BENCHMARK("dummy sink") {
      DummySink sink;
      minjson::impl::unescape(sink, string, minjson::UnescapeMode::Strict, minjson::impl::DoNotReplaceSurrogates);
      return sink.size;
    };
    BENCHMARK("polymorphic dummy sink") {
      DummySink sink;
      minjson::impl::unescape<PolymorphicSink>(sink, string, minjson::UnescapeMode::Strict,
                                               minjson::impl::DoNotReplaceSurrogates);
      return sink.size;
    };
  }
}


TEST_CASE("parsing benchmark", "[parse][!benchmark]") {
  SECTION("tiny JSON") {
    const auto string = R"({
  "null": null,
  "boolean": true,
  "integer": 42,
  "double": 3.14,
  "string": "hello",
  "array": [ null, false, 23, 0.5, "there" ],
  "object": {
    "nested null": null,
    "nested boolean": true,
    "nested integer": 13,
    "nested double": 1.618,
    "nested string": "General Kenobi"
  }
})";

    static const auto dummy = std::invoke([&string] {
      std::cout << "\nparsing benchmark JSON:\n" << string << "\n\n";
      return 42;
    });
    (void)dummy;


    BENCHMARK("default") {
      return std::move(minjson::parse(string).parsedSize);
    };

#if defined(POLYMORPHIC_ALLOCATOR_IS_SUPPORTED)
    BENCHMARK("pmr allocator: heap") {
      return minjson::parse(string, {}, {}, std::pmr::polymorphic_allocator<char>{ std::pmr::new_delete_resource() })
        .parsedSize;
    };
    BENCHMARK("pmr allocator: buffer->heap") {
      std::pmr::monotonic_buffer_resource buffer{ std::pmr::new_delete_resource() };
      return minjson::parse(string, {}, {}, std::pmr::polymorphic_allocator<char>{ &buffer }).parsedSize;
    };

    BENCHMARK("pmr allocator: pool->heap") {
      std::pmr::unsynchronized_pool_resource pool{ std::pmr::new_delete_resource() };
      return minjson::parse(string, {}, {}, std::pmr::polymorphic_allocator<char>{ &pool }).parsedSize;
    };

    BENCHMARK("pmr allocator: pool->buffer->heap") {
      std::pmr::monotonic_buffer_resource buffer{ std::pmr::new_delete_resource() };
      std::pmr::unsynchronized_pool_resource pool{ &buffer };
      return minjson::parse(string, {}, {}, std::pmr::polymorphic_allocator<char>{ &pool }).parsedSize;
    };
#endif
  }

  // canada.json from https://github.com/mloskot/json_benchmark/blob/master/data/canada.json
  // citm_catalog.json from
  // https://github.com/RichardHightower/json-parsers-benchmark/blob/master/data/citm_catalog.json
  const std::string filename = GENERATE("canada.json", "citm_catalog.json");

  DYNAMIC_SECTION(filename) {
    const auto input = loadFile(filename);

    BENCHMARK("default") {
      return minjson::parse(input).parsedSize;
    };
#if defined(POLYMORPHIC_ALLOCATOR_IS_SUPPORTED)
    BENCHMARK("pmr allocator: buffer->heap") {
      std::pmr::monotonic_buffer_resource buffer{ std::pmr::new_delete_resource() };
      return minjson::parse(input, {}, {}, std::pmr::polymorphic_allocator<char>{ &buffer }).parsedSize;
    };

    BENCHMARK("pmr allocator: pool->heap") {
      std::pmr::unsynchronized_pool_resource pool{ std::pmr::new_delete_resource() };
      return minjson::parse(input, {}, {}, std::pmr::polymorphic_allocator<char>{ &pool }).parsedSize;
    };
#endif
  }
}
