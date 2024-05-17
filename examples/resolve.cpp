#include <minjsoncpp.h>

#include <iostream>

using namespace std::string_view_literals;

template<typename... T>
void printResolved(const minjson::Value &document, T &&...referenceTokens) {
  const auto resolved = document.resolve(std::forward<T>(referenceTokens)...);

  std::cout << (resolved ? "resolved value" : "resolution failed");

  std::cout << " for \'";
  ((std::cout << '/' << referenceTokens), ...);
  std::cout << '\'';

  if (resolved) {
    std::cout << ":\n\n";
    minjson::serializeToStream(std::cout, *resolved);
    std::cout << '\n';
  }
  std::cout << '\n';
}

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

  std::cout << "JSON:\n\n" << minjson::serializeToString(value, { 2 }) << "\n\n";

  printResolved(value, "array", 1);
  printResolved(value, "object", "nested number");
  printResolved(value, "object", "nested array", 0);
  printResolved(value, "object", "nested object", "foo");

  printResolved(value, "nonexistent");
}
