#include <minjsoncpp.h>

#include "utils.h"

#include <catch2/catch_all.hpp>

#include <climits>

using namespace std::string_view_literals;

TEST_CASE("parse values", "[parse]") {
  SECTION("null") {
    const auto before = GENERATE(""sv, "   "sv);
    const auto after = GENERATE(""sv, "   "sv);
    const auto string = before + "null"sv + after;
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(value.isNull());
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }

  SECTION("boolean") {
    using pair = std::pair<std::string_view, bool>;
    const auto [string, parsedValue] = GENERATE(pair{ "false"sv, false }, pair{ "true"sv, true });
    const auto before = GENERATE(""sv, "   "sv);
    const auto after = GENERATE(""sv, "   "sv);
    const auto stringWithWhitespaces = before + string + after;
    CAPTURE(stringWithWhitespaces);

    const auto [value, status, parsedSize, issues] = minjson::parse(stringWithWhitespaces);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(value.isBool());
    CHECK(value.asBool() == parsedValue);
    CHECK(parsedSize == stringWithWhitespaces.size());
    CHECK(issues.empty());
  }

  SECTION("integer") {
    const auto before = GENERATE(""sv, "   "sv);
    const auto after = GENERATE(""sv, "   "sv);
    const auto string = before + "42"sv + after;
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(value.isInt());
    CHECK(value.asInt() == 42);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }

  SECTION("decimal") {
    const auto before = GENERATE(""sv, "   "sv);
    const auto after = GENERATE(""sv, "   "sv);
    const auto string = before + "3.14"sv + after;
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(value.isDouble());
    CHECK(value.asDouble() == 3.14);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }

  SECTION("string") {
    const auto before = GENERATE(""sv, "   "sv);
    const auto after = GENERATE(""sv, "   "sv);
    const auto string = before + R"("hello")"sv + after;
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(value.isString());
    CHECK(value.asString() == "hello"sv);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }

  SECTION("array") {
    SECTION("general") {
      const auto before = GENERATE(""sv, "   "sv);
      const auto beforeMember = GENERATE(""sv, "\n  "sv);
      const auto afterMember = GENERATE(""sv, "   "sv);
      const auto after = GENERATE(""sv, "   "sv);
      auto append = [beforeMember, afterMember](std::string &s, std::string_view member) -> std::string & {
        s += beforeMember;
        s += member;
        s += afterMember;
        return s;
      };
      auto string = before + "["sv;
      append(string, "null"sv) += ',';
      append(string, "false"sv) += ',';
      append(string, "true"sv) += ',';
      append(string, "42"sv) += ',';
      append(string, "3.14"sv) += ',';
      append(string, R"("hello")"sv) += ',';
      append(string, "[]"sv) += ',';
      append(string, "{}"sv);
      string += "]"sv + after;
      CAPTURE(string);

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Success);
      CHECK(value.isArray());
      CHECK(value.asArray() ==
            minjson::Array{ minjson::Null{}, false, true, 42, 3.14, "hello", minjson::Array{}, minjson::Object{} });
      CHECK(parsedSize == string.size());
      CHECK(issues.empty());
    }

    SECTION("long array") {
      const auto string = R"([
 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
10,11,12,13,14,15,16,17,18,19,
20,21,22,23,24,25,26,27,28,29,
30,31,32,33,34,35,36,37,38,39,
40,41,42,43,44,45,46,47,48,49,
50,51,52,53,54,55,56,57,58,59,
60,61,62,63,64,65,66,67,68,69,
70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,
90,91,92,93,94,95,96,97,98,99
])"sv;
      const auto array =
        minjson::Array{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                        40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                        60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
                        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99 };

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Success);
      CHECK(value.isArray());
      CHECK(value.asArray() == array);
      CHECK(parsedSize == string.size());
      CHECK(issues.empty());
    }
  }

  SECTION("nested array") {
    const auto string = R"([1,[2,3],{"foo":"bar"}])"sv;
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(value.isArray());
    CHECK(value.asArray() == minjson::Array{ 1, minjson::Array{ 2, 3 }, minjson::Object{ { "foo", "bar" } } });
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }

  SECTION("object") {
    const auto before = GENERATE(""sv, "   "sv);
    const auto beforeKey = GENERATE(""sv, "\n  "sv);
    const auto afterKey = GENERATE(""sv, "  "sv);
    const auto beforeValue = GENERATE(""sv, "  "sv);
    const auto afterValue = GENERATE(""sv, "  "sv);
    const auto after = GENERATE(""sv, "   "sv);
    auto append = [beforeKey, afterKey, beforeValue, afterValue](std::string &s, std::string_view key,
                                                                 std::string_view value) -> std::string & {
      s += beforeKey;
      s += key;
      s += afterKey;
      s += ':';
      s += beforeValue;
      s += value;
      s += afterValue;
      return s;
    };
    auto string = before + "{"sv;
    append(string, R"("null")"sv, "null"sv) += ',';
    append(string, R"("false")"sv, "false"sv) += ',';
    append(string, R"("true")"sv, "true"sv) += ',';
    append(string, R"("integer")"sv, "42"sv) += ',';
    append(string, R"("decimal")"sv, "3.14"sv) += ',';
    append(string, R"("string")"sv, R"("hello")"sv) += ',';
    append(string, R"("array")"sv, "[]"sv) += ',';
    append(string, R"("object")"sv, "{}"sv);
    string += "}"sv + after;
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(value.isObject());
    CHECK(value.asObject() == minjson::Object{ { "null", minjson::Null{} },
                                               { "false", false },
                                               { "true", true },
                                               { "integer", 42 },
                                               { "decimal", 3.14 },
                                               { "string", "hello" },
                                               { "array", minjson::Array{} },
                                               { "object", minjson::Object{} } });
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }

  SECTION("nested object") {
    const auto string = R"({"foo":"bar","array":[1,2,3],"hello":{"there":"General Kenobi"}})"sv;
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(value.isObject());
    CHECK(value.asObject() == minjson::Object{ { "foo", "bar" },
                                               { "array", minjson::Array{ 1, 2, 3 } },
                                               { "hello", minjson::Object{ { "there", "General Kenobi" } } } });
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }
}


TEST_CASE("parse numbers", "[parse]") {
  SECTION("valid integers") {
    using pair = std::pair<std::string_view, int64_t>;
    const auto [string, expectedInteger] =
      GENERATE(pair{ "[0]"sv, 0 }, pair{ "[-0]"sv, 0 }, pair{ "[1]"sv, 1 }, pair{ "[-1]"sv, -1 },
               pair{ "[9223372036854775807]"sv, 9223372036854775807ll },  // max in64_t value
               pair{ "[-9223372036854775808]"sv, LLONG_MIN }              // min in64_t value
      );
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
    REQUIRE(value.isArray());
    const auto &array = value.asArray();
    REQUIRE(array.size() == 1);
    CHECK(array[0].isInt());
    CHECK(array[0].asInt() == expectedInteger);
  }

  using pair = std::pair<std::string_view, double>;
  SECTION("valid decimal numbers") {
    const auto [string, expectedDecimal] =
      GENERATE(pair{ "[0.0]"sv, 0. }, pair{ "[-0.0]"sv, 0. }, pair{ "[0e0]"sv, 0. }, pair{ "[-0E0]"sv, 0. },
               pair{ "[0e-0]"sv, 0. }, pair{ "[-0E+0]"sv, 0. }, pair{ "[1.0]"sv, 1. }, pair{ "[-1.0]"sv, -1. },
               pair{ "[1e0]"sv, 1. }, pair{ "[-1E0]"sv, -1. }, pair{ "[1e-0]"sv, 1. }, pair{ "[-1E+0]"sv, -1. },
               pair{ "[1.0e-0]"sv, 1. }, pair{ "[-1.0E+0]"sv, -1. });
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
    REQUIRE(value.isArray());
    const auto &array = value.asArray();
    REQUIRE(array.size() == 1);
    REQUIRE(array[0].isDouble());
    CHECK(array[0].asDouble() == expectedDecimal);
  }

  SECTION("valid decimal numbers with ridiculous amount of zeros") {
    const auto [string, expectedDecimal] =
      GENERATE(pair{ "[0.00000000000000000000]"sv, 0. }, pair{ "[-0.00000000000000000000]"sv, 0. },
               pair{ "[0e00000000000000000000]"sv, 0. }, pair{ "[-0E00000000000000000000]"sv, 0. },
               pair{ "[0e-00000000000000000000]"sv, 0. }, pair{ "[-0E+00000000000000000000]"sv, 0. },
               pair{ "[1.00000000000000000000]"sv, 1. }, pair{ "[-1.00000000000000000000]"sv, -1. },
               pair{ "[1e00000000000000000000]"sv, 1. }, pair{ "[-1E00000000000000000000]"sv, -1. },
               pair{ "[1e-00000000000000000000]"sv, 1. }, pair{ "[-1E+00000000000000000000]"sv, -1. },
               pair{ "[1.00000000000000000000e-00000000000000000000]"sv, 1. },
               pair{ "[-1.00000000000000000000E+00000000000000000000]"sv, -1. });

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
    REQUIRE(value.isArray());
    const auto &array = value.asArray();
    REQUIRE(array.size() == 1);
    REQUIRE(array[0].isDouble());
    CHECK(array[0].asDouble() == expectedDecimal);
  }

  SECTION("large integers beyond in64_t range") {
    const auto string = GENERATE("[9223372036854775808]"sv,  // max in64_t value + 1
                                 "[-9223372036854775809]"sv  // min in64_t value - 1
    );
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
    REQUIRE(value.isArray());
    const auto &array = value.asArray();
    REQUIRE(array.size() == 1);
    CHECK(array[0].isDouble());
  }
}


