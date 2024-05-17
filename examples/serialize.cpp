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
  options.objectKeyValueSeparator = ": "sv;

  const auto serialized = minjson::serializeToString(value, options);

  std::cout << "serialized JSON:\n\n" << serialized << '\n';
}
