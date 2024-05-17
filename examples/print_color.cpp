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
  options.nullLiteral = "\x1b[90mnull\x1b[0m";
  options.falseLiteral = "\x1b[96mfalse\x1b[0m";
  options.trueLiteral = "\x1b[96mtrue\x1b[0m";
  options.emptyObject = "\x1b[91m{}\x1b[0m";
  options.objectOpeningBrace = "\x1b[91m{\x1b[0m";
  options.objectClosingBrace = "\x1b[91m}\x1b[0m";
  options.objectKeyValueSeparator = "\x1b[31m:\x1b[0m ";
  options.objectMemberSeparator = "\x1b[31m,\x1b[0m";
  options.emptyArray = "\x1b[95m[]\x1b[0m";
  options.arrayOpeningBracket = "\x1b[95m[\x1b[0m";
  options.arrayClosingBracket = "\x1b[95m]\x1b[0m";
  options.arrayMemberSeparator = "\x1b[35m,\x1b[0m";
  options.openingStringQuotation = "\x1b[90m\"\x1b[97m";
  options.closingStringQuotation = "\x1b[90m\"\x1b[0m";

  std::cout << "serialized JSON:\n\n";
  minjson::serializeToStream(std::cout, value, options);
  std::cout << '\n';
}
