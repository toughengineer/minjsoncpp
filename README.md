# Minimalistic JSON C++ library

**minjsoncpp** is a minimalistic C++17 header-only library that implements facilities
to programmatically manipulate, serialize/stringify and parse JSON data as defined by
[RFC 8259 The JavaScript Object Notation (JSON) Data Interchange Format](https://datatracker.ietf.org/doc/html/rfc8259).

The goals of this project (from the highest priority to the lowest):
1. strictly conform to JSON specification,
2. minimal footprint (source size, binary size, etc.),
3. reasonable performance.

As such the code is only 1500 lines, no dependencies except for the C++ standard library,
yet it follows the spec very closely and has reasonably good performance
compared to popular JSON C++ libraries.

It is primarily intended for situations when you want to read a small JSON file, a config file for example,
or to write a small JSON file, but don't want to deal with huge and/or complicated dependencies
because of that small part where you need such functionality.


## Supported compilers and standard libraries

Compilers:
* **GCC**: 11.1 and later\*
* **Clang**: 8 (with libstdc++ 11.1..12.3) and later (with respectively supported versions of standard library)
* **MSVC**: 19.16 (comes with Visual Studio 2017 version 15.9.11) and later

Standard libraries:
* **libstdc++**: 11.1 and later\*\
(goes with GCC, can be used with Clang)
* **libc++**: 14 and later\*\*\
(goes with Clang)
* **MSVC**: the one that comes with Visual Studio 2017 version 15.9.11 and later\
(can be used with Clang on Windows)

\*Since the earliest supported version of libstdc++ is 11.1 that's the corresponding earliest supported version of GCC
because they (typically) come together.\
\*\*libc++ until version 20.1 does not implement `std::from_chars()` for `double`; `strtod()` is used instead when building with libc++.

In general you should be able to use it with any fully C++17 conformant compiler and standard library.


## How to acquire the library

The easiest way is to put the header file [minjsoncpp.h](include/minjsoncpp.h)
or copy-paste its contents into your project.

### Cloning repo/using submodule

You can clone the repo `https://github.com/toughengineer/minjsoncpp.git`, or add it as a submodule.
<details><summary>Click/tap to expand details.</summary>

```
git submodule add https://github.com/toughengineer/minjsoncpp.git
```
or
```
git submodule add https://github.com/toughengineer/minjsoncpp.git thirdparty/minjsoncpp
```
to specify destination directory, in this case `thirdparty/minjsoncpp`,\
and then initialize it with
```
git submodule update --init
```

In your CMakeLists.txt you can just add include directory with something like this:
```
include_directories("thirdparty/minjsoncpp/include")
```
Or you can add include directory for a particular CMake target (`${TARGET_NAME}` in this case):
```
target_include_directories(${TARGET_NAME} PRIVATE "thirdparty/minjsoncpp/include")
```
In this case you don't need to explicitly add dependencies to the targets.

Instead of explicitly adding include directory you can add a dependency to your CMake target:
```
target_link_libraries(${TARGET_NAME} minjsoncpp)

add_subdirectory("thirdparty/minjsoncpp")
```
`target_link_libraries` adds dependency to a target and makes include file available.

`add_subdirectory` processes **minjsoncpp**'s CMakeLists.txt and adds `minjsoncpp` target for consumption.
</details>

### Using CMake _FetchContent_

You can use [_FetchContent_](https://cmake.org/cmake/help/latest/module/FetchContent.html)
to get the library via CMakeLists.txt:
```
include(FetchContent)

FetchContent_Declare(
  minjsoncpp
  GIT_REPOSITORY https://github.com/toughengineer/minjsoncpp.git
  GIT_TAG main
)

FetchContent_MakeAvailable(minjsoncpp)
```

For your CMake target add:
```
target_link_libraries(${TARGET_NAME} minjsoncpp)
```
where `${TARGET_NAME}` is the name of a CMake target like an executable or a library.


## API

It is relatively straightforward.

You can jump straight to [examples](examples/)
if you want to see how it can be used.

This library assumes UTF-8 encoding of strings inside JSON values, of inputs that are parsed, and
of serialized/stringified JSON representation
prescribed by [RFC 8259 section 8.1](https://datatracker.ietf.org/doc/html/rfc8259#section-8.1).

Entities in the namespace `minjson` are all public and can be used freely.

Entities in the namespace `minjson::impl` are also public, but should be seen as "advanced" API
intended to expose the full functionality without conveniently defined defaults.

Entities in the namespace `minjson::detail` are implementation details, you should not use them explicitly.

### JSON values

JSON value is represented as template `minjson::BasicValue<Allocator>`
with allocator parameter.

`minjson::Value` is defined for covenience as an alias to
`minjson::BasicValue<std::allocator<char>>`.
See section about allocator support [below](#allocators) for details.

By convention `minjson::BasicValue` is parameterized with an allocator type for `char` elements.

Assuming `minjson::Value`, mapping of JSON value types to C++ types is as follows:

| JSON  | C++ |
|-------|-----|
|null   |`minjson::Null` (alias for `std::monostate`)|
|boolean|`minjson::Boolean` (alias for `bool`)|
|number |`int64_t` for integers,<br>`double` for decimal numbers|
|string |`minjson::String` (alias for `std::string`, assumed to contain UTF-8 text)|
|array  |`minjson::Array` (alias for `std::vector<minjson::Value>`)|
|object |`minjson::Object` (alias for `std::unordered_map<minjson::String, minjson::Value>`)|

Concrete value is stored in a variant defined as:
```c++
using Variant = std::variant<Null, Boolean, int64_t, double, String, Array, Object>;
```

`minjson::Value` provides access to it via `variant()` method and
can be implicitly converted to a reference to `minjson::Variant`.

#### `visit()` functionality

You can `std::visit()` the underlying variant as usual, e.g.:
```c++
minjson::Value value = foo();
std::visit([](auto &x) { bar(x); }, value.variant());
```
or use `minjson::visit()` function directly on a `minjson::Value` instance:
```c++
minjson::Value value = foo();
minjson::visit([](auto &x) { bar(x); }, value);
```
or you can omit the namespace and rely on [ADL](https://en.cppreference.com/w/cpp/language/adl):
```c++
minjson::Value value = foo();
visit([](auto &x) { bar(x); }, value);
```

See [example](examples/#visit) in [visit.cpp](examples/visit.cpp).

#### Value type inspection

You can explicitly check the type of a value and access the underlying object by using the following methods:

| check type | access value |
|------------|--------------|
|`isNull()`  |
|`isBool()`  |`asBool()`|
|`isInt()`   |`asInt()`|
|`isDouble()`|`asDouble()`|
|`isString()`|`asString()`|
|`isArray()` |`asArray()`|
|`isObject()`|`asObject()`|

#### Initialization

You can construct `minjson::Value` in the usual way, e.g.:
```c++
minjson::Value null = minjson::Null{};
minjson::Value boolean = true;
minjson::Value integer = 42;
minjson::Value decimal = 3.14;
minjson::Value string = "hello";
minjson::Value array = minjson::Array{ 1, 2, 3 };
minjson::Value object = minjson::Object{ { "foo", 42 }, { "bar", "baz" } };
```

You can combine it any way you want, e.g.:
```c++
const minjson::Value value = minjson::Object{
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
```

#### Accessing values throughout JSON document

To access values within the JSON document throughout multiple levels you can use `resolve()` method
which returns a pointer to a value (`const minjson::Value*`, can be `nullptr` if resolution fails).

Below are some examples of usage.
```c++
minjson::Value o = minjson::Object{ { "foo", 42 }, { "bar", "baz" } };

const auto *foo = o.resolve("foo");
if (foo) {
  // use '*foo'
}
```
In this case, string key argument `"foo"` implies that we expect `o` to contain an object and
we are trying to get member value designated by the corresponding key.

```c++
minjson::Value a = minjson::Array{ 1, 2, 3 };

const auto *item = a.resolve(0);
if (item) {
  // use '*item'
}
```
Here numeric index argument `0` implies that we expect `a` to contain an array and
we are trying to get member value with the corresponding index.

You can provide multiple arguments to resolve values throughout multiple levels,
which would be similar to arguments being tokens of a JSON Pointer string, e.g.
```c++
const minjson::Value value = minjson::Object{
  { "object", minjson::Object{
      { "nested array", minjson::Array{ 4, 5, 6 } }
    }
  }
};

const auto *item = value.resolve("object", "nested array", 0);
if (item) {
  // use '*item'
}
```
In this case
* string key argument `"object"` implies that we expect `value` to contain an object and
we are trying to get member value designated by that key,
* in turn, string key argument `"nested array"` implies that we expect the value that we get
from the previous step to be an object and we are trying to get member value designated by this key,
* finally, numeric index argument `0` implies that we expect the value that we get from the previous step
(the `"nested array"` one) to contain an array and we are trying to get member value with that index.

This would be similar to trying to resolve JSON Pointer `/object/nested array/1`.

See [more examples](examples/#resolve) in [resolve.cpp](examples/resolve.cpp).

### Sinks

Function templates in namespace `minjson::impl` receive "sinks": objects with implemented `operator()`
or functions that can receive instances of `std::string_view` as subsequent parts of the result.

E.g. it can be a lambda:
```c++
const auto stdOutSink = [](std::string_view s) { std::cout << s; };
```
In this case it outputs everything into `stdout`.

Or
```c++
std::string string;
const auto stringSink = [&string](std::string_view s) { string += s; };
```
In this case the result is subsequently appended to `string`.

<a id="escape"></a>
### Escaping strings

```c++
namespace minjson {
  enum class Escape {
    Default,
    NonAscii
  };
}
```
* `Default`: escape only mandated characters `\x0..\x1F`, `"`&nbsp;(`\x22`) and `\`&nbsp;(`\x5C`).
* `NonAscii`: escape mandated caracters as well as non-ASCII characters (code points `\x80..\x10FFFF`).\
Note that escaped characters will always occupy larger space (more bytes) compared to characters as is
(when they are not escaped).

-----

```c++
namespace minjson {
  enum class Utf8Validation {
    IgnoreInvalidUtf8CodeUnits,
    FailOnInvalidUtf8CodeUnits
  };
}
```
* `IgnoreInvalidUtf8CodeUnits`: turns UTF-8 validation off.
* `FailOnInvalidUtf8CodeUnits`: operation fails when an invalid UTF-8 code point/code unit is encountered.

-----

```c++
namespace minjson {
  enum class HexDigitsCase {
    Lower,
    Upper
  };
}
```
**`minjson::HexDigitsCase`** specifies hexadecimal digits case in escapes (e.g. `"\u00ae"`).

-----

```c++
namespace minjson {
  template<typename String_t = String>
  [[nodiscard]] String_t escape(std::string_view s,
                                Escape escapeMode = {},
                                Utf8Validation validation = {},
                                HexDigitsCase hexDigitsCase = HexDigitsCase::Lower);
}
```
**`minjson::escape()`** returns escaped input string.
String type must support `operator+=` with `std::string_view` operand and `reserve()` method receiving `size_t`.

When `minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits` is specified, returns empty string on failure.\
Otherwise fails only if underlying string appending fails, e.g. as a result of failed allocation.

See [example](examples/#escape) in [escape.cpp](examples/escape.cpp).

-----

```c++
namespace minjson {
  namespace impl {
    template<typename Sink>
    size_t escape(Sink &&sink,
                  std::string_view s,
                  Escape escapeMode,
                  Utf8Validation validation,
                  HexDigitsCase hexDigitsCase);
  }
}
```
**`minjson::impl::escape()`** passes subsequent parts of escaped string to the _sink_.

Returns
* the size in bytes/code units of the input string that was successfully escaped, or
* the offset (in bytes/code units) of invalid code point on failure when `minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits` is specified.

This function does not throw exceptions on its own.\
Exceptions thrown by the _sink_ operations are let through, i.e. are not handled.

See [example](examples/#escape_impl) in [escape_impl.cpp](examples/escape_impl.cpp).

-----

### Serialization

```c++
namespace minjson {
  struct SerializationOptions {
    size_t indent = 0;
    Escape escape = {};
    Utf8Validation validation = {};
    HexDigitsCase hexDigitsCase = HexDigitsCase::Lower;
    enum class IndentationChar {
      Space,
      Tab
    } indentationChar = {};
    struct NewlineOptions {
      std::string_view separator = {};
      bool afterObjectOpeningBrace = true;
      bool beforeObjectClosingBrace = true;
      bool afterObjectMemberKey = false;
      bool beforeObjectMemberValue = false;
      bool beforeObjectMemberCollectionValue = false;
      bool beforeObjectMemberSeparator = false;
      bool afterObjectMemberSeparator = true;
      bool afterArrayOpeningBracket = true;
      bool beforeArrayClosingBracket = true;
      bool beforeArrayMemberSeparator = false;
      bool afterArrayMemberSeparator = true;
    } newline{};
    bool sortObjectKeys = false;
    std::string_view nullLiteral = NullLiteral;
    std::string_view falseLiteral = FalseLiteral;
    std::string_view trueLiteral = TrueLiteral;
    std::string_view emptyObject = "{}";
    std::string_view objectOpeningBrace = "{";
    std::string_view objectClosingBrace = "}";
    std::string_view objectKeyValueSeparator = ":";
    std::string_view objectMemberSeparator = ",";
    std::string_view emptyArray = "[]";
    std::string_view arrayOpeningBracket = "[";
    std::string_view arrayClosingBracket = "]";
    std::string_view arrayMemberSeparator = ",";
    std::string_view openingStringQuotation = "\"";
    std::string_view closingStringQuotation = "\"";
  };
}
```
**`minjson::SerializationOptions`** defines options to control serialization/stringification.

`indent` specifies the amount of spaces or tabs to use to indent nested values
(everything _inside_ an object `{}` or an array `[]` is indented).

`escape`, `validation` and `hexDigitsCase` specify the same things as in the case of
escape functions [above](#escape).

`indentationChar` tells to use spaces for indentation when `IndentationChar::Space` is set,
and tabs when `IndentationChar::Tab` is set.

`newline.separator` specifies line separator like `\n` or `\r\n`.\
If empty and `indent` is set to non-zero, defaults to `\n`.

<details><summary>
The rest of the <code>newline</code> options specify after or before which element
to insert new line and indentation. (Click/tap to show.)
</summary>

`newline.afterObjectOpeningBrace` designates new line after `{`, e.g.
```json
{
  "foo":"bar"}
```
`newline.beforeObjectClosingBrace` designates new line before `}`, e.g.
```json
{"foo":"bar"
}
```
If either `newline.afterObjectOpeningBrace` or `newline.beforeObjectClosingBrace` is set and
`emptyObject` is set to an empty string, empty objects are serialized like this:
```json
{
}
```

`newline.afterObjectMemberKey` designates new line after key before `:`, e.g.
```json
{"foo"
  :"bar"}
```
`newline.beforeObjectMemberValue` designates new line after `:` before value, e.g.
```json
{"foo":
  "bar"}
```
`newline.beforeObjectMemberCollectionValue` designates new line after `:` before an array or an object, e.g.
```json
{"foo":"bar",
  "baz":
  ["qux"]}
```
`newline.beforeObjectMemberSeparator` designates new line before `,` inside objects, e.g.
```json
{"foo":"bar"
  ,"baz":"qux"}
```
`newline.afterObjectMemberSeparator` designates new line after `,` inside objects, e.g.
```json
{"foo":"bar",
  "baz":"qux"}
```
`newline.afterArrayOpeningBracket` designates new line after `[`, e.g.
```json
[
  "foo","bar"]
```
`newline.beforeArrayClosingBracket` designates new line before `]`, e.g.
```json
["foo","bar"
]
```
If either `newline.afterArrayOpeningBracket` or `newline.beforeArrayClosingBracket` is set and
`emptyArray` is set to an empty string, empty arrays are serialized like this:
```json
[
]
```

`newline.beforeArrayMemberSeparator` designates new line before `,` inside arrays, e.g.
```json
["foo"
  ,"bar"]
```
`newline.afterArrayMemberSeparator` designates new line after `,` inside arrays, e.g.
```json
["foo",
  "bar"]
```
</details>

`sortObjectKeys` tells to serialize object members in ascending order of keys when set to `true`.

<details><summary>
The following options allow to customize elements of JSON when serializing.
(Click/tap to show.)
</summary>

| options      | description |
|--------------|-------------|
|`nullLiteral` |literal `null`|
|`falseLiteral`|literal `false`|
|`trueLiteral` |literal `true`|
|`emptyObject` |used for empty objects if set to a non-empty string, e.g. `{}`|
|`objectOpeningBrace`|`{`|
|`objectClosingBrace`|`}`|
|`objectKeyValueSeparator`|`:`|
|`objectMemberSeparator`|`,` inside objects|
|`emptyArray`  |used for empty arrays if set to a non-empty string, e.g. `[]`|
|`arrayOpeningBracket`|`[`|
|`arrayClosingBracket`|`]`|
|`arrayMemberSeparator`|`,` inside arrays|
|`openingStringQuotation`|string opening `"`|
|`closingStringQuotation`|string closing `"`|
</details>

-----

```c++
namespace minjson {
  struct InvalidUtf8CodeUnitsError : std::runtime_error {
    InvalidUtf8CodeUnitsError(const char *msg,
                              std::string_view codeUnits,
                              size_t offset);
    const std::string codeUnits;
    const size_t offset;
  };
}
```
**`minjson::InvalidUtf8CodeUnitsError`** is thrown by `serialize()` functions when UTF-8 validation fails.

`codeUnits` field contains code units at the point of failure from the string that caused it.

`offset` specifies the offset of `codeUnits` in the string that caused failure.

> [!NOTE]
> When `minjson::SerializationOptions::validation` is set to `minjson::Utf8Validation::FailOnInvalidUtf8CodeUnits`
> all the serialization functions `minjson::serializeToStream()`, `minjson::serializeToString()` and
> `minjson::impl::serialize()` throw `minjson::InvalidUtf8CodeUnitsError` on validation failure.

-----

```c++
namespace minjson {
  template<typename Allocator>
  void serializeToStream(std::ostream &s,
                         const BasicValue<Allocator> &v,
                         const SerializationOptions &o = {});
}
```
**`minjson::serializeToStream()`** writes serialized JSON value `v` to standard output stream `s`.

May throw `minjson::InvalidUtf8CodeUnitsError`.\
Otherwise fails only if the underlying stream write operation fails.

-----

```c++
namespace minjson {
  template<typename Allocator>
  [[nodiscard]] typename BasicValue<Allocator>::String serializeToString(const BasicValue<Allocator> &v,
                                                                         const SerializationOptions &o = {});
}
```
**`minjson::serializeToString()`** returns JSON value `v` serialized to a string.

May throw `minjson::InvalidUtf8CodeUnitsError`.\
Otherwise fails only if underlying string appending fails, e.g. as a result of failed allocation.

See [example](examples/#serialize) in [serialize.cpp](examples/serialize.cpp).

-----

```c++
namespace minjson {
  namespace impl {
    template<typename Sink, typename Allocator>
    void serialize(Sink &&sink,
                   const BasicValue<Allocator> &v,
                   const SerializationOptions &options,
                   size_t initialIndentation = 0);
  }
}
```
**`minjson::impl::serialize()`** passes subsequent parts of serialized JSON value `v` to the _sink_.

`initialIndentation` specifies initial indentation of nested collection members.
_Note that the root value itself is not indented._

May throw `minjson::InvalidUtf8CodeUnitsError`.\
Exceptions thrown by the _sink_ operations are let through, i.e. are not handled.

See [example](examples/#serialize_impl) in [serialize_impl.cpp](examples/serialize_impl.cpp).

-----

### Unescaping strings

```c++
namespace minjson {
  enum class UnescapeMode {
    Relaxed,
    Strict
  };
}
```
* `Relaxed` mode allows input to have any characters.
* `Strict` mode allows only valid JSON string characters to be in the input, i.e. code points `\x20..\x10FFFF`;
`"`&nbsp;(`\x22`) _must_ be escaped.

In both modes `\`&nbsp;(`\x5C`) must introduce a valid escape sequence, e.g.: `"\n"`.

-----

```c++
namespace minjson {
  template<typename String_t = String>
  [[nodiscard]] String_t unescape(std::string_view input,
                                  UnescapeMode unescapeMode = {});
}
```
**`minjson::unescape()`** returns string containing unescaped _input_, or empty string on failure.

Fails on invalid escape sequences, and invalid JSON string characters if `minjson::UnescapeMode::Strict` is specified.\
Otherwise fails only if underlying string appending fails, e.g. as a result of failed allocation.

See [example](examples/#unescape) in [unescape.cpp](examples/unescape.cpp).

-----

```c++
namespace minjson {
  template<typename String_t = String>
  [[nodiscard]] String_t unescape(std::string_view input,
                                  UnescapeMode unescapeMode,
                                  char32_t unpairedSurrogateReplacement);
}
```
**`minjson::unescape()`** returns string containing unescaped _input_, or empty string on failure.

`unpairedSurrogateReplacement` specifies replacement character for incorrectly situated UTF-16 surrogates,
e.g. when high surrogate is not followed by low surrogate (hence not meaningfully encoding a character):
`"\ud83d"`.

Fails on invalid escape sequences, and invalid JSON string characters if `minjson::UnescapeMode::Strict` is specified.\
Otherwise fails only if underlying string appending fails, e.g. as a result of failed allocation.

-----

```c++
namespace minjson {
  namespace impl {
    inline constexpr size_t NPos = ~size_t{ 0 };

    inline constexpr char32_t DoNotReplaceSurrogates = ~char32_t{ 0 };

    template<typename Sink>
    size_t unescape(Sink &&sink,
                    std::string_view input,
                    UnescapeMode mode,
                    char32_t surrogateReplacement = DoNotReplaceSurrogates);
  }
}
```
**`minjson::impl::unescape()`** passes subsequent parts of unescaped input to the _sink_.

Returns
* the size in bytes/code units of the _input_ that was successfully unescaped (size of _input_ on success), or
* `minjson::impl::NPos` if end of input was reached unexpectedly, e.g. in the middle of an escape: `"\u0"`.

`surrogateReplacement` specifies replacement character for incorrectly situated UTF-16 surrogates.
If set to `minjson::impl::DoNotReplaceSurrogates` causes unmatched surrogate code points to be encoded as UTF-8.

Fails on invalid escape sequences, and invalid JSON string characters if `minjson::UnescapeMode::Strict` is specified.\
Exceptions thrown by the _sink_ operations are let through, i.e. are not handled.

See [example](examples/#unescape_impl) in [unescape_impl.cpp](examples/unescape_impl.cpp).

-----

### Parsing

```c++
namespace minjson {
  struct ParsingOptions {
    enum class Option {
      Ignore,
      Report,
      Fail
    };
    Option duplicateObjectKeys = Option::Fail;
    Option unpairedUtf16Surrogates = Option::Ignore;
    bool replaceInvalidUtf16Surrogates = false;
    char32_t replacement = U'\xfffd';
  };
}
```
**`minjson::ParsingOptions`** defines options to control parsing behavior.

`duplicateObjectKeys` tells what to do when duplicate keys are encountered in an object during parsing:
* `Option::Ignore`: the resulting value is set to one of the encountered values;
* `Option::Report`: additionally the issue is added to the list of issues;
* `Option::Fail`: parsing fails when duplicate keys are encountered, the issue is added to the list of issues.

`unpairedUtf16Surrogates` tells what to do when an unpaired UTF-16 surrogate is encountered during parsing
(e.g. when high surrogate is not followed by low surrogate hence not meaningfully encoding a character: `"\ud83d"`):
* `Option::Ignore`: no validation is performed;
* `Option::Report`: the issue is added to the list of issues;
* `Option::Fail`: parsing fails when an unpaired surrogate is encounterd, the issue is added to the list of issues.

`replaceInvalidUtf16Surrogates` specifies whether to replace incorrectly situated surrogates.

`replacement` specifies replacement character for incorrectly situated surrogates, by default it's U+FFFD: �.

-----

```c++
namespace minjson {
  struct ParsingIssue {
    enum class Code {
      Other,
      InvalidCharacter,
      UnexpectedEndOfInput,
      FailedToParseNumber,
      ParsedNumberOutOfRange,
      DuplicateKeys,
      StringContainsUnpairedUtf16HighSurrogate,
      StringContainsUnpairedUtf16LowSurrogate
    };
    size_t offset;
    std::string_view description;
    Code code = {};
  };
}
```
**`minjson::ParsingIssue`** bundles information about a parsing issue.

`offset` designates
* offset in bytes/code units of characters that caused the issue, or
* equals to the input size if unexpected end of input is encountered.

`description` contains textual human friendly description of the issue.

`code` contains a code from `minjson::ParsingIssue::Code` enumeration for distinguishing of the issue cause.\
`FailedToParseNumber` in theory should never happen but is included in case a particular implementation
of `std::from_chars()` (and libc++'s `strtod()`) fails for some unexpected reason.

-----

```c++
namespace minjson {
  enum class ParsingResultStatus {
    Failure,
    Success,
    PartialSuccess
  };
}
```
**`minjson::ParsingResultStatus`** signifies the result of parsing. See descriptions of
individual `parse()` functions below.

-----

```c++
namespace minjson {
  template<typename Allocator>
  struct BasicParsingResult {
    using Status = ParsingResultStatus;
    using Issues = std::vector<ParsingIssue,
      typename std::allocator_traits<Allocator>::template rebind_alloc<ParsingIssue>>;

    BasicValue<Allocator> value;
    Status status;
    size_t parsedSize;
    Issues issues;
  };

  using ParsingResult = BasicParsingResult<std::allocator<char>>;
}
```
**`minjson::BasicParsingResult`** template instantiation is returned by parsing functions.

`value` contains parsed JSON value in case of success.

`status` contains parsing result status, e.g. success or failure.

`parsedSize` constains size in bytes/code units of the input that was successfully parsed.
_Note that if end of input was reached unexpectedly it contains the size of the input._

`issues` contains a list of issues encountered during parsing.

**`minjson::ParsingResult`** alias is defined for convenience.

-----

```c++
namespace minjson {
  enum class ParsingMode {
    VerifyTrailingWhitespace,
    StopAfterValueEnds
  };
}
```
* `VerifyTrailingWhitespace` tells `minjson::parse()` to verify trailing whitespaces up to the end of input.
* `StopAfterValueEnds` stops parsing immediately after a valid JSON value.

-----

```c++
namespace minjson {
  template<typename Allocator = std::allocator<char>>
  [[nodiscard]] BasicParsingResult<Allocator> parse(std::string_view input,
                                                    const ParsingOptions &options = {},
                                                    ParsingMode parsingMode = ParsingMode::VerifyTrailingWhitespace,
                                                    const Allocator &allocator = {});
}
```
**`minjson::parse()`** parses _input_ string, returns an instance of `minjson::BasicParsingResult` with the result.

When `minjson::ParsingMode::VerifyTrailingWhitespace` is specified,
in case _input_ contains non-whitespace characters after a valid JSON value,
result's `status` field is set to `minjson::ParsingResultStatus::PartialSuccess` and
an issue is added to the `issues` list.

Provided `allocator` is used for all memory allocations.

`minjson::parse()` does not handle byte order mark, it is not skipped and is treated as invalid characters.

This function does not throw exceptions on its own.\
Otherwise fails only if underlying entity operation fails, e.g. as a result of failed allocation.

See [example](examples/#parse) in [parse.cpp](examples/parse.cpp).

-----

```c++
namespace minjson {
  namespace impl {
    template<typename Allocator>
    [[nodiscard]] BasicParsingResult<Allocator> parse(std::string_view input,
                                                      const ParsingOptions &options,
                                                      const Allocator &allocator);
  }
}
```
**`minjson::impl::parse()`** parses _input_ string, returns an instance of `minjson::BasicParsingResult` with the result.

Contrary to `minjson::parse()` always stops parsing immediately after a valid JSON value.\
This function never sets result's `status` to `minjson::ParsingResultStatus::PartialSuccess`.

Provided `allocator` is used for all memory allocations.

`minjson::impl::parse()` does not handle byte order mark, it is not skipped and is treated as invalid characters.

This function does not throw exceptions on its own.\
Otherwise fails only if underlying entity operation fails, e.g. as a result of failed allocation.

See [example](examples/#parse_impl) in [parse_impl.cpp](examples/parse_impl.cpp).

-----

### Polymorphic sinks

In order not to bloat the generated binary code with many types of sinks, you can use polymorphic sinks,
e.g.
```c++
std::function<void(const std::string_view&)>
```

You can specify it with `minjson::impl` versions of the functions, e.g. making a wrapper:
```c++
template<typename String_t = std::string>
[[nodiscard]] String_t myEscape(std::string_view input,
                                minjson::Escape escapeMode = {},
                                minjson::Utf8Validation validation = {},
                                minjson::HexDigitsCase hexDigitsCase = minjson::HexDigitsCase::Lower) {
  String_t s;
  if (validation == minjson::Utf8Validation::IgnoreInvalidUtf8CodeUnits)
    s.reserve(s.size());
  const size_t unescapedSize = minjson::impl::escape<std::function<void(const std::string_view&)>>(
    [&s](std::string_view t) { s += t; },
    input,
    escapeMode,
    validation,
    hexDigitsCase);
  return unescapedSize == input.size() ? std::move(s) : String_t{};
}
```
This way `minjson::impl::escape()` function template is _always_ instantiated with parameter
`std::function<void(const std::string_view&)>` regardless of the string type.


<a id="allocators"></a>
## Allocator support

### Support for _`Allocator`_ types

`minjson::BasicValue<Allocator>` template does not allocate memory for scalar types like `bool` or `int64_t`.\
However it uses an allocator for its stored string, array and object values.

You can instantiate `minjson::BasicValue` with a custom allocator, e.g. `std::pmr::polymorphic_allocator`.

Allocator type is supposed to satisfy [_`Allocator`_](https://en.cppreference.com/w/cpp/named_req/Allocator)
requirements.

By convention, in order not to multiply instantiations, `minjson::BasicValue` is instantiated
with an allocator type parameterized with `char` type,
e.g. a convenient alias for instantiation with `std::allocator` is already provided in the following form:
```c++
namespace minjson {
  using Value = BasicValue<std::allocator<char>>;
}
```

You can define your own instantiation like this:
```c++
using PmrValue = minjson::BasicValue<std::pmr::polymorphic_allocator<char>>;
```

### Allocator awareness

`std::basic_string`, `std::vector` and `std::unordered_map` are all allocator aware containers.

`minjson::BasicValue` is allocator aware in the sense that it bridges stored container's allocator awareness functionality
(see [_`AllocatorAwareContainer`_](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer) named requirements)
and the outside world. E.g. when stored in allocator aware containers
(e.g. `minjson::Array` which is an alias for `std::vector<minjson::Value>`) allocators are properly propagated into
`minjson::Value` and in turn into string, array and object values stored in each particular instance,
i.e. it's ~~turtles~~ allocator propagation all the way down.


## License

This software is licensed under the MIT license (SPDX identifier: MIT), see [LICENSE](LICENSE) file for full text.

As an exception [minjsoncpp.h](include/minjsoncpp.h) is available under
the [MIT No Attribution license (SPDX identifier: MIT-0)](https://opensource.org/license/mit-0).