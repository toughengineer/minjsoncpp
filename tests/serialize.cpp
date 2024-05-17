#include <minjsoncpp.h>

#include "utils.h"

#include <catch2/catch_all.hpp>

using namespace std::string_view_literals;

TEST_CASE("serialize values", "[serialize]") {
  SECTION("scalar values") {
    CHECK(minjson::serializeToString(minjson::Value{ minjson::Null{} }) == "null"sv);
    CHECK(minjson::serializeToString(minjson::Value{ true }) == "true"sv);
    CHECK(minjson::serializeToString(minjson::Value{ false }) == "false"sv);
    CHECK(minjson::serializeToString(minjson::Value{ 42 }) == "42"sv);
    CHECK(minjson::serializeToString(minjson::Value{ 3.14 }) == "3.14"sv);
    CHECK(minjson::serializeToString(minjson::Value{ "foo bar"sv }) == "\"foo bar\""sv);
  }

  SECTION("collections") {
    CHECK(minjson::serializeToString(minjson::Value{ minjson::Array{ 1, 2, 3 } }) == "[1,2,3]"sv);
    CHECK(minjson::serializeToString(minjson::Value{ minjson::Object{ { "foo", "bar" } } }) == R"({"foo":"bar"})"sv);
  }

  SECTION("nested collections") {
    CHECK(minjson::serializeToString(minjson::Value{ minjson::Array{ 1, 2, minjson::Object{ { "foo", "bar" } } } }) ==
          R"([1,2,{"foo":"bar"}])"sv);

    CHECK(minjson::serializeToString(minjson::Value{ minjson::Object{ { "foo", minjson::Array{ 1, 2, 3 } } } }) ==
          R"({"foo":[1,2,3]})"sv);
  }

  SECTION("serialize object members in key-ascending order") {
    minjson::SerializationOptions options;
    options.sortObjectKeys = true;

    CHECK(minjson::serializeToString(minjson::Value{ minjson::Object{ { "1", 1 } } }, options) == R"({"1":1})"sv);

    const minjson::Value value = minjson::Object{ { "1", 1 }, { "2", 2 }, { "a", 3 }, { "b", 4 } };
    CAPTURE(value);
    CHECK(minjson::serializeToString(value, options) == R"({"1":1,"2":2,"a":3,"b":4})"sv);
  }
}


