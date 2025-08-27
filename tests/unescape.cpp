#include <minjsoncpp.h>

#include "utils.h"

#include "catch2/catch_amalgamated.hpp"

using namespace std::string_view_literals;

TEST_CASE("unescape/parse string without escapes", "[unescape][parse]") {
  SECTION("empty string is unchanged") {
    SECTION("unescape") {
      CHECK(minjson::unescape(""sv) == ""sv);
      CHECK(minjson::unescape(""sv, minjson::UnescapeMode::Relaxed) == ""sv);
      CHECK(minjson::unescape(""sv, minjson::UnescapeMode::Strict) == ""sv);
    }

    SECTION("parse") {
      const auto [value, status, parsedSize, issues] = minjson::parse(R"("")"sv);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Success);
      CHECK(value.isString());
      CHECK(value.asString() == ""sv);
      CHECK(parsedSize == 2);
      CHECK(issues.empty());
    }
  }

  SECTION("strings without special characters are unchanged") {
    // "utf8 2 byte code point £ 3 byte code point € 4 byte code point 😀 characters"
    const auto string =
      "utf8 2 byte code point \xC2\xA3 3 byte code point \xE2\x82\xAC 4 byte code point \xF0\x9F\x98\x80 characters"sv;

    SECTION("unescape") {
      CAPTURE(string);
      CHECK(minjson::unescape(string) == string);
      CHECK(minjson::unescape(string, minjson::UnescapeMode::Relaxed) == string);
      CHECK(minjson::unescape(string, minjson::UnescapeMode::Strict) == string);
    }

    SECTION("parse") {
      CAPTURE(string);
      const auto jsonString = getQuoted(string);
      const auto [value, status, parsedSize, issues] = minjson::parse(jsonString);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Success);
      CHECK(value.isString());
      CHECK(value.asString() == string);
      CHECK(parsedSize == jsonString.size());
      CHECK(issues.empty());
    }
  }

  SECTION("strings with ASCII control characters and \"") {
    const auto asciiControlsString =
      "null \0 tab \t carriage return \r line feed \n whatever this is \xf quotation mark \" chars"sv;

    SECTION("unchanged in relaxed mode") {
      CAPTURE(asciiControlsString);
      CHECK(minjson::unescape(asciiControlsString, minjson::UnescapeMode::Relaxed) == asciiControlsString);
    }


    SECTION("error in strict mode and during parsing") {
      SECTION("unescape") {
        CAPTURE(asciiControlsString);
        CHECK(minjson::unescape(asciiControlsString).empty());
        CHECK(minjson::unescape(asciiControlsString, minjson::UnescapeMode::Strict).empty());
      }

      SECTION("parse") {
        CAPTURE(asciiControlsString);
        const auto [value, status, parsedSize, issues] = minjson::parse(getQuoted(asciiControlsString));
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Failure);
        CHECK(parsedSize == 6);
        REQUIRE(issues.size() == 1);
        CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
        CHECK(issues[0].offset == 6);
      }


      SECTION("checking individual characters") {
        auto dummy = [](std::string_view) {};  // dummy sink
        constexpr auto DoNotReplaceSurrogates = minjson::impl::DoNotReplaceSurrogates;

        SECTION("unescape string containing \"") {
          CHECK(minjson::impl::unescape(dummy, R"(invalid " quotation mark character)"sv, minjson::UnescapeMode::Strict,
                                        DoNotReplaceSurrogates) == 8);
        }

        const auto invalidAsciiCharsString = GENERATE(
          "invalid \0 null character"sv, "invalid \t tab character"sv, "invalid \r carriage return character"sv,
          "invalid \n line feed character"sv, "invalid \xf whatever this character is"sv);
        CAPTURE(invalidAsciiCharsString);

        SECTION("unescape") {
          CAPTURE(invalidAsciiCharsString);
          CHECK(minjson::impl::unescape(dummy, invalidAsciiCharsString, minjson::UnescapeMode::Strict,
                                        DoNotReplaceSurrogates) == 8);
        }

        SECTION("parse") {
          CAPTURE(invalidAsciiCharsString);
          const auto [value, status, parsedSize, issues] = minjson::parse(getQuoted(invalidAsciiCharsString));
          INFO(PrintIssues{ issues });
          CHECK(status == minjson::ParsingResultStatus::Failure);
          CHECK(parsedSize == 9);
          REQUIRE(issues.size() == 1);
          CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
          CHECK(issues[0].offset == 9);
        }
      }
    }
  }
}


