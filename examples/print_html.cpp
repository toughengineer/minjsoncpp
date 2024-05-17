#include <minjsoncpp.h>

#include <iostream>

using namespace std::string_view_literals;

int main() {
  // clang-format off
  const minjson::Value value = minjson::Object {
    { "null", minjson::Null{} },
    { "boolean", true },
    { "integer", 42 },
    { "decimal", 3.14 },
    { "string", "hello there"sv },
    { "array", minjson::Array{ 1, 2, 3 } },
    { "object", minjson::Object{
        { "nested number", 23 },
        { "nested string", "General Kenobi"sv },
        { "nested array", minjson::Array{ 4, 5, 6 } },
        { "nested object", minjson::Object{ { "foo", "bar"sv } } }
      }
    }
  };
  // clang-format on

  minjson::SerializationOptions options;
  options.indent = 2;
  options.nullLiteral = R"(<span style="color: gray;">null</span>)"sv;
  options.falseLiteral = R"(<span style="color: blue;">false</span>)"sv;
  options.trueLiteral = R"(<span style="color: blue;">true</span>)"sv;
  options.emptyObject = R"(<span style="color: red;">{}</span>)"sv;
  options.objectOpeningBrace = R"(<span style="color: red;">{</span>)"sv;
  options.objectClosingBrace = R"(<span style="color: red;">}</span>)"sv;
  options.objectKeyValueSeparator = R"(<span style="color: darkred;">: </span>)"sv;
  options.objectMemberSeparator = R"(<span style="color: darkred;">,</span>)"sv;
  options.emptyArray = R"(<span style="color: magenta;">[]</span>)"sv;
  options.arrayOpeningBracket = R"(<span style="color: magenta;">[</span>)"sv;
  options.arrayClosingBracket = R"(<span style="color: magenta;">]</span>)"sv;
  options.arrayMemberSeparator = R"(<span style="color: darkmagenta;">,</span>)"sv;
  options.openingStringQuotation = R"(<span style="color: lightgray;">"</span><span style="color: teal;">)"sv;
  options.closingStringQuotation = R"(</span><span style="color: lightgray;">"</span>)"sv;

  std::cout << "<pre>\n";
  minjson::serializeToStream(std::cout, value, options);
  std::cout << "\n</pre>";
}
