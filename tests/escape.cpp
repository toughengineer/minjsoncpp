#include <minjsoncpp.h>

#include "utils.h"

#include <catch2/catch_all.hpp>

using namespace std::string_view_literals;

TEST_CASE("escape valid UTF-8 strings", "[escape][serialize]") {
  SECTION("empty string is unchanged") {
    SECTION("escape") {
      CHECK(minjson::escape(""sv) == ""sv);
      CHECK(minjson::escape(""sv, minjson::Escape::Default) == ""sv);
      CHECK(minjson::escape(""sv, minjson::Escape::NonAscii) == ""sv);
    }

    SECTION("serialize") {
      const minjson::Value v = ""sv;
      CHECK(minjson::serializeToString(v) == getQuoted(""sv));
      CHECK(minjson::serializeToString(v, { /*indent =*/0, minjson::Escape::Default }) == "\"\""sv);
      CHECK(minjson::serializeToString(v, { /*indent =*/0, minjson::Escape::NonAscii }) == "\"\""sv);
    }
  }

  using pair = std::pair<std::string_view, std::string_view>;

  SECTION("control and common escape characters") {
    // clang-format off
    const auto [string, escapedString] = GENERATE(
      pair{ "null \0 character"sv, "null \\u0000 character"sv },
      pair{ "tab \t character"sv, "tab \\t character"sv },
      pair{ "carriage return \r character"sv, "carriage return \\r character"sv },
      pair{ "line feed \n character"sv, "line feed \\n character"sv },
      pair{ "whatever this \xf character is"sv, "whatever this \\u000f character is"sv },
      pair{ "quotation mark \" character"sv, "quotation mark \\\" character"sv },
      pair{ "reverse solidus \\ character"sv, "reverse solidus \\\\ character"sv });
    // clang-format on
    CAPTURE(string, escapedString);

    SECTION("escape") {
      CHECK(minjson::escape(string) == escapedString);
    }

    SECTION("serialize") {
      CHECK(minjson::serializeToString(minjson::Value{ string }) == getQuoted(escapedString));
    }
  }

  SECTION("control and common escape characters at different ends of a string") {
    // clang-format off
    const auto [string, escapedString] = GENERATE(
      pair{ "null \0"sv, "null \\u0000"sv },
      pair{ "\0 character"sv, "\\u0000 character"sv },
      pair{ "\0"sv, "\\u0000"sv },

      pair{ "tab \t"sv, "tab \\t"sv },
      pair{ "\t character"sv, "\\t character"sv },
      pair{ "\t"sv, "\\t"sv },

      pair{ "line feed \n"sv, "line feed \\n"sv },
      pair{ "\n character"sv, "\\n character"sv },
      pair{ "\n"sv, "\\n"sv },

      pair{ "quotation mark \""sv, "quotation mark \\\""sv },
      pair{ "\" character"sv, "\\\" character"sv },
      pair{ "\""sv, "\\\""sv },

      pair{ "reverse solidus \\"sv, "reverse solidus \\\\"sv },
      pair{ "\\ character is"sv, "\\\\ character is"sv },
      pair{ "\\"sv, "\\\\"sv });
    // clang-format on
    CAPTURE(string, escapedString);

    SECTION("escape") {
      CHECK(minjson::escape(string) == escapedString);
    }

    SECTION("serialize") {
      CHECK(minjson::serializeToString(minjson::Value{ string }) == getQuoted(escapedString));
    }
  }

  const auto utf8Validation =
    GENERATE(minjson::Utf8Validation::IgnoreInvalidUtf8CodeUnits, minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits);
  INFO("UTF-8 validation: " << (utf8Validation == minjson::Utf8Validation::IgnoreInvalidUtf8CodeUnits ? "on" : "off"));

  SECTION("no characters to escape") {
    minjson::SerializationOptions options;
    options.validation = utf8Validation;

    SECTION("escape") {
      CHECK(minjson::escape("no escape"sv, {}, utf8Validation) == "no escape"sv);
    }

    SECTION("serialize") {
      CHECK(minjson::serializeToString(minjson::Value{ "no escape"sv }, options) == "\"no escape\""sv);
    }

    // "utf8 2 byte code point £ 3 byte code point € 4 byte code point 😀 characters"
    const auto string =
      "utf8 2 byte code point \xC2\xA3 3 byte code point \xE2\x82\xAC 4 byte code point \xF0\x9F\x98\x80 characters"sv;
    CAPTURE(string);

    SECTION("escape") {
      CHECK(minjson::escape(string, {}, utf8Validation) == string);
    }

    SECTION("serialize") {
      CHECK(minjson::serializeToString(minjson::Value{ string }, options) == getQuoted(string));
    }
  }

  SECTION("escape non-ASCII characters") {
    const auto [string, escapedString] = GENERATE(
      // "pound sign £ character"
      pair{ "pound sign \xC2\xA3 character"sv, "pound sign \\u00a3 character"sv },
      // "euro sign € character"
      pair{ "euro sign \xE2\x82\xAC character"sv, "euro sign \\u20ac character"sv },
      // "whatever this emoji 😀 character is"
      pair{ "whatever this emoji \xF0\x9F\x98\x80 character is"sv,
            "whatever this emoji \\ud83d\\ude00 character is"sv });
    CAPTURE(string, escapedString);

    SECTION("escape") {
      CHECK(minjson::escape(string, minjson::Escape::NonAscii, utf8Validation) == escapedString);
    }

    SECTION("serialize") {
      minjson::SerializationOptions options;
      options.escape = minjson::Escape::NonAscii;
      options.validation = utf8Validation;
      CHECK(minjson::serializeToString(minjson::Value{ string }, options) == getQuoted(escapedString));
    }
  }

  SECTION("uppercase hex digits") {
    minjson::SerializationOptions options;
    options.hexDigitsCase = minjson::HexDigitsCase::Upper;
    options.validation = utf8Validation;

    SECTION("default") {
      const auto string = "whatever this \xf character is"sv;
      const auto escapedString = "whatever this \\u000F character is"sv;
      CAPTURE(string, escapedString);

      SECTION("escape") {
        CHECK(minjson::escape(string, {}, utf8Validation, minjson::HexDigitsCase::Upper) == escapedString);
      }

      SECTION("serialize") {
        CHECK(minjson::serializeToString(minjson::Value{ string }, options) == getQuoted(escapedString));
      }
    }

    SECTION("escape non-ASCII") {
      const auto [string, escapedString] = GENERATE(
        // "pound sign £ character"
        pair{ "pound sign \xC2\xA3 character"sv, "pound sign \\u00A3 character"sv },
        // "euro sign € character"
        pair{ "euro sign \xE2\x82\xAC character"sv, "euro sign \\u20AC character"sv },
        // "whatever this emoji 😀 character is"
        pair{ "whatever this emoji \xF0\x9F\x98\x80 character is"sv,
              "whatever this emoji \\uD83D\\uDE00 character is"sv });
      CAPTURE(string, escapedString);

      SECTION("escape") {
        CHECK(minjson::escape(string, minjson::Escape::NonAscii, utf8Validation, minjson::HexDigitsCase::Upper) ==
              escapedString);
      }

      SECTION("serialize") {
        options.escape = minjson::Escape::NonAscii;
        CHECK(minjson::serializeToString(minjson::Value{ string }, options) == getQuoted(escapedString));
      }
    }
  }

  SECTION("UTF-8 code points at ends of a string") {
    const auto string =
      GENERATE("pound sign \xC2\xA3"sv, "\xC2\xA3 character"sv, "\xC2\xA3"sv, "euro sign \xE2\x82\xAC"sv,
               "\xE2\x82\xAC character"sv, "\xE2\x82\xAC"sv, "whatever this emoji \xF0\x9F\x98\x80"sv,
               "\xF0\x9F\x98\x80 character is"sv, "\xF0\x9F\x98\x80"sv);
    CAPTURE(string);

    SECTION("escape") {
      CHECK(minjson::escape(string, {}, utf8Validation) == string);
    }

    SECTION("serialize") {
      minjson::SerializationOptions options;
      options.validation = utf8Validation;
      CHECK(minjson::serializeToString(minjson::Value{ string }, options) == getQuoted(string));
    }
  }
}


