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

  std::cout << "serialized JSON:\n\n";

  // a sink repeatedly receives parts of serialized string as 'std::string_view' instances
  const auto stdOutSink = [](std::string_view s) { std::cout << s; };

  minjson::SerializationOptions options;
  options.indent = 2;
  options.objectKeyValueSeparator = ": "sv;

  std::cout << "   "sv;                 // need to indent the first line
  const size_t initialIndentation = 3;  // indent the whole output

  // 'minjson::impl::serialize()' can be called with any type of sink
  // that is able to receive parts as 'std::string_view' instances
  minjson::impl::serialize(stdOutSink, value, options, initialIndentation);

  std::cout << '\n';
}