TEST_CASE("serialization formatting: indentation", "[serialize][formatting]") {
  SECTION("default indentation options") {
    minjson::SerializationOptions options{ 5 };

    REQUIRE(options.indent == 5);
    REQUIRE(options.indentationChar == minjson::SerializationOptions::IndentationChar::Space);
  }

  SECTION("scalar values are not indented") {
    minjson::SerializationOptions options;
    options.indent = 4;

    CHECK(minjson::serializeToString(minjson::Value{ minjson::Null{} }, options) == "null"sv);
    CHECK(minjson::serializeToString(minjson::Value{ true }, options) == "true"sv);
    CHECK(minjson::serializeToString(minjson::Value{ false }, options) == "false"sv);
    CHECK(minjson::serializeToString(minjson::Value{ 42 }, options) == "42"sv);
    CHECK(minjson::serializeToString(minjson::Value{ 3.14 }, options) == "3.14"sv);
    CHECK(minjson::serializeToString(minjson::Value{ "foo bar"sv }, options) == "\"foo bar\""sv);
  }

  SECTION("simple indentation") {
    SECTION("3 spaces") {
      minjson::SerializationOptions options;
      options.indent = 3;

      SECTION("array") {
        const minjson::Value a = minjson::Array{ 1, 2, 3 };
        CAPTURE(a);
        CHECK(minjson::serializeToString(a, options) == R"([
   1,
   2,
   3
])"sv);
      }

      SECTION("object") {
        const minjson::Value o = minjson::Object{ { "foo", "bar" } };
        CAPTURE(o);
        CHECK(minjson::serializeToString(o, options) == R"({
   "foo":"bar"
})"sv);
      }

      SECTION("nested array") {
        const minjson::Value nested1 = minjson::Object{ { "foo", minjson::Array{ 1, 2, 3 } } };
        CAPTURE(nested1);
        CHECK(minjson::serializeToString(nested1, options) == R"({
   "foo":[
      1,
      2,
      3
   ]
})"sv);
      }

      SECTION("nested object") {
        const minjson::Value nested2 = minjson::Array{ 1, 2, minjson::Object{ { "foo", "bar" } } };
        CAPTURE(nested2);
        CHECK(minjson::serializeToString(nested2, options) == R"([
   1,
   2,
   {
      "foo":"bar"
   }
])"sv);
      }
    }

    SECTION("2 tabs") {
      minjson::SerializationOptions options;
      options.indent = 2;
      options.indentationChar = minjson::SerializationOptions::IndentationChar::Tab;

      SECTION("array") {
        const minjson::Value a = minjson::Array{ 1, 2, 3 };
        CAPTURE(a);
        CHECK(minjson::serializeToString(a, options) == "[\n\t\t1,\n\t\t2,\n\t\t3\n]"sv);
      }

      SECTION("object") {
        const minjson::Value o = minjson::Object{ { "foo", "bar" } };
        CAPTURE(o);
        CHECK(minjson::serializeToString(o, options) == "{\n\t\t\"foo\":\"bar\"\n}"sv);
      }

      SECTION("nested array") {

        const minjson::Value nested1 = minjson::Object{ { "foo", minjson::Array{ 1, 2, 3 } } };
        CAPTURE(nested1);
        CHECK(minjson::serializeToString(nested1, options) ==
              "{\n\t\t\"foo\":[\n\t\t\t\t1,\n\t\t\t\t2,\n\t\t\t\t3\n\t\t]\n}"sv);
      }

      SECTION("nested object") {
        const minjson::Value nested2 = minjson::Array{ 1, 2, minjson::Object{ { "foo", "bar" } } };
        CAPTURE(nested2);
        CHECK(minjson::serializeToString(nested2, options) ==
              "[\n\t\t1,\n\t\t2,\n\t\t{\n\t\t\t\t\"foo\":\"bar\"\n\t\t}\n]"sv);
      }
    }
  }
}

