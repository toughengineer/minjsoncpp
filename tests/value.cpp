#include <minjsoncpp.h>

#include "utils.h"

#include <catch2/catch_all.hpp>

using namespace std::string_view_literals;

TEST_CASE("default value construction", "[value][construction]") {
  minjson::Value defaultInitialization;
  minjson::Value valueInitialization{};
  auto valueInitialization2 = minjson::Value();

  CHECK(defaultInitialization == valueInitialization);
  CHECK_FALSE(defaultInitialization != valueInitialization);
  CHECK(defaultInitialization == valueInitialization2);
  CHECK_FALSE(defaultInitialization != valueInitialization2);
}


TEST_CASE("null value construction and assignment", "[value][construction][assignment]") {
  SECTION("construction") {
    minjson::Value directInitialization{ minjson::Null{} };
    CHECK(directInitialization.isNull());

    minjson::Value directInitialization2(minjson::Null{});
    CHECK(directInitialization2.isNull());

    minjson::Value copyInitialization = minjson::Null{};
    CHECK(copyInitialization.isNull());

    CHECK(directInitialization == directInitialization2);
    CHECK_FALSE(directInitialization != directInitialization2);
    CHECK(directInitialization == copyInitialization);
    CHECK_FALSE(directInitialization != copyInitialization);
  }

  SECTION("assignment") {
    minjson::Value v;
    v = minjson::Null{};
    CHECK(v.isNull());
  }
}


TEST_CASE("boolean value construction and assignment", "[value][construction][assignment]") {
  SECTION("construction") {
    minjson::Value directInitialization{ false };
    REQUIRE(directInitialization.isBool());
    CHECK(directInitialization.asBool() == false);

    minjson::Value directInitialization2(false);
    REQUIRE(directInitialization2.isBool());
    CHECK(directInitialization2.asBool() == false);

    minjson::Value copyInitialization = false;
    REQUIRE(copyInitialization.isBool());
    CHECK(copyInitialization.asBool() == false);

    CHECK(directInitialization == directInitialization2);
    CHECK_FALSE(directInitialization != directInitialization2);
    CHECK(directInitialization == copyInitialization);
    CHECK_FALSE(directInitialization != copyInitialization);
  }

  SECTION("assignment") {
    minjson::Value v;
    v = false;
    REQUIRE(v.isBool());
    CHECK(v.asBool() == false);
  }
}


TEMPLATE_TEST_CASE("integer value construction and assignment",
                   "[value][construction][assignment]",
                   char,
                   signed char,
                   unsigned char,
                   int8_t,
                   short,
                   unsigned short,
                   int16_t,
                   int,
                   int32_t,
                   long,
                   long long,
                   int64_t) {
  SECTION("construction") {
    minjson::Value directInitialization{ static_cast<TestType>(42) };
    REQUIRE(directInitialization.isInt());
    CHECK(directInitialization.asInt() == static_cast<TestType>(42));

    minjson::Value directInitialization2(static_cast<TestType>(42));
    REQUIRE(directInitialization2.isInt());
    CHECK(directInitialization2.asInt() == static_cast<TestType>(42));

    minjson::Value copyInitialization = static_cast<TestType>(42);
    REQUIRE(copyInitialization.isInt());
    CHECK(copyInitialization.asInt() == static_cast<TestType>(42));

    CHECK(directInitialization == directInitialization2);
    CHECK_FALSE(directInitialization != directInitialization2);
    CHECK(directInitialization == copyInitialization);
    CHECK_FALSE(directInitialization != copyInitialization);
  }

  SECTION("assignment") {
    minjson::Value v;
    v = static_cast<TestType>(42);
    REQUIRE(v.isInt());
    CHECK(v.asInt() == static_cast<TestType>(42));
  }
}


TEMPLATE_TEST_CASE("floating point value construction and assignment",
                   "[value][construction][assignment]",
                   float,
                   double) {
  SECTION("construction") {
    minjson::Value directInitialization{ static_cast<TestType>(3.14) };
    REQUIRE(directInitialization.isDouble());
    CHECK(directInitialization.asDouble() == static_cast<TestType>(3.14));

    minjson::Value directInitialization2(static_cast<TestType>(3.14));
    REQUIRE(directInitialization2.isDouble());
    CHECK(directInitialization2.asDouble() == static_cast<TestType>(3.14));

    minjson::Value copyInitialization = static_cast<TestType>(3.14);
    REQUIRE(copyInitialization.isDouble());
    CHECK(copyInitialization.asDouble() == static_cast<TestType>(3.14));

    CHECK(directInitialization == directInitialization2);
    CHECK_FALSE(directInitialization != directInitialization2);
    CHECK(directInitialization == copyInitialization);
    CHECK_FALSE(directInitialization != copyInitialization);
  }

  SECTION("assignment") {
    minjson::Value v;
    v = static_cast<TestType>(3.14);
    REQUIRE(v.isDouble());
    CHECK(v.asDouble() == static_cast<TestType>(3.14));
  }
}


