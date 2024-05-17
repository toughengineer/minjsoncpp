#include <minjsoncpp.h>

#include <iostream>

using namespace std::string_view_literals;

void printType(minjson::Null) {
  std::cout << "got null";
}
void printType(bool) {
  std::cout << "got a bool";
}
template<typename T>
void printType(T) {
  static_assert(std::is_same_v<T, int64_t> || std::is_same_v<T, double>);
  std::cout << "got a number";
}
void printType(const std::string &) {
  std::cout << "got a string";
}
void printType(const minjson::Array &) {
  std::cout << "got an array";
}
void printType(const minjson::Object &) {
  std::cout << "got an object";
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

  for (const auto &[key, val] : value.asObject()) {
    minjson::visit([](auto &x) { printType(x); }, val);
    std::cout << " for \'" << key << "\'\n";
  }
}