TEST_CASE("serialization formatting: newlines", "[serialize][formatting]") {
  minjson::SerializationOptions options;
  options.newline.separator = "\n"sv;
  options.newline.afterObjectOpeningBrace = false;
  options.newline.beforeObjectClosingBrace = false;
  options.newline.afterObjectMemberKey = false;
  options.newline.beforeObjectMemberValue = false;
  options.newline.beforeObjectMemberCollectionValue = false;
  options.newline.beforeObjectMemberSeparator = false;
  options.newline.afterObjectMemberSeparator = false;
  options.newline.afterArrayOpeningBracket = false;
  options.newline.beforeArrayClosingBracket = false;
  options.newline.beforeArrayMemberSeparator = false;
  options.newline.afterArrayMemberSeparator = false;

  SECTION("around '{', '}' and ':'") {
    const minjson::Value value = minjson::Object{ { "foo", "bar"sv } };
    CAPTURE(value);

    SECTION("after '{'") {
      options.newline.afterObjectOpeningBrace = true;

      CHECK(minjson::serializeToString(value, options) == R"({
"foo":"bar"})"sv);
    }

    SECTION("before '}'") {
      options.newline.beforeObjectClosingBrace = true;
      CHECK(minjson::serializeToString(value, options) == R"({"foo":"bar"
})"sv);
    }

    SECTION("after key before ':'") {
      options.newline.afterObjectMemberKey = true;
      CHECK(minjson::serializeToString(value, options) == R"({"foo"
:"bar"})"sv);
    }

    SECTION("after ':' before value") {
      options.newline.beforeObjectMemberValue = true;
      CHECK(minjson::serializeToString(value, options) == R"({"foo":
"bar"})"sv);
    }
  }

  SECTION("between '{' and '}' for empty objects") {
    const minjson::Value emptyObject = minjson::Object{};
    options.emptyObject = {};

    SECTION("after '{'") {
      options.newline.afterObjectOpeningBrace = true;
      CHECK(minjson::serializeToString(emptyObject, options) == "{\n}"sv);
    }

    SECTION("before '}'") {
      options.newline.beforeObjectClosingBrace = true;
      CHECK(minjson::serializeToString(emptyObject, options) == "{\n}"sv);
    }
  }

  SECTION("after ':' before collection") {
    options.newline.beforeObjectMemberCollectionValue = true;

    SECTION("before array") {
      const minjson::Value value = minjson::Object{ { "foo", "bar"sv } };
      CAPTURE(value);
      CHECK(minjson::serializeToString(value, options) == R"({"foo":"bar"})"sv);
    }

    SECTION("before array") {
      const minjson::Value objectWithNestedArray = minjson::Object{ { "foo", minjson::Array{ "bar"sv } } };
      CAPTURE(objectWithNestedArray);
      CHECK(minjson::serializeToString(objectWithNestedArray, options) == R"({"foo":
["bar"]})"sv);
    }

    SECTION("before object") {
      const minjson::Value objectWithNestedObject = minjson::Object{ { "foo", minjson::Object{ { "bar", "baz"sv } } } };
      CAPTURE(objectWithNestedObject);
      CHECK(minjson::serializeToString(objectWithNestedObject, options) == R"({"foo":
{"bar":"baz"}})"sv);
    }
  }

  SECTION("',' in objects") {
    const minjson::Value objectWithTwoMembers = minjson::Object{ { "1", "foo"sv }, { "2", "bar"sv } };
    CAPTURE(objectWithTwoMembers);

    SECTION("before ','") {
      options.newline.beforeObjectMemberSeparator = true;
      options.sortObjectKeys = true;
      CHECK(minjson::serializeToString(objectWithTwoMembers, options) == R"({"1":"foo"
,"2":"bar"})"sv);
    }

    SECTION("after ','") {
      options.newline.afterObjectMemberSeparator = true;
      options.sortObjectKeys = true;
      CHECK(minjson::serializeToString(objectWithTwoMembers, options) == R"({"1":"foo",
"2":"bar"})"sv);
    }
  }

  SECTION("around '[', ']' and ',' in arrays") {
    const minjson::Value value = minjson::Array{ "foo"sv, "bar"sv };
    CAPTURE(value);

    SECTION("after '['") {
      options.newline.afterArrayOpeningBracket = true;
      CHECK(minjson::serializeToString(value, options) == R"([
"foo","bar"])"sv);
    }

    SECTION("after '['") {
      options.newline.beforeArrayClosingBracket = true;
      CHECK(minjson::serializeToString(value, options) == R"(["foo","bar"
])"sv);
    }

    SECTION("after ','") {
      options.newline.beforeArrayMemberSeparator = true;
      CHECK(minjson::serializeToString(value, options) == R"(["foo"
,"bar"])"sv);
    }

    SECTION("after ','") {
      options.newline.afterArrayMemberSeparator = true;
      CHECK(minjson::serializeToString(value, options) == R"(["foo",
"bar"])"sv);
    }
  }

  SECTION("between '[' and ']' for empty arrays") {
    const minjson::Value emptyArray = minjson::Array{};
    options.emptyArray = {};

    SECTION("after '['") {
      options.newline.afterArrayOpeningBracket = true;
      CHECK(minjson::serializeToString(emptyArray, options) == "[\n]"sv);
    }

    SECTION("before ']'") {
      options.newline.beforeArrayClosingBracket = true;
      CHECK(minjson::serializeToString(emptyArray, options) == "[\n]"sv);
    }
  }
}

