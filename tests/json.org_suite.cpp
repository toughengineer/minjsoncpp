#include <minjsoncpp.h>

#include "utils.h"

#include <catch2/catch_all.hpp>

using namespace std::string_view_literals;

// test data from https://www.json.org/JSON_checker/:
// https://www.json.org/JSON_checker/test.zip

TEST_CASE("JSON.org suite: pass", "[parse][JSON.org][pass]") {
  SECTION("pass1.json") {
    const auto string = R"([
    "JSON Test Pattern pass1",
    {"object with 1 member":["array with 1 element"]},
    {},
    [],
    -42,
    true,
    false,
    null,
    {
        "integer": 1234567890,
        "real": -9876.543210,
        "e": 0.123456789e-12,
        "E": 1.234567890E+34,
        "":  23456789012E66,
        "zero": 0,
        "one": 1,
        "space": " ",
        "quote": "\"",
        "backslash": "\\",
        "controls": "\b\f\n\r\t",
        "slash": "/ & \/",
        "alpha": "abcdefghijklmnopqrstuvwyz",
        "ALPHA": "ABCDEFGHIJKLMNOPQRSTUVWYZ",
        "digit": "0123456789",
        "0123456789": "digit",
        "special": "`1~!@#$%^&*()_+-={':[,]}|;.</>?",
        "hex": "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A",
        "true": true,
        "false": false,
        "null": null,
        "array":[  ],
        "object":{  },
        "address": "50 St. James Street",
        "url": "http://www.JSON.org/",
        "comment": "// /* <!-- --",
        "# -- --> */": " ",
        " s p a c e d " :[1,2 , 3

,

4 , 5        ,          6           ,7        ],"compact":[1,2,3,4,5,6,7],
        "jsontext": "{\"object with 1 member\":[\"array with 1 element\"]}",
        "quotes": "&#34; \u0022 %22 0x22 034 &#x22;",
        "\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"
: "A key can be any string"
    },
    0.5 ,98.6
,
99.44
,

1066,
1e1,
0.1e1,
1e-1,
1e00,2e+00,2e-00
,"rosebud"])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }

  SECTION("pass2.json") {
    const auto string = R"([[[[[[[[[[[[[[[[[[["Not too deep"]]]]]]]]]]]]]]]]]]])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }

  SECTION("pass3.json") {
    const auto string = R"({
    "JSON Test Pattern pass3": {
        "The outermost value": "must be an object or array.",
        "In this test": "It is an object."
    }
}
)"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }
}


TEST_CASE("JSON.org suite: fail", "[parse][JSON.org][fail]") {
  SECTION("fail1.json") {
    const auto string = R"("A JSON payload should be an object or array, not a string.")"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
    CHECK(value.isString());
    // this test is supposed to successfully parse the input and verify that it's not a collection
    // but we consider any JSON value type to be valid JSON value
    CHECK_NOFAIL((value.isArray() || value.isObject()));
  }

  SECTION("fail2.json") {
    const auto string = R"(["Unclosed array")"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == string.size());
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == string.size());
    CHECK(issues[0].code == minjson::ParsingIssue::Code::UnexpectedEndOfInput);
  }

  SECTION("fail3.json") {
    const auto string = R"({unquoted_key: "keys must be quoted"})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 1);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail4.json") {
    const auto string = R"(["extra comma",])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 15);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 15);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail5.json") {
    const auto string = R"(["double extra comma",,])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 22);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 22);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail6.json") {
    const auto string = R"([   , "<-- missing value"])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 4);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 4);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail7.json") {
    const auto string = R"(["Comma after the close"],)"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::PartialSuccess);
    CHECK(parsedSize == 25);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 25);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::Other);
  }

  SECTION("fail8.json") {
    const auto string = R"(["Extra close"]])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::PartialSuccess);
    CHECK(parsedSize == 15);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 15);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::Other);
  }

  SECTION("fail9.json") {
    const auto string = R"({"Extra comma": true,})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 21);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 21);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail10.json") {
    const auto string = R"({"Extra value after close": true} "misplaced quoted value")"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::PartialSuccess);
    CHECK(parsedSize == 34);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 34);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::Other);
  }

  SECTION("fail11.json") {
    const auto string = R"({"Illegal expression": 1 + 2})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 25);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 25);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail12.json") {
    const auto string = R"({"Illegal invocation": alert()})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 23);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 23);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail13.json") {
    const auto string = R"({"Numbers cannot have leading zeroes": 013})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 40);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 40);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail14.json") {
    const auto string = R"({"Numbers cannot be hex": 0x14})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 27);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 27);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail15.json") {
    const auto string = R"(["Illegal backslash escape: \x15"])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 29);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 29);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail16.json") {
    const auto string = R"([\naked])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 1);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail17.json") {
    const auto string = R"(["Illegal backslash escape: \017"])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 29);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 29);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail18.json") {
    const auto string = R"([[[[[[[[[[[[[[[[[[[["Too deep"]]]]]]]]]]]]]]]]]]]])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
    // we do not explicitly impose limits on nesting depth
  }

  SECTION("fail19.json") {
    const auto string = R"({"Missing colon" null})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 17);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 17);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail20.json") {
    const auto string = R"({"Double colon":: null})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 16);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 16);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail21.json") {
    const auto string = R"({"Comma instead of colon", null})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 25);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 25);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail22.json") {
    const auto string = R"(["Colon instead of comma": false])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 25);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 25);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail23.json") {
    const auto string = R"(["Bad value", truth])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 17);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 17);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail24.json") {
    const auto string = "['single quote']"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 1);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail25.json") {
    const auto string = "[\"\ttab\tcharacter\tin\tstring\t\"]"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 2);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 2);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail26.json") {
    const auto string = R"(["tab\   character\   in\  string\  "])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 6);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 6);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail27.json") {
    const auto string = R"(["line
break"])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 6);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 6);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail28.json") {
    const auto string = R"(["line\
break"])"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 7);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 7);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail29.json") {
    const auto string = "[0e]"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 3);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 3);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail30.json") {
    const auto string = "[0e+]"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 4);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 4);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail31.json") {
    const auto string = "[0e+-1]"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 4);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 4);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }

  SECTION("fail32.json") {
    const auto string = R"({"Comma instead if closing brace": true,)"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == string.size());
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == string.size());
    CHECK(issues[0].code == minjson::ParsingIssue::Code::UnexpectedEndOfInput);
  }

  SECTION("fail33.json") {
    const auto string = R"(["mismatch"})"sv;

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 11);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].offset == 11);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
  }
}