TEMPLATE_TEST_CASE("string value construction and assignment",
                   "[value][construction][assignment]",
                   const char[],
                   const char[4],
                   const char *,
                   std::string_view,
                   std::string) {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdangling"
#endif
  TestType s = "abc";
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
  SECTION("construction") {
    minjson::Value directInitialization{ s };
    REQUIRE(directInitialization.isString());
    CHECK(directInitialization.asString() == s);

    minjson::Value directInitialization2(s);
    REQUIRE(directInitialization2.isString());
    CHECK(directInitialization2.asString() == s);

    minjson::Value copyInitialization = s;
    REQUIRE(copyInitialization.isString());
    CHECK(copyInitialization.asString() == s);

    CHECK(directInitialization == directInitialization2);
    CHECK_FALSE(directInitialization != directInitialization2);
    CHECK(directInitialization == copyInitialization);
    CHECK_FALSE(directInitialization != copyInitialization);
  }

  SECTION("assignment") {
    minjson::Value v;
    v = s;
    REQUIRE(v.isString());
    CHECK(v.asString() == s);
  }
}


TEST_CASE("array value construction and assignment", "[value][construction][assignment]") {
  const auto a = GENERATE(minjson::Array{}, minjson::Array{ 1, 2, 3 });
  SECTION("construction") {
    minjson::Value directInitialization{ a };
    REQUIRE(directInitialization.isArray());
    CHECK(directInitialization.asArray() == a);

    minjson::Value directInitialization2(a);
    REQUIRE(directInitialization2.isArray());
    CHECK(directInitialization2.asArray() == a);

    minjson::Value copyInitialization = a;
    REQUIRE(copyInitialization.isArray());
    CHECK(copyInitialization.asArray() == a);

    CHECK(directInitialization == directInitialization2);
    CHECK_FALSE(directInitialization != directInitialization2);
    CHECK(directInitialization == copyInitialization);
    CHECK_FALSE(directInitialization != copyInitialization);
  }

  SECTION("assignment") {
    minjson::Value v;
    v = a;
    REQUIRE(v.isArray());
    CHECK(v.asArray() == a);
  }
}


TEST_CASE("object value construction and assignment", "[value][construction][assignment]") {
  const auto o = GENERATE(minjson::Object{}, minjson::Object{ { "a", 42 }, { "b", "c" } });
  SECTION("construction") {
    minjson::Value directInitialization{ o };
    REQUIRE(directInitialization.isObject());
    CHECK(directInitialization.asObject() == o);

    minjson::Value directInitialization2(o);
    REQUIRE(directInitialization2.isObject());
    CHECK(directInitialization2.asObject() == o);

    minjson::Value copyInitialization = o;
    REQUIRE(copyInitialization.isObject());
    CHECK(copyInitialization.asObject() == o);

    CHECK(directInitialization == directInitialization2);
    CHECK_FALSE(directInitialization != directInitialization2);
    CHECK(directInitialization == copyInitialization);
    CHECK_FALSE(directInitialization != copyInitialization);
  }

  SECTION("assignment") {
    minjson::Value v;
    v = o;
    REQUIRE(v.isObject());
    CHECK(v.asObject() == o);
  }
}


TEST_CASE("moving into value", "[value][construction][assignment][!mayfail]") {
  SECTION("strings") {
    {
      std::string s = "test";
      minjson::Value v = std::move(s);
      CHECK(s.empty());
    }
    {
      std::string s = "test";
      minjson::Value v;
      v = std::move(s);
      CHECK(s.empty());
    }
  }

  SECTION("arrays") {
    {
      minjson::Array a{ 1, 2, 3 };
      minjson::Value v{ std::move(a) };
      CHECK(a.empty());
    }
    {
      minjson::Array a{ 1, 2, 3 };
      minjson::Value v;
      v = std::move(a);
      CHECK(a.empty());
    }
  }

  SECTION("objects") {
    {
      minjson::Object o{ { "foo", "bar"sv } };
      minjson::Value v{ std::move(o) };
      CHECK(o.empty());
    }
    {
      minjson::Object o{ { "foo", "bar"sv } };
      minjson::Value v;
      v = std::move(o);
      CHECK(o.empty());
    }
  }
}


TEST_CASE("value comparison", "[value][comparison]") {
  const minjson::Value v1 = 42;
  const minjson::Value v2 = v1;
  const minjson::Value u = "foo"sv;
  CAPTURE(v1, v2, u);

  CHECK(v1 == v1);
  CHECK(v1 == v2);
  CHECK_FALSE(v1 != v1);
  CHECK_FALSE(v1 != v2);

  CHECK(v1 != u);
  CHECK_FALSE(v1 == u);
}


TEST_CASE("resolve value", "[value][resolve]") {
  SECTION("scalar types") {
    const auto value = GENERATE(minjson::Value{ minjson::Null{} }, minjson::Value{ true }, minjson::Value{ 42 },
                                minjson::Value{ 3.14 }, minjson::Value{ "hello"sv });
    CHECK(value.resolve(0) == nullptr);
    CHECK(value.resolve("") == nullptr);
  }

  SECTION("array") {
    minjson::Value value = minjson::Array{ minjson::Null{}, true, 2 };
    {
      const auto resolved0 = value.resolve(0);
      REQUIRE(resolved0 != nullptr);
      CHECK(resolved0 == std::as_const(value).resolve(0));
      CHECK(resolved0->isNull());
    }
    {
      const auto resolved1 = value.resolve(1);
      REQUIRE(resolved1 != nullptr);
      CHECK(resolved1 == std::as_const(value).resolve(1));
      CHECK(resolved1->isBool());
      CHECK(resolved1->asBool() == true);
    }
    {
      const auto resolved2 = value.resolve(2);
      REQUIRE(resolved2 != nullptr);
      CHECK(resolved2 == std::as_const(value).resolve(2));
      CHECK(resolved2->isInt());
      CHECK(resolved2->asInt() == 2);
    }
    CHECK(value.resolve(3) == nullptr);
  }

  SECTION("object") {
    minjson::Value value = minjson::Object{ { "null", minjson::Null{} }, { "bool", true }, { "int", 2 } };
    {
      const auto resolvedNull = value.resolve("null");
      REQUIRE(resolvedNull != nullptr);
      CHECK(resolvedNull == std::as_const(value).resolve("null"));
      CHECK(resolvedNull->isNull());
    }
    {
      const auto resolvedBool = value.resolve(std::string{ "bool" });
      REQUIRE(resolvedBool != nullptr);
      CHECK(resolvedBool == std::as_const(value).resolve("bool"));
      CHECK(resolvedBool->isBool());
      CHECK(resolvedBool->asBool() == true);
    }
    {
      const auto resolvedInt = value.resolve(std::string{ "int" });
      REQUIRE(resolvedInt != nullptr);
      CHECK(resolvedInt == std::as_const(value).resolve("int"));
      CHECK(resolvedInt->isInt());
      CHECK(resolvedInt->asInt() == 2);
    }
    CHECK(value.resolve("inexistent") == nullptr);
  }

  SECTION("nested arrays") {
    // clang-format off
    minjson::Value value = minjson::Array{
      minjson::Array{
        minjson::Array{ 1, 2, 3 },
        "four"sv,
        "five"sv,
        "six"sv
      },
      minjson::Object{
        { "array", minjson::Array{
                    7.,
                    8.,
                    9.
        }}
      }
    };
    // clang-format on
    {
      const minjson::Value *resolved_0_0_0 = value.resolve(0, 0, 0);
      REQUIRE(resolved_0_0_0 != nullptr);
      CHECK(resolved_0_0_0 == std::as_const(value).resolve(0, 0, 0));
      CHECK(resolved_0_0_0->isInt());
      CHECK(resolved_0_0_0->asInt() == 1);
    }
    {
      const auto resolved_0_1 = value.resolve(0, 1);
      REQUIRE(resolved_0_1 != nullptr);
      CHECK(resolved_0_1 == std::as_const(value).resolve(0, 1));
      CHECK(resolved_0_1->isString());
      CHECK(resolved_0_1->asString() == "four"sv);
    }
    {
      const auto resolved_1_array_0 = value.resolve(1, "array", 0);
      REQUIRE(resolved_1_array_0 != nullptr);
      CHECK(resolved_1_array_0 == std::as_const(value).resolve(1, "array", 0));
      CHECK(resolved_1_array_0->isDouble());
      CHECK(resolved_1_array_0->asDouble() == 7.);
    }
  }

  SECTION("nested objects") {
    // clang-format off
    minjson::Value value = minjson::Object{
      { "array", minjson::Array{
        minjson::Object{ { "foo", "bar" }, { "baz","qux" } },
        2,
        3
      }},
      { "object", minjson::Object{
        { "nested", minjson::Object{
          { "xxx", 4. },
          { "yyy", 5. }
        }},
        { "something", "else"sv }
      }},
      { "null", minjson::Null{} }
    };
    // clang-format on
    {
      const auto resolved_array_0_foo = value.resolve("array", 0, "foo");
      REQUIRE(resolved_array_0_foo != nullptr);
      CHECK(resolved_array_0_foo == std::as_const(value).resolve("array", 0, "foo"));
      CHECK(resolved_array_0_foo->isString());
      CHECK(resolved_array_0_foo->asString() == "bar"sv);
    }
    {
      const auto resolved_object_nested_xxx = value.resolve("object", "nested", "xxx");
      REQUIRE(resolved_object_nested_xxx != nullptr);
      CHECK(resolved_object_nested_xxx == std::as_const(value).resolve("object", "nested", "xxx"));
      CHECK(resolved_object_nested_xxx->isDouble());
      CHECK(resolved_object_nested_xxx->asDouble() == 4.);
    }
    {
      const auto resolved_object_something = value.resolve("object", "something");
      REQUIRE(resolved_object_something != nullptr);
      CHECK(resolved_object_something == std::as_const(value).resolve("object", "something"));
      CHECK(resolved_object_something->isString());
      CHECK(resolved_object_something->asString() == "else"sv);
    }
  }
}