TEST_CASE("parse large integers beyond in64_t range", "[parse][!mayfail]") {
  using pair = std::pair<std::string_view, double>;
  const auto [string, expectedDecimalNumber] = GENERATE(
    // max in64_t value + 1 = exactly 2^63 = 9223372036854776808.0 = inexact fp64 9223372036854776000.0
    pair{ "[9223372036854775808]"sv, 9223372036854776000. },
    // min in64_t value - 1 = exactly -2^63 - 1 = -9223372036854775809.0 = inexact fp64 -9223372036854776000.0
    pair{ "[-9223372036854775809]"sv, -9223372036854776000. });
  CAPTURE(string);

  const auto [value, status, parsedSize, issues] = minjson::parse(string);
  INFO(PrintIssues{ issues });
  CHECK(status == minjson::ParsingResultStatus::Success);
  CHECK(parsedSize == string.size());
  CHECK(issues.empty());
  REQUIRE(value.isArray());
  const auto &array = value.asArray();
  REQUIRE(array.size() == 1);
  REQUIRE(array[0].isDouble());
  CHECK(array[0].asDouble() == expectedDecimalNumber);
}


TEST_CASE("parse duplicate object keys", "[parse][invalid json]") {
  const auto string = R"({
"same key": 42,
"same key": 3.14
})"sv;
  CAPTURE(string);

  SECTION("default") {
    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 30);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::DuplicateKeys);
    CHECK(issues[0].offset == 18);
  }

  SECTION("report") {
    minjson::ParsingOptions options;
    options.duplicateObjectKeys = minjson::ParsingOptions::Option::Report;
    const auto [value, status, parsedSize, issues] = minjson::parse(string, options);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::DuplicateKeys);
    CHECK(issues[0].offset == 18);
  }

  SECTION("ignore") {
    minjson::ParsingOptions options;
    options.duplicateObjectKeys = minjson::ParsingOptions::Option::Ignore;
    const auto [value, status, parsedSize, issues] = minjson::parse(string, options);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == string.size());
    CHECK(issues.empty());
  }
}