TEST_CASE("unescape/parse string with valid escapes", "[unescape][parse]") {
  using pair = std::pair<std::string_view, std::string_view>;
  SECTION("common escapes") {
    const auto [string, unescapedString] =
      GENERATE(pair{ R"(null \u0000 character)"sv, "null \0 character"sv },
               pair{ R"(backspace \b character)"sv, "backspace \b character"sv },
               pair{ R"(form feed \f character)"sv, "form feed \f character"sv },
               pair{ R"(line feed \n character)"sv, "line feed \n character"sv },
               pair{ R"(carriage return \r character)"sv, "carriage return \r character"sv },
               pair{ R"(tab \t character)"sv, "tab \t character"sv },
               pair{ R"(whatever this \u000f character is)"sv, "whatever this \xf character is"sv },
               pair{ R"(quotation mark \" character)"sv, R"(quotation mark " character)"sv },
               pair{ R"(solidus \/ character)"sv, "solidus / character"sv },
               pair{ R"(reverse solidus \\ character)"sv, R"(reverse solidus \ character)"sv });

    SECTION("unescape") {
      CAPTURE(string);
      CHECK(minjson::unescape(string) == unescapedString);
    }

    SECTION("parse") {
      const auto jsonString = getQuoted(string);
      CAPTURE(jsonString);
      const auto [value, status, parsedSize, issues] = minjson::parse(jsonString);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Success);
      CHECK(value.isString());
      CHECK(value.asString() == unescapedString);
      CHECK(parsedSize == jsonString.size());
      CHECK(issues.empty());
    }
  }

  SECTION("unicode escapes") {
    const auto [string, unescapedString] =
      GENERATE(pair{ R"(pound sign \u00a3 character)"sv, "pound sign \xC2\xA3 character"sv },
               pair{ R"(pound sign \u00A3 character)"sv, "pound sign \xC2\xA3 character"sv },
               pair{ R"(euro sign \u20ac character)"sv, "euro sign \xE2\x82\xAC character"sv },
               pair{ R"(euro sign \u20AC character)"sv, "euro sign \xE2\x82\xAC character"sv });

    SECTION("unescape") {
      CAPTURE(string);
      CHECK(minjson::unescape(string) == unescapedString);
    }

    SECTION("parse") {
      const auto jsonString = getQuoted(string);
      CAPTURE(jsonString);
      const auto [value, status, parsedSize, issues] = minjson::parse(jsonString);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Success);
      CHECK(value.isString());
      CHECK(value.asString() == unescapedString);
      CHECK(parsedSize == jsonString.size());
      CHECK(issues.empty());
    }
  }

  SECTION("utf16 surrogates escapes") {
    const auto string = GENERATE(R"(whatever this emoji \ud83d\ude00 character is)"sv,
                                 R"(whatever this emoji \ud83D\uDe00 character is)"sv);
    const auto unescapedString = "whatever this emoji \xF0\x9F\x98\x80 character is"sv;

    SECTION("unescape") {
      CAPTURE(string);
      CHECK(minjson::unescape(string) == unescapedString);
    }

    SECTION("parse") {
      const auto jsonString = getQuoted(string);
      CAPTURE(jsonString);
      const auto [value, status, parsedSize, issues] = minjson::parse(jsonString);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Success);
      CHECK(value.isString());
      CHECK(value.asString() == unescapedString);
      CHECK(parsedSize == jsonString.size());
      CHECK(issues.empty());
    }
  }
}


