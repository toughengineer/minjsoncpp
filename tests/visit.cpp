#include <minjsoncpp.h>

#include "utils.h"

#include "catch2/catch_amalgamated.hpp"

using namespace std::string_view_literals;

template<typename... T>
struct Overloaded : T... {
  using T::operator()...;
};
template<typename... T>
Overloaded(T...) -> Overloaded<T...>;

TEST_CASE("visitation", "[visit]") {
  SECTION("null") {
    const minjson::Value v = minjson::Null{};
    visit(Overloaded{ [](minjson::Null) { SUCCEED(); }, [](auto &&) { FAIL(); } }, v);
  }

  SECTION("boolean") {
    const minjson::Value v = false;
    visit(Overloaded{ [](minjson::Boolean b) { REQUIRE(b == false); }, [](auto &&) { FAIL(); } }, v);
  }

  SECTION("integer") {
    const minjson::Value v = 42;
    visit(Overloaded{ [](int64_t i) { REQUIRE(i == 42); }, [](auto &&) { FAIL(); } }, v);
  }

  SECTION("floating point") {
    const minjson::Value v = 3.14;
    visit(Overloaded{ [](double d) { REQUIRE(d == 3.14); }, [](auto &&) { FAIL(); } }, v);
  }

  SECTION("string") {
    const minjson::Value v = "foo bar"sv;
    visit(Overloaded{ [](const minjson::String &s) { REQUIRE(s == "foo bar"sv); }, [](auto &&) { FAIL(); } }, v);
  }

  SECTION("array") {
    const minjson::Value v = minjson::Array{ 1, 2, 3 };
    visit(
      Overloaded{ [](const minjson::Array &a) { REQUIRE(a == minjson::Array{ 1, 2, 3 }); }, [](auto &&) { FAIL(); } },
      v);
  }

  SECTION("object") {
    const minjson::Value v = minjson::Object{ { "foo", "bar" }, { "baz", 42 } };
    visit(
      Overloaded{ [](const minjson::Object &a) { REQUIRE(a == minjson::Object{ { "foo", "bar" }, { "baz", 42 } }); },
                  [](auto &&) { FAIL(); } },
      v);
  }
}