TEST_CASE("serialization formatting: custom literals and structure characters", "[serialize][formatting]") {
  minjson::SerializationOptions options;

  SECTION("custom literals") {
    SECTION("null") {
      options.nullLiteral = "Null"sv;
      const minjson::Value value = minjson::Array{ minjson::Null{} };
      CAPTURE(value);
      CHECK(minjson::serializeToString(value, options) == "[Null]"sv);
    }

    SECTION("false") {
      options.falseLiteral = "False"sv;
      const minjson::Value value = minjson::Array{ false };
      CAPTURE(value);
      CHECK(minjson::serializeToString(value, options) == "[False]"sv);
    }

    SECTION("true") {
      options.trueLiteral = "True"sv;
      const minjson::Value value = minjson::Array{ true };
      CAPTURE(value);
      CHECK(minjson::serializeToString(value, options) == "[True]"sv);
    }
  }

  SECTION("object structure characters") {
    const minjson::Value value = minjson::Object{ { "foo", minjson::Object{} } };
    CAPTURE(value);

    SECTION("empty object") {
      options.emptyObject = "()"sv;
      CHECK(minjson::serializeToString(value, options) == R"({"foo":()})"sv);
    }

    SECTION("opening '{'") {
      options.objectOpeningBrace = "(";
      CHECK(minjson::serializeToString(value, options) == R"(("foo":{}})"sv);

      options.emptyObject = {};
      CHECK(minjson::serializeToString(value, options) == R"(("foo":(}})"sv);
    }

    SECTION("closing '}'") {
      options.objectClosingBrace = ")";
      CHECK(minjson::serializeToString(value, options) == R"({"foo":{}))"sv);

      options.emptyObject = {};
      CHECK(minjson::serializeToString(value, options) == R"({"foo":{)))"sv);
    }

    SECTION("key-value separator ':'") {
      options.objectKeyValueSeparator = "=";
      CHECK(minjson::serializeToString(value, options) == R"({"foo"={}})"sv);
    }
  }

  SECTION("object member separator ','") {
    options.objectMemberSeparator = ";";
    options.sortObjectKeys = true;
    const minjson::Value value = minjson::Object{ { "1", 1 }, { "2", 2 } };
    CAPTURE(value);
    CHECK(minjson::serializeToString(value, options) == R"({"1":1;"2":2})"sv);
  }

  SECTION("array structure characters") {
    const minjson::Value value = minjson::Array{ 1, 2, minjson::Array{} };
    CAPTURE(value);

    SECTION("empty array") {
      options.emptyArray = "()";
      CHECK(minjson::serializeToString(value, options) == "[1,2,()]"sv);
    }

    SECTION("opening '['") {
      options.arrayOpeningBracket = "(";
      CHECK(minjson::serializeToString(value, options) == "(1,2,[]]"sv);

      options.emptyArray = {};
      CHECK(minjson::serializeToString(value, options) == "(1,2,(]]"sv);
    }

    SECTION("closing ']'") {
      options.arrayClosingBracket = ")";
      CHECK(minjson::serializeToString(value, options) == "[1,2,[])"sv);

      options.emptyArray = {};
      CHECK(minjson::serializeToString(value, options) == "[1,2,[))"sv);
    }

    SECTION("separator ','") {
      options.arrayMemberSeparator = ";";
      CHECK(minjson::serializeToString(value, options) == "[1;2;[]]"sv);
    }
  }

  SECTION("string quotation marks '\"'") {
    const minjson::Value value = minjson::Array{ "foo"sv };
    CAPTURE(value);

    SECTION("opening '\"'") {
      options.openingStringQuotation = "\'";
      CHECK(minjson::serializeToString(value, options) == R"(['foo"])"sv);
    }

    SECTION("closing '\"'") {
      options.closingStringQuotation = "\'";
      CHECK(minjson::serializeToString(value, options) == R"(["foo'])"sv);
    }
  }
}