TEST_CASE("unescape/parse string with unpaired utf16 surrogates", "[unescape][parse][meh]") {
  SECTION("non-consecutive high and low surrogates") {
    const auto string = R"(unpaired surrogates \ud83d \ude00 characters)"sv;
    const auto unescapedString = "unpaired surrogates \xED\xA0\xBD \xED\xB8\x80 characters"sv;
    // replacing with replacement character �
    const auto unescapedStringWithReplacedSurrogates = "unpaired surrogates \xEF\xBF\xBD \xEF\xBF\xBD characters"sv;

    SECTION("unescape") {
      CAPTURE(string, unescapedString, unescapedStringWithReplacedSurrogates);

      CHECK(minjson::unescape(string) == unescapedString);
      CHECK(minjson::unescape(string, {}, U'\xFFFD') == unescapedStringWithReplacedSurrogates);
    }

    SECTION("parse") {
      const auto jsonString = getQuoted(string);
      CAPTURE(jsonString, unescapedString, unescapedStringWithReplacedSurrogates);

      SECTION("default") {
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == unescapedString);
        CHECK(parsedSize == jsonString.size());
        CHECK(issues.empty());
      }

      SECTION("replace unpaired surrogates") {
        minjson::ParsingOptions options;
        options.replaceInvalidUtf16Surrogates = true;
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == unescapedStringWithReplacedSurrogates);
        CHECK(parsedSize == jsonString.size());
        REQUIRE(issues.empty());
      }


      SECTION("report unpaired surrogates") {
        minjson::ParsingOptions options;
        options.unpairedUtf16Surrogates = minjson::ParsingOptions::Option::Report;
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == unescapedString);
        CHECK(parsedSize == jsonString.size());
        REQUIRE(issues.size() == 2);
        CHECK(issues[0].code == minjson::ParsingIssue::Code::StringContainsUnpairedUtf16HighSurrogate);
        CHECK(issues[0].offset == 21);
        CHECK(issues[1].code == minjson::ParsingIssue::Code::StringContainsUnpairedUtf16LowSurrogate);
        CHECK(issues[1].offset == 28);
      }


      SECTION("fail on unpaired surrogates") {
        minjson::ParsingOptions options;
        options.unpairedUtf16Surrogates = minjson::ParsingOptions::Option::Fail;
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Failure);
        CHECK(parsedSize == 28);
        REQUIRE(issues.size() == 1);
        CHECK(issues[0].code == minjson::ParsingIssue::Code::StringContainsUnpairedUtf16HighSurrogate);
        CHECK(issues[0].offset == 21);
      }
    }
  }

  SECTION("unpaired surrogates") {
    using tuple = std::tuple<std::string_view, std::string_view, std::string_view, minjson::ParsingIssue::Code>;
    const auto [string, unescapedString, unescapedStringWithReplacedSurrogates, parsingIssueCode] =
      GENERATE(tuple{ R"(unpaired surrogate \ud83d)"sv,  // at the end of the string
                      "unpaired surrogate \xED\xA0\xBD"sv,
                      "unpaired surrogate \xEF\xBF\xBD"sv,  // replacing with replacement character �
                      minjson::ParsingIssue::Code::StringContainsUnpairedUtf16HighSurrogate },
               tuple{ R"(unpaired surrogate \ude00)"sv,  // at the end of the string
                      "unpaired surrogate \xED\xB8\x80"sv,
                      "unpaired surrogate \xEF\xBF\xBD"sv,  // replacing with replacement character �
                      minjson::ParsingIssue::Code::StringContainsUnpairedUtf16LowSurrogate },
               tuple{ R"(unpaired surrogate \ud83d\u20ac followed by euro sign)"sv,
                      "unpaired surrogate \xED\xA0\xBD\xE2\x82\xAC followed by euro sign"sv,
                      // replacing with replacement character �
                      "unpaired surrogate \xEF\xBF\xBD\xE2\x82\xAC followed by euro sign"sv,
                      minjson::ParsingIssue::Code::StringContainsUnpairedUtf16HighSurrogate });

    SECTION("unescape") {
      CAPTURE(string, unescapedString, unescapedStringWithReplacedSurrogates);

      CHECK(minjson::unescape(string) == unescapedString);
      CHECK(minjson::unescape(string, {}, U'\xFFFD') == unescapedStringWithReplacedSurrogates);
    }

    SECTION("parse") {
      const auto jsonString = getQuoted(string);
      CAPTURE(jsonString, unescapedString, unescapedStringWithReplacedSurrogates);

      SECTION("default") {
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == unescapedString);
        CHECK(parsedSize == jsonString.size());
        CHECK(issues.empty());
      }

      SECTION("replace unpaired surrogates") {
        minjson::ParsingOptions options;
        options.replaceInvalidUtf16Surrogates = true;
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == unescapedStringWithReplacedSurrogates);
        CHECK(parsedSize == jsonString.size());
        REQUIRE(issues.empty());
      }

      SECTION("report unpaired surrogates") {
        minjson::ParsingOptions options;
        options.unpairedUtf16Surrogates = minjson::ParsingOptions::Option::Report;
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == unescapedString);
        CHECK(parsedSize == jsonString.size());
        REQUIRE(issues.size() == 1);
        CHECK(issues[0].code == parsingIssueCode);
        CHECK(issues[0].offset == 20);
      }
    }
  }

  SECTION("consecutive unpaired high surrogates") {
    const auto string = R"(unpaired surrogate \ud83d\ud83d followed by an unpaired surrogate)"sv;
    const auto unescapedString = "unpaired surrogate \xED\xA0\xBD\xED\xA0\xBD followed by an unpaired surrogate"sv;
    // replacing with replacement character �
    const auto unescapedStringWithReplacedSurrogates =
      "unpaired surrogate \xEF\xBF\xBD\xEF\xBF\xBD followed by an unpaired surrogate"sv;

    SECTION("unescape") {
      CAPTURE(string, unescapedString, unescapedStringWithReplacedSurrogates);

      CHECK(minjson::unescape(string) == unescapedString);
      CHECK(minjson::unescape(string, {}, U'\xFFFD') == unescapedStringWithReplacedSurrogates);
    }

    SECTION("parse") {
      const auto jsonString = getQuoted(string);
      CAPTURE(jsonString, unescapedString, unescapedStringWithReplacedSurrogates);

      SECTION("default") {
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == unescapedString);
        CHECK(parsedSize == jsonString.size());
        CHECK(issues.empty());
      }

      SECTION("replace unpaired surrogates") {
        minjson::ParsingOptions options;
        options.replaceInvalidUtf16Surrogates = true;
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == unescapedStringWithReplacedSurrogates);
        CHECK(parsedSize == jsonString.size());
        REQUIRE(issues.empty());
      }

      SECTION("report unpaired surrogates") {
        minjson::ParsingOptions options;
        options.unpairedUtf16Surrogates = minjson::ParsingOptions::Option::Report;
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == unescapedString);
        CHECK(parsedSize == jsonString.size());
        REQUIRE(issues.size() == 2);
        CHECK(issues[0].code == minjson::ParsingIssue::Code::StringContainsUnpairedUtf16HighSurrogate);
        CHECK(issues[0].offset == 20);
        CHECK(issues[1].code == minjson::ParsingIssue::Code::StringContainsUnpairedUtf16HighSurrogate);
        CHECK(issues[1].offset == 26);
      }
    }
  }

  SECTION("fail parsing on unpaired surrogates") {
    minjson::ParsingOptions options;
    options.unpairedUtf16Surrogates = minjson::ParsingOptions::Option::Fail;

    SECTION("unpaired high surrogate at the end of the string") {
      const auto jsonString = R"("unpaired surrogate \ud83d")"sv;
      CAPTURE(jsonString);

      const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == jsonString.size());
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::StringContainsUnpairedUtf16HighSurrogate);
      CHECK(issues[0].offset == 20);
    }

    SECTION("unpaired low surrogate at the end of the string") {
      const auto jsonString = R"("unpaired surrogate \ude00")"sv;
      CAPTURE(jsonString);

      const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == jsonString.size() - 1);  // closing '"' is not parsed due to fail on low surrogate before it
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::StringContainsUnpairedUtf16LowSurrogate);
      CHECK(issues[0].offset == 20);
    }

    SECTION("unpaired high surrogate followed by non-surrogate") {
      const auto jsonString = R"("unpaired surrogate \ud83d\u20ac followed by euro sign")"sv;
      CAPTURE(jsonString);

      const auto [value, status, parsedSize, issues] = minjson::parse(jsonString, options);
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == 32);
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::StringContainsUnpairedUtf16HighSurrogate);
      CHECK(issues[0].offset == 20);
    }
  }
}