TEST_CASE("escape invalid UTF-8 strings", "[escape][serialize][invalid utf8]") {
  using pair = std::pair<std::string_view, std::string_view>;

  const auto IgnoreInvalidUtf8CodeUnits = minjson::Utf8Validation::IgnoreInvalidUtf8CodeUnits;
  const auto FailOnInvalidUtf8CodeUnits = minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits;

  auto dummy = [](std::string_view) {};  // dummy sink

  SECTION("invalid code point length") {
    const auto [string, invalidCodeUnits] = GENERATE(
      pair{ "pound sign          \xC2 character (missing continuation byte for 2 byte code point)"sv, "\xC2 "sv },
      pair{ "euro sign           \xE2\x82 character (missing last continuation byte for 3 byte code point)"sv,
            "\xE2\x82 "sv },
      pair{ "whatever this emoji \xF0\x9F\x98 character is (missing last continuation byte for 4 byte code point)"sv,
            "\xF0\x9F\x98 "sv });
    CAPTURE(string);

    SECTION("escape") {
      CHECK(minjson::escape(string, {}, IgnoreInvalidUtf8CodeUnits) == string);
      CHECK(minjson::escape(string, {}, FailOnInvalidUtf8CodeUnits).empty());
      CHECK(minjson::impl::escape(dummy, string, {}, FailOnInvalidUtf8CodeUnits, {}) == 20);
    }

    SECTION("serialize") {
      CHECK(minjson::serializeToString(minjson::Value{ string }) == getQuoted(string));

      minjson::SerializationOptions options;
      options.validation = FailOnInvalidUtf8CodeUnits;
      try {
        const auto s = minjson::serializeToString(minjson::Value{ string }, options);
        (void)s;
        FAIL();
      } catch (const minjson::InvalidUtf8CodeUnitsError &e) {
        CHECK(e.offset == 20);
        CHECK(NonPrintStr{ e.codeUnits } == NonPrintStr{ invalidCodeUnits });
      } catch (...) {
        FAIL();
      }
    }
  }

  SECTION("extra continuation byte") {
    const auto [string, invalidCodeUnits] =
      GENERATE(pair{ "pound sign            \xC2\xA3\xA3 character (2 byte code point)"sv, "\xA3"sv },
               pair{ "euro sign            \xE2\x82\xAC\xAC character (3 byte code point)"sv, "\xAC"sv },
               pair{ "whatever this emoji \xF0\x9F\x98\x80\x80 character is (4 byte code point)"sv, "\x80"sv });
    CAPTURE(string);

    SECTION("escape") {
      CHECK(minjson::escape(string, {}, IgnoreInvalidUtf8CodeUnits) == string);
      CHECK(minjson::escape(string, {}, FailOnInvalidUtf8CodeUnits).empty());
      CHECK(minjson::impl::escape(dummy, string, {}, FailOnInvalidUtf8CodeUnits, {}) == 24);
    }

    SECTION("serialize") {
      CHECK(minjson::serializeToString(minjson::Value{ string }) == getQuoted(string));

      minjson::SerializationOptions options;
      options.validation = FailOnInvalidUtf8CodeUnits;
      try {
        const auto s = minjson::serializeToString(minjson::Value{ string }, options);
        (void)s;
        FAIL();
      } catch (const minjson::InvalidUtf8CodeUnitsError &e) {
        CHECK(e.offset == 24);
        CHECK(NonPrintStr{ e.codeUnits } == NonPrintStr{ invalidCodeUnits });
      } catch (...) {
        FAIL();
      }
    }
  }

  SECTION("continuation bytes without starting byte") {
    const auto [string, invalidCodeUnits] =
      GENERATE(pair{ "pound sign          \xA3 character"sv, "\xA3"sv },
               pair{ "euro sign           \x82\xAC character"sv, "\x82"sv },
               pair{ "whatever this emoji \x9F\x98\x80 character is"sv, "\x9F"sv });
    CAPTURE(string);

    SECTION("escape") {
      CHECK(minjson::escape(string, {}, IgnoreInvalidUtf8CodeUnits) == string);
      CHECK(minjson::escape(string, {}, FailOnInvalidUtf8CodeUnits).empty());
      CHECK(minjson::impl::escape(dummy, string, {}, FailOnInvalidUtf8CodeUnits, {}) == 20);
    }

    SECTION("serialize") {
      CHECK(minjson::serializeToString(minjson::Value{ string }) == getQuoted(string));

      minjson::SerializationOptions options;
      options.validation = FailOnInvalidUtf8CodeUnits;
      try {
        const auto s = minjson::serializeToString(minjson::Value{ string }, options);
        (void)s;
        FAIL();
      } catch (const minjson::InvalidUtf8CodeUnitsError &e) {
        CHECK(e.offset == 20);
        CHECK(NonPrintStr{ e.codeUnits } == NonPrintStr{ invalidCodeUnits });
      } catch (...) {
        FAIL();
      }
    }
  }
}