TEST_CASE("parse syntactically invalid JSON", "[parse][invalid json]") {
  SECTION("empty/whitespace string") {
    const auto string = GENERATE(""sv, "   "sv, "\t\t\t"sv, "\n\n\n"sv);
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == string.size());
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::UnexpectedEndOfInput);
    CHECK(issues[0].offset == string.size());
  }

  SECTION("arbitrary literals/naked text") {
    const auto string = GENERATE("Null"sv, "True"sv, "False"sv, "hello"sv);
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 0);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
    CHECK(issues[0].offset == 0);
  }

  SECTION("misspelled literals") {
    const auto string = GENERATE("nul0"sv, "truu"sv, "falSe"sv);
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == 3);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
    CHECK(issues[0].offset == 3);
  }

  SECTION("invalid numbers") {
    SECTION("invalid character") {
      const auto string =
        GENERATE("[ 01]"sv,  // '[]' is required because "01"/"-01" is parsed as valid number 0
                 "[-01]"sv,  // followed by something else
                 "   +1"sv,
                 "  - 1"sv,  // whitespace between '-' and number
                 // whitespaces between different parts:
                 "[1 .0]"sv, " 1. 0"sv, "[1 e0]"sv, " 1e 0"sv, " 1e +0"sv, " 1e -0"sv, "1e+ 0"sv, "1e- 0"sv);
      CAPTURE(string);

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == 3);
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
      CHECK(issues[0].offset == 3);
    }

    SECTION("unexpected end of input") {
      const auto string = GENERATE("0."sv,   // incomplete fraction part
                                   "0e"sv,   // incomplete exponent part
                                   "0e+"sv,  // incomplete exponent part
                                   "0e-"sv,  // incomplete exponent part
                                   "-"sv     // incomplete integer part
      );
      CAPTURE(string);

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == string.size());
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::UnexpectedEndOfInput);
      CHECK(issues[0].offset == string.size());
    }

    SECTION("out of range") {
      const auto string = GENERATE("1e309"sv, "-1e309"sv);
      CAPTURE(string);

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == string.size());
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::ParsedNumberOutOfRange);
      CHECK(issues[0].offset == 0);
    }
  }

  SECTION("invalid strings") {
    const auto string = GENERATE(R"("no closing quote)"sv, R"("closing quote is escaped\")"sv);
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Failure);
    CHECK(parsedSize == string.size());
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::UnexpectedEndOfInput);
    CHECK(issues[0].offset == string.size());
  }

  SECTION("invalid arrays") {
    SECTION("incomplete") {
      const auto string = GENERATE(R"(["no closing bracket")"sv, R"(["no value after comma",)"sv);
      CAPTURE(string);

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == string.size());
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::UnexpectedEndOfInput);
      CHECK(issues[0].offset == string.size());
    }

    SECTION("invalid separator, missing member/extra comma") {
      const auto string =
        GENERATE(R"(["no comma"       "between members"])"sv, R"(["wrong separator"; "between members"])"sv,
                 R"(["missing value", ])"sv, R"(["missing value", , "or extra comma"])"sv,
                 R"([1, 2, 3, 4, 5, 6,, 7])"sv, R"([1, 2, 3, 4, 5, 6,])"sv);
      CAPTURE(string);

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == 18);
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
      CHECK(issues[0].offset == 18);
    }
  }

  SECTION("invalid objects") {
    SECTION("incomplete") {
      const auto string = GENERATE(R"({"no colon after key")"sv, R"({"no value after colon":)"sv,
                                   R"({"no closing brace":null)"sv, R"({"no member after comma":null,)"sv);
      CAPTURE(string);

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == string.size());
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::UnexpectedEndOfInput);
      CHECK(issues[0].offset == string.size());
    }

    SECTION("invalid key data type") {
      const auto string = GENERATE(R"({null:"non-string key"})"sv, R"({42:"non-string key"})"sv,
                                   R"({[]:"non-string key"})"sv, R"({{}:"non-string key"})"sv);
      CAPTURE(string);

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == 1);
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
      CHECK(issues[0].offset == 1);
    }

    SECTION("invalid separators, missing member/extra comma") {
      // clang-format off
      const auto string =
        GENERATE(R"({"no comma":null       "between members":null})"sv,
                 R"({"wrong separator":null; "between members"})"sv,
                 R"({"wrong key separator" = null})"sv,
                 R"({"missing member":null,, "or extra comma":null})"sv,
                 R"({"a":1, "b":2, "c":3,  , "d":4})"sv,
                 R"({"a":1, "b":2, "c":3,  })"sv
                 );
      // clang-format on
      CAPTURE(string);

      const auto [value, status, parsedSize, issues] = minjson::parse(string);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == 23);
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
      CHECK(issues[0].offset == 23);
    }
  }
}


TEST_CASE("parse garbage after valid value", "[parse]") {
  SECTION("default") {
    const auto string = GENERATE("  42 something else"sv, "[42] something else"sv, "    01"sv, "   -01"sv);
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] = minjson::parse(string);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::PartialSuccess);
    CHECK(parsedSize == 5);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].code == minjson::ParsingIssue::Code::Other);
    CHECK(issues[0].offset == 5);
  }

  SECTION("stop after valid JSON value") {
    const auto string = GENERATE("  42something else"sv, "[42]something else"sv, "   01"sv, "  -01"sv);
    CAPTURE(string);

    const auto [value, status, parsedSize, issues] =
      minjson::parse(string, {}, minjson::ParsingMode::StopAfterValueEnds);
    INFO(PrintIssues{ issues });
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == 4);
    CHECK(issues.empty());
  }
}