TEST_CASE("unescape/parse string with invalid escapes", "[unescape][parse][invalid escapes]") {
  SECTION("invalid escapes") {

    const auto string = R"(invalid escapes: \a \b \0 \1 \. \, \: \ (space))"sv;
    CAPTURE(string);

    SECTION("unescape") {
      REQUIRE(minjson::unescape(string).empty());
      CHECK(minjson::unescape(string, minjson::UnescapeMode::Relaxed).empty());
      CHECK(minjson::unescape(string, minjson::UnescapeMode::Strict).empty());
    }

    SECTION("parse") {
      const auto [value, status, parsedSize, issues] = minjson::parse(getQuoted(string));
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == 19);
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
      CHECK(issues[0].offset == 19);
    }
  }

  auto dummy = [](std::string_view) {};  // dummy sink
  constexpr auto DoNotReplaceSurrogates = minjson::impl::DoNotReplaceSurrogates;

  SECTION("invalid characters as escape spec") {
    // clang-format off
    const auto c = GENERATE(
      range('\0', static_cast<char>('\x1F' + 1)),  // chars \x0..\x1F
      // chars \x20..\x2F except "/
      ' ', '!', '#', '$', '%', '&', '(', ')', '*', '+', ',', '-', '.', // \x20..\x2F
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', // \x30..\x3F
      '@', // \x40
      range('A', static_cast<char>('Z' + 1)),  // \x41..\x5A
      // chars \x5B..\x5F except \ (reverse solidus)
      '[', ']', '^', '_', // \x5B..\x5F
      '`', // \x60
      // chars 'a'..'z' except bfnrtu
      'a', 'c', 'd', 'e', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'o', 'p', 'q', 's', 'v', 'w', 'x', 'y', 'z', // \x61..\x7A
      '{', '|', '}', '~', '\x7f', // \x7B..\x7F
      range('\x80', '\xFF'), '\xFF' // \x80..\xFF
      );
    // clang-format on

    INFO(PrintCharHex{ c } << R"( after \)");
    const auto string = std::string{ R"(invalid escape \)" } + c + std::string{ " spec character" };
    CAPTURE(string);

    SECTION("unescape") {
      CHECK(minjson::impl::unescape(dummy, string, {}, DoNotReplaceSurrogates) == 16);
    }

    SECTION("parse") {
      const auto [value, status, parsedSize, issues] = minjson::parse(getQuoted(string));
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == 17);
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
      CHECK(issues[0].offset == 17);
    }
  }

  SECTION("invalid chararacter after unicode escape spec start \\u") {
    // clang-format off
      const auto c = GENERATE(
        range('\0', static_cast<char>('\x2F' + 1)),  // chars \x0..\x2F
        // chars \x30..\x3F except 0..9
        ':', ';', '<', '=', '>', '?', // \x30..\x3F
        '@', // \x40
        range('G', static_cast<char>('Z' + 1)),  // \x47..\x5A
        // chars \x5B..\x5F except \ (reverse solidus)
        '[', ']', '^', '_', // \x5B..\x5F
        '`', // \x60
        // chars 'a'..'z' except bfnrtu
        range('g', static_cast<char>('z' + 1)), // \x67..\x7A
        '{', '|', '}', '~', '\x7f', // \x7B..\x7F
        range('\x80', '\xFF'), '\xFF' // \x80..\xFF
      );
    // clang-format on

    INFO(PrintCharHex{ c } << R"( after \u)");
    const auto string = std::string{ R"(invalid escape \u)" } + c + std::string{ "0000 spec" };
    CAPTURE(string);

    SECTION("unescape") {
      CHECK(minjson::impl::unescape(dummy, string, {}, DoNotReplaceSurrogates) == 17);
    }

    SECTION("parse") {
      const auto [value, status, parsedSize, issues] = minjson::parse(getQuoted(string));
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == 18);
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
      CHECK(issues[0].offset == 18);
    }
  }

  SECTION("invalid unicode escape number spec") {
    const auto string =
      GENERATE(R"(invalid escape   \uaG00)"sv, R"(invalid escape  \u1aG0)"sv, R"(invalid escape \u01aG)"sv);
    CAPTURE(string);

    SECTION("unescape") {
      CHECK(minjson::unescape(string).empty());
      CHECK(minjson::impl::unescape(dummy, string, {}, DoNotReplaceSurrogates) == 20);
    }

    SECTION("parse") {
      const auto [value, status, parsedSize, issues] = minjson::parse(getQuoted(string));
      INFO(PrintIssues{ issues });
      CHECK(status == minjson::ParsingResultStatus::Failure);
      CHECK(parsedSize == 21);
      REQUIRE(issues.size() == 1);
      CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
      CHECK(issues[0].offset == 21);
    }
  }


  SECTION("incomplete escape at the end of the string") {
    const auto validString = R"(escape at the end of the string \u0000)"sv;

    SECTION("sanity check") {
      const auto escapedString = "escape at the end of the string \0"sv;

      SECTION("unescape") {
        CAPTURE(validString);
        REQUIRE(minjson::unescape(validString) == escapedString);
      }

      SECTION("parse") {
        const auto jsonString = getQuoted(validString);
        CAPTURE(jsonString);
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Success);
        CHECK(value.isString());
        CHECK(value.asString() == escapedString);
        CHECK(parsedSize == jsonString.size());
        REQUIRE(issues.empty());
      }
    }

    SECTION("incomplete escape") {
      SECTION("unescape") {
        // cutting 5 characters makes an invalid JSON string like this: "...\"
        // so we only test unescape()
        const auto string = validString.substr(0, validString.size() - 5);
        CAPTURE(string);
        CHECK(minjson::unescape(string).empty());
        CHECK(minjson::impl::unescape(dummy, string, {}, DoNotReplaceSurrogates) == minjson::impl::NPos);
      }

      const size_t cut = GENERATE(1, 2, 3, 4);
      const auto string = validString.substr(0, validString.size() - cut);

      SECTION("unescape") {
        CAPTURE(string);
        CHECK(minjson::unescape(string).empty());
        CHECK(minjson::impl::unescape(dummy, string, {}, DoNotReplaceSurrogates) == minjson::impl::NPos);
      }

      SECTION("parse") {
        const auto jsonString = getQuoted(string);
        CAPTURE(jsonString);
        const auto [value, status, parsedSize, issues] = minjson::parse(jsonString);
        INFO(PrintIssues{ issues });
        CHECK(status == minjson::ParsingResultStatus::Failure);
        CHECK(parsedSize == string.size() + 1);
        REQUIRE(issues.size() == 1);
        CHECK(issues[0].code == minjson::ParsingIssue::Code::InvalidCharacter);
        CHECK(issues[0].offset == string.size() + 1);
      }
    }
  }
}
