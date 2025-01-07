// Minimalistic JSON C++ library by Pavel Novikov (2024)
// for details see https://github.com/toughengineer/minjsoncpp
#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <variant>
#include <charconv>
#include <limits>
#include <memory>
#include <algorithm>
#include <type_traits>
#include <new>
#include <functional>
#include <ostream>
#include <stdexcept>
#include <utility>
#if defined(_LIBCPP_VERSION)
#include <cstring> // for memcpy()
#include <cstdlib> // for strtod()
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbitwise-op-parentheses"
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-label"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4309 4459 4102) // truncation of constant value; declaration hides global declaration; unreferenced label
#endif
#if defined(_MSC_VER) && _MSVC_STL_UPDATE < 202408L && \
    (defined(_MSVC_LANG) && _MSVC_LANG > __cplusplus && _MSVC_LANG == 201703L || __cplusplus == 201703L)
#define NEED_WORKAROUND_FOR_UNIMPLEMENTED_P0608
#endif

namespace minjson {
  inline constexpr std::string_view NullLiteral{ "null" };
  inline constexpr std::string_view FalseLiteral{ "false" };
  inline constexpr std::string_view TrueLiteral{ "true" };

  namespace detail {
    template<typename Allocator, typename T>
    using ReboundAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
    template<typename T>
    using RemoveCVRef = std::remove_cv_t<std::remove_reference_t<T>>;
    template<typename String, typename... T>
    inline constexpr bool AreTypesConvertibleToStringXorSizeT =
      ((std::is_convertible_v<T, String> != std::is_convertible_v<T, size_t>) && ...);
  }

  template<typename Allocator>
  struct BasicValue {
    using Null = std::monostate;
    using Boolean = bool;
    using String = std::basic_string<char, std::char_traits<char>, detail::ReboundAllocator<Allocator, char>>;
    using Array = std::vector<BasicValue, detail::ReboundAllocator<Allocator, BasicValue>>;
    using Object = std::unordered_map<String, BasicValue,
      std::hash<String>, std::equal_to<String>,
      detail::ReboundAllocator<Allocator, std::pair<const String, BasicValue>>>;
    using Variant = std::variant<Null, Boolean, int64_t, double, String, Array, Object>;

    BasicValue() = default;
#if defined(NEED_WORKAROUND_FOR_UNIMPLEMENTED_P0608)
    BasicValue(int i) noexcept : m_data{ std::in_place_type<int64_t>, i } {}
    BasicValue(const char *s) : m_data{ std::in_place_type<String>, s } {}
#endif
    BasicValue(std::string_view s) : m_data{ std::in_place_type<String>, s } {}
    template<typename T,
      std::enable_if_t<!std::is_same_v<detail::RemoveCVRef<T>, BasicValue> &&
      std::is_convertible_v<T&&, Variant>, int> = 0>
    BasicValue(T &&v) noexcept(std::is_nothrow_constructible_v<Variant, T&&>) : m_data{ std::forward<T>(v) } {}

    BasicValue(const Allocator&) noexcept {} // uses-allocator machinery support
    BasicValue(const BasicValue &other, const Allocator &a) : m_data{ construct(other.variant(), a) } {}
    BasicValue(BasicValue &&other, const Allocator &a) : m_data{ construct(std::move(other.variant()), a) } {}
#if defined(NEED_WORKAROUND_FOR_UNIMPLEMENTED_P0608)
    BasicValue &operator=(int i) noexcept { variant().template emplace<int64_t>(i); return *this; }
    BasicValue &operator=(const char *s) { variant().template emplace<String>(s); return *this; }
#endif
    BasicValue &operator=(std::string_view s) { variant().template emplace<String>(s); return *this; }
    template<typename T>
    std::enable_if_t<!std::is_same_v<detail::RemoveCVRef<T>, BasicValue> &&
      std::is_convertible_v<T&&, Variant>,
      BasicValue> &operator=(T &&v) noexcept(std::is_nothrow_assignable_v<Variant, T&&>) {
      variant() = std::forward<T>(v); return *this;
    }

    friend bool operator==(const BasicValue &a, const BasicValue &b) { return a.variant() == b.variant(); }
    friend bool operator!=(const BasicValue &a, const BasicValue &b) { return a.variant() != b.variant(); }

    [[nodiscard]] const Variant &variant() const& noexcept { return m_data; }
    [[nodiscard]] Variant &variant() & noexcept { return m_data; }
    [[nodiscard]] Variant &&variant() && noexcept { return std::move(m_data); }
    [[nodiscard]] const Variant &&variant() const&& noexcept { return std::move(m_data); }
    operator const Variant&() const& noexcept { return m_data; }
    operator Variant&() & noexcept { return m_data; }
    operator Variant && () && noexcept { return std::move(m_data); }
    operator const Variant && () const&& noexcept { return std::move(m_data); }

    [[nodiscard]] bool isObject() const noexcept { return std::holds_alternative<Object>(variant()); }
    [[nodiscard]] const Object &asObject() const& { return std::get<Object>(variant()); }
    [[nodiscard]] Object &asObject() & { return std::get<Object>(variant()); }
    [[nodiscard]] Object &&asObject() && { return std::move(std::get<Object>(variant())); }

    [[nodiscard]] bool isArray() const noexcept { return std::holds_alternative<Array>(variant()); }
    [[nodiscard]] const Array &asArray() const& { return std::get<Array>(variant()); }
    [[nodiscard]] Array &asArray() & { return std::get<Array>(variant()); }
    [[nodiscard]] Array &&asArray() && { return std::move(std::get<Array>(variant())); }

    [[nodiscard]] bool isString() const noexcept { return std::holds_alternative<String>(variant()); }
    [[nodiscard]] const String &asString() const& { return std::get<String>(variant()); }
    [[nodiscard]] String &asString() & { return std::get<String>(variant()); }
    [[nodiscard]] String &&asString() && { return std::move(std::get<String>(variant())); }

    [[nodiscard]] bool isDouble() const noexcept { return std::holds_alternative<double>(variant()); }
    [[nodiscard]] double asDouble() const { return std::get<double>(variant()); }
    [[nodiscard]] double &asDouble() { return std::get<double>(variant()); }

    [[nodiscard]] bool isInt() const noexcept { return std::holds_alternative<int64_t>(variant()); }
    [[nodiscard]] int64_t asInt() const { return std::get<int64_t>(variant()); }
    [[nodiscard]] int64_t &asInt() { return std::get<int64_t>(variant()); }

    [[nodiscard]] bool isBool() const noexcept { return std::holds_alternative<Boolean>(variant()); }
    [[nodiscard]] Boolean asBool() const { return std::get<Boolean>(variant()); }
    [[nodiscard]] Boolean &asBool() { return std::get<Boolean>(variant()); }

    [[nodiscard]] bool isNull() const noexcept { return std::holds_alternative<Null>(variant()); }

    template<typename... T>
    [[nodiscard]] std::enable_if_t<sizeof...(T) >= 1 && detail::AreTypesConvertibleToStringXorSizeT<String, T&&...>,
      const BasicValue*> resolve(T&&... refTokens) const noexcept {
      const BasicValue *value = this;
      (void)(static_cast<bool>(value = value->resolveImpl(std::forward<T>(refTokens))) && ...);
      return value;
    }
    template<typename... T>
    [[nodiscard]] std::enable_if_t<sizeof...(T) >= 1 && detail::AreTypesConvertibleToStringXorSizeT<String, T&&...>,
      BasicValue*> resolve(T&&... refTokens) noexcept {
      return const_cast<BasicValue*>(std::as_const(*this).resolve(std::forward<T>(refTokens)...));
    }

  private:
    template<typename T>
    Variant construct(T &&other, const Allocator &allocator) {
      if (other.valueless_by_exception())
        return std::forward<T>(other);
      return std::visit([&allocator](auto &&value) {
        using U = decltype(value);
        using ValueType = detail::RemoveCVRef<U>;
        if constexpr (std::is_same_v<ValueType, Object> ||
                      std::is_same_v<ValueType, Array> ||
                      std::is_same_v<ValueType, String>) {
          return Variant{ std::in_place_type<ValueType>, std::forward<U>(value), allocator };
        }
        else if constexpr (std::is_same_v<ValueType, Null>) {
          return Variant{ std::in_place_type<Null> };
        }
        else {
          return Variant{ std::in_place_type<ValueType>, value };
        }
                        }, std::forward<T>(other));
    }
    const BasicValue *resolveImpl(size_t index) const noexcept {
      if (auto *array = std::get_if<Array>(&variant()); array && index < array->size())
        return &(*array)[index];
      return nullptr;
    }
    const BasicValue *resolveImpl(const String &key) const noexcept {
      if (auto *object = std::get_if<Object>(&variant())) {
        if (auto i = object->find(key); i != object->end())
          return &i->second;
      }
      return nullptr;
    }

#if !defined(_GLIBCXX_RELEASE) || _GLIBCXX_RELEASE >= 12
    Variant m_data;
#else // std::unordered_map does not support incomplete types in libstdc++ version < 12
    struct Workaround final {
      Workaround() noexcept { ::new(&storage) Variant{}; }
      Workaround(const Workaround &other) { ::new(&storage) Variant{ other.operator const Variant & () }; }
      Workaround(Workaround &&other) noexcept(std::is_nothrow_move_constructible_v<Variant>) {
        ::new(&storage) Variant{ std::move(other.operator Variant & ()) };
      }
      template<typename... T>
      Workaround(T&&... v) noexcept(std::is_nothrow_constructible_v<Variant, T&&...>) {
        ::new(&storage) Variant{ std::forward<T>(v)... };
      }
      ~Workaround() {
        static_assert(sizeof(DummyVariant) == sizeof(Variant));
        static_assert(alignof(DummyVariant) == alignof(Variant));
        operator Variant&().~Variant();
      }
      auto &operator=(const Workaround &other) { return operator Variant & () = other.operator const Variant & (); }
      auto &operator=(Workaround &&other) noexcept(std::is_nothrow_move_assignable_v<Variant>) {
        return operator Variant & () = std::move(other.operator Variant & ());
      }
      operator const Variant&() const& noexcept { return *std::launder(reinterpret_cast<const Variant*>(&storage)); }
      operator Variant&() & noexcept { return *std::launder(reinterpret_cast<Variant*>(&storage)); }
      operator Variant && () && noexcept { return std::move(operator Variant & ()); }
      operator const Variant && () const&& noexcept { return std::move(operator const Variant & ()); }

    private:
      using DummyUnorderedMap = std::unordered_map<String, Array,
        std::hash<String>, std::equal_to<String>,
        detail::ReboundAllocator<Allocator, std::pair<const String, Array>>>;
      using DummyVariant = std::variant<Null, Boolean, int64_t, double, String, Array, DummyUnorderedMap>;
      alignas(DummyVariant) uint8_t storage[sizeof(DummyVariant)];
    } m_data;
#endif
  };
#if defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE < 12 && defined(__clang__)
  static_assert(std::is_copy_constructible_v<BasicValue<std::allocator<char>>>); // for some reason clang needs this
#endif

  using Value = BasicValue<std::allocator<char>>;
  using Object = Value::Object;
  using Array = Value::Array;
  using String = Value::String;
  using Boolean = bool;
  using Null = std::monostate;
  using Variant = Value::Variant;

  namespace detail {
    void GetValue(...); // intentionally not implemented
    template<typename Allocator>
    BasicValue<Allocator> GetValue(const BasicValue<Allocator>&); // intentionally not implemented
    template<typename T>
    using RawBaseValueType = decltype(GetValue(std::declval<RemoveCVRef<T>>()));
  }
  template<typename F, typename T>
  std::enable_if_t<std::is_base_of_v<detail::RawBaseValueType<T>, detail::RemoveCVRef<T>>,
    decltype(std::visit(std::declval<F&&>(), std::declval<T&&>().detail::template RawBaseValueType<T>::variant()))>
    visit(F &&f, T &&value) {
    return std::visit(std::forward<F>(f), std::forward<T>(value).detail::template RawBaseValueType<T>::variant());
  }


  enum class Escape {
    Default,
    NonAscii
  };
  enum class Utf8Validation {
    IgnoreInvalidUtf8CodeUnits, // a.k.a. "garbage in - garbage out"
    FailOnInvalidUtf8CodeUnits  // operation fails on first invalid UTF-8 code point/code unit
  };
  enum class HexDigitsCase {
    Lower,
    Upper
  };

  namespace detail {
    inline char matchCommonCharacterToEscape(char c) {
      switch (c) {
      case '\b': return 'b';
      case '\t': return 't';
      case '\n': return 'n';
      case '\f': return 'f';
      case '\r': return 'r';
      case '\"': return '\"';
      case '\\': return '\\';
      }
      return '\0';
    }
    inline bool isControlCharacter(char c) { return (c & '\xe0') == 0; }
    template<size_t N>
    bool isUtf8CodeUnit(char c) {
      constexpr uint8_t mask = static_cast<uint8_t>(0xffu << (7 - N));
      constexpr uint8_t pattern = static_cast<uint8_t>(0xffu << (8 - N));
      return (mask & static_cast<uint8_t>(c)) == pattern;
    }
    inline size_t getExpectedUtf8CodePointSize(char c) {
      if (isUtf8CodeUnit<2>(c)) return 2;
      if (isUtf8CodeUnit<3>(c)) return 3;
      if (isUtf8CodeUnit<4>(c)) return 4;
      return 1;
    }
    inline size_t detectUtf8CodePointSize(const char *begin, const char *end) {
      const size_t expectedSize = getExpectedUtf8CodePointSize(*begin);
      if (expectedSize != 1 && static_cast<size_t>(end - begin) >= expectedSize &&
          std::all_of(begin + 1, begin + expectedSize, &isUtf8CodeUnit<1>))
        return expectedSize;
      return 0;
    }
    template<size_t Size>
    inline constexpr uint32_t LowerBitsMask = (1u << Size) - 1u;
    template<typename Int_t, size_t Size0, size_t... Size1, typename T0, typename... T1>
    Int_t gatherBits(T0 c0, T1... c1) {
      static_assert(((Size0 <= sizeof(c0) * 8) && ... && (Size1 <= sizeof(c1) * 8)));
      static_assert((Size0 + ... + Size1) <= sizeof(Int_t) * 8);
      const Int_t result = static_cast<Int_t>(c0) & LowerBitsMask<Size0>;
      if constexpr (sizeof...(Size1) == 0) {
        return result;
      }
      else {
        return (result << (Size1 + ...)) | gatherBits<Int_t, Size1...>(c1...);
      }
    }
    struct EscapedChar final {
      EscapedChar(HexDigitsCase hexDigitsCase) {
        static constexpr char lower[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
        static constexpr char upper[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        hexDigits = hexDigitsCase == HexDigitsCase::Lower ? lower : upper;
      }
      std::string_view decorate(char escape) {
        buf[0] = '\\';
        buf[1] = escape;
        return { buf, 2 };
      }
      std::string_view get(char c) {
        write(buf, '0', '0', hexDigits[c >> 4], hexDigits[c & 0xfu]);
        return { buf, 6 };
      }
      std::string_view get(uint16_t c) {
        write(buf, c);
        return { buf, 6 };
      }
      std::string_view getSurrogates(uint32_t c) {
        const uint32_t surrogatePair = c - 0x10000u;
        write(buf, static_cast<uint16_t>((surrogatePair >> 10) | 0xd800u));
        write(buf + 6, static_cast<uint16_t>(surrogatePair & LowerBitsMask<10> | 0xdc00u));
        return { buf, 12 };
      }
    private:
      static void write(char *p, char c1, char c2, char c3, char c4) {
        p[0] = '\\';
        p[1] = 'u';
        p[2] = c1;
        p[3] = c2;
        p[4] = c3;
        p[5] = c4;
      }
      void write(char *p, uint16_t c) {
        write(p, hexDigits[c >> 12], hexDigits[(c >> 8) & 0xfu], hexDigits[(c >> 4) & 0xfu], hexDigits[c & 0xfu]);
      }
      const char *hexDigits;
      char buf[12];
    };

    template<typename Sink>
    struct EscapedStringWriter final {
      EscapedStringWriter(Sink &&sink, HexDigitsCase hexDigitsCase) :
        sink{ std::forward<Sink>(sink) }, escaped{ hexDigitsCase } {}
      const char *write(const char *begin, const char *end,
                        Escape escape, Utf8Validation validation) {
        if (escape == Escape::Default) {
          return validation == Utf8Validation::FailOnInvalidUtf8CodeUnits ?
            writeImpl<ValidateUtf8>(begin, end) :
            writeImpl<Default>(begin, end);
        }
        return validation == Utf8Validation::FailOnInvalidUtf8CodeUnits ?
          writeImpl<EscapeNonAsciiAndValidateUtf8>(begin, end) :
          writeImpl<EscapeNonAscii>(begin, end);
      }

    private:
      enum EscapeMode {
        Default = 0,
        EscapeNonAscii = 1,
        ValidateUtf8 = 1 << 1,
        EscapeNonAsciiAndValidateUtf8 = EscapeNonAscii | ValidateUtf8
      };
      template<EscapeMode escapeMode>
      const char *writeImpl(const char *begin, const char *end) {
        pendingBegin = begin;
        while (pendingBegin != end) {
          const char *i = pendingBegin;
          for (;;) {
            if (const char esc = matchCommonCharacterToEscape(*i)) {
              writeAndAdvance(i, escaped.decorate(esc), 1);
              break;
            }
            if (isControlCharacter(*i)) { // characters 0x0..0x1f must be escaped
              writeAndAdvance(i, escaped.get(*i), 1);
              break;
            }
            if constexpr (escapeMode != Default) {
              if (*i & '\x80') { // handling UTF-8 multibyte code point
                const size_t codePointSize = detectUtf8CodePointSize(i, end);
                if constexpr (escapeMode & EscapeNonAscii) {
                  if (codePointSize != 0) {
                    const auto escapeSequence = std::invoke([this, codePointSize, i] {
                      switch (codePointSize) {
                      default: // avoids error : non-void lambda does not return a value in all control paths
                      case 2: return escaped.get(gatherBits<uint16_t, 5, 6>(i[0], i[1]));
                      case 3: return escaped.get(gatherBits<uint16_t, 4, 6, 6>(i[0], i[1], i[2]));
                      case 4: return escaped.getSurrogates(gatherBits<uint32_t, 3, 6, 6, 6>(i[0], i[1], i[2], i[3]));
                      }
                                                            });
                    writeAndAdvance(i, escapeSequence, codePointSize);
                    break; // for (;;)
                  }
                  else if constexpr (escapeMode & ValidateUtf8) {
                    return i;
                  }
                }
                else /* escapeMode == ValidateUtf8 */ {
                  if (codePointSize == 0)
                    return i;
                  i += codePointSize;
                  goto CheckEnd;
                }
              }
            }
            ++i;
          CheckEnd:
            if (i == end) {
              writePending(end);
              return end;
            }
          } // for (;;)
        } // while ()
        return end;
      }
      void writePending(const char *i) {
        sink(std::string_view{ pendingBegin, static_cast<size_t>(i - pendingBegin) });
      }
      void writeAndAdvance(const char *i, std::string_view escapeSequence, size_t inc) {
        if (pendingBegin != i)
          writePending(i);
        pendingBegin = i + inc;
        sink(escapeSequence);
      }

      Sink &&sink;
      EscapedChar escaped;
      const char *pendingBegin;
    };

    template<typename String>
    struct StringSink final {
      void operator()(std::string_view t) { s += t; }
      String &s;
    };

    struct StdOStreamSink final {
      void operator()(std::string_view v) const { s.write(v.data(), v.size()); }
      std::ostream &s;
    };
  }

  namespace impl {
    template<typename Sink>
    size_t escape(Sink &&sink,
                  std::string_view s,
                  Escape escapeMode,
                  Utf8Validation validation,
                  HexDigitsCase hexDigitsCase) {
      detail::EscapedStringWriter<Sink> writer{ std::forward<Sink>(sink), hexDigitsCase };
      const char *escapedEnd = writer.write(s.data(), s.data() + s.size(), escapeMode, validation);
      return static_cast<size_t>(escapedEnd - s.data());
    }
  }

  template<typename String_t = String>
  [[nodiscard]] String_t escape(std::string_view s,
                                Escape escapeMode = {},
                                Utf8Validation validation = {},
                                HexDigitsCase hexDigitsCase = HexDigitsCase::Lower) {
    if (!s.empty()) {
      String_t result;
      if (validation == Utf8Validation::IgnoreInvalidUtf8CodeUnits)
        result.reserve(s.size());
      if (impl::escape(detail::StringSink<String_t>{ result }, s, escapeMode, validation, hexDigitsCase) == s.size())
        return result;
    }
    return {};
  }


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

  struct InvalidUtf8CodeUnitsError : std::runtime_error {
    InvalidUtf8CodeUnitsError(const char *msg, std::string_view codeUnits, size_t offset) : runtime_error{ msg },
      codeUnits{ codeUnits }, offset{ offset } {}
    const std::string codeUnits;
    const size_t offset;
  };

  namespace detail {
    enum class SerializationMode {
      WithoutIndentation,
      WithIndentation
    };
    template<typename Sink, typename Allocator, SerializationMode serializationMode>
    struct SerializingVisitor final {
      using Value = BasicValue<Allocator>;
      void operator()(const typename Value::Object &o) {
        if (o.empty() && !options.emptyObject.empty()) {
          sink(options.emptyObject);
          return;
        }
        sink(options.objectOpeningBrace);
        if (!o.empty()) {
          if constexpr (serializationMode == SerializationMode::WithIndentation) indentation += options.indent;
          writeNewlineAndIndentation(options.newline.afterObjectOpeningBrace);
          if (o.size() == 1) {
            writeObjectMember(*o.begin());
          }
          else if (options.sortObjectKeys) {
            std::vector<std::reference_wrapper<const typename Value::Object::value_type>> items{ o.begin(), o.end() };
            std::sort(items.begin(), items.end(),
                      [](auto a, auto b) { return std::get<0>(a.get()) < std::get<0>(b.get()); });
            writeObjectMembers(items.begin(), items.end());
          }
          else {
            writeObjectMembers(o.begin(), o.end());
          }
          if constexpr (serializationMode == SerializationMode::WithIndentation) indentation -= options.indent;
          writeNewlineAndIndentation(options.newline.beforeObjectClosingBrace);
        }
        else if constexpr (serializationMode == SerializationMode::WithIndentation) {
          writeNewlineAndIndentation(options.newline.afterObjectOpeningBrace || options.newline.beforeObjectClosingBrace);
        }
        sink(options.objectClosingBrace);
      }
      void operator()(const typename Value::Array &a) {
        if (a.empty() && !options.emptyArray.empty()) {
          sink(options.emptyArray);
          return;
        }
        sink(options.arrayOpeningBracket);
        if (!a.empty()) {
          if constexpr (serializationMode == SerializationMode::WithIndentation) indentation += options.indent;
          writeNewlineAndIndentation(options.newline.afterArrayOpeningBracket);
          auto i = a.begin();
          visit(*this, *i);
          for (++i; i != a.end(); ++i) {
            writeNewlineAndIndentation(options.newline.beforeArrayMemberSeparator);
            sink(options.arrayMemberSeparator);
            writeNewlineAndIndentation(options.newline.afterArrayMemberSeparator);
            visit(*this, *i);
          }
          if constexpr (serializationMode == SerializationMode::WithIndentation) indentation -= options.indent;
          writeNewlineAndIndentation(options.newline.beforeArrayClosingBracket);
        }
        else if constexpr (serializationMode == SerializationMode::WithIndentation) {
          writeNewlineAndIndentation(options.newline.afterArrayOpeningBracket || options.newline.beforeArrayClosingBracket);
        }
        sink(options.arrayClosingBracket);
      }
      void operator()(const typename Value::String &t) { writeString(t); }
      void operator()(double d) { writeNumber(d); }
      void operator()(int64_t i) { writeNumber(i); }
      void operator()(Boolean b) { b ? sink(options.trueLiteral) : sink(options.falseLiteral); }
      void operator()(Null) { sink(options.nullLiteral); }

      Sink &&sink;
      const SerializationOptions &options;
      const std::string_view newlineSeparator;
      size_t indentation;
      const std::string_view indentationChars = std::invoke([this] {
        using namespace std::string_view_literals;
        std::string_view chars;
        if constexpr (serializationMode == SerializationMode::WithIndentation)
          chars = options.indentationChar == SerializationOptions::IndentationChar::Space ?
          "                                                                                                    "sv :
          "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
          "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"sv;
        (void)this;
        return chars;
                                                            });

    private:
      void writeString(std::string_view s) const {
        sink(options.openingStringQuotation);
        const size_t escapedSize = impl::escape(sink, s, options.escape, options.validation, options.hexDigitsCase);
        if (options.validation == Utf8Validation::FailOnInvalidUtf8CodeUnits && escapedSize != s.size()) {
          const size_t expectedCodePointSize = getExpectedUtf8CodePointSize(s[escapedSize]);
          throw InvalidUtf8CodeUnitsError{ "string contains invalid UTF-8 code units",
            s.substr(escapedSize, std::min(expectedCodePointSize, s.size() - escapedSize)), escapedSize };
        }
        sink(options.closingStringQuotation);
      }
      void writeObjectMember(const typename Value::Object::value_type &i) {
        const auto &[key, value] = i;
        writeString(key);
        writeNewlineAndIndentation(options.newline.afterObjectMemberKey);
        sink(options.objectKeyValueSeparator);
        if constexpr (serializationMode == SerializationMode::WithIndentation)
          writeNewlineAndIndentation(
            options.newline.beforeObjectMemberValue ||
            options.newline.beforeObjectMemberCollectionValue && (value.isObject() || value.isArray()));
        visit(*this, value);
      }
      template<typename Iterator>
      void writeObjectMembers(Iterator begin, const Iterator &end) {
        writeObjectMember(*begin);
        for (++begin; begin != end; ++begin) {
          writeNewlineAndIndentation(options.newline.beforeObjectMemberSeparator);
          sink(options.objectMemberSeparator);
          writeNewlineAndIndentation(options.newline.afterObjectMemberSeparator);
          writeObjectMember(*begin);
        }
      }
      void writeNewlineAndIndentation([[maybe_unused]] bool doWrite) {
        if constexpr (serializationMode == SerializationMode::WithIndentation)
          if (doWrite) {
            sink(newlineSeparator);
            size_t indent = indentation;
            while (indent) {
              const size_t charsToWrite = std::min(indent, indentationChars.size());
              sink(std::string_view{ indentationChars.data(), charsToWrite });
              indent -= charsToWrite;
            }
          }
      }
      template<typename T>
      void writeNumber(T n) {
        char buf[24];
        const auto result = std::to_chars(buf, buf + sizeof(buf), n);
        sink(std::string_view{ buf, static_cast<size_t>(result.ptr - buf) });
      }
    };
  }

  namespace impl {
    template<typename Sink, typename Allocator>
    void serialize(Sink &&sink,
                   const BasicValue<Allocator> &v,
                   const SerializationOptions &options,
                   size_t initialIndentation = 0) {
      if (options.indent || !options.newline.separator.empty()) {
        using Visitor = detail::SerializingVisitor<Sink, Allocator, detail::SerializationMode::WithIndentation>;
        const std::string_view newlineSeparator = options.newline.separator.empty() ? "\n" : options.newline.separator;
        visit(Visitor{ std::forward<Sink>(sink), options, newlineSeparator, initialIndentation }, v);
      }
      else {
        using Visitor = detail::SerializingVisitor<Sink, Allocator, detail::SerializationMode::WithoutIndentation>;
        visit(Visitor{ std::forward<Sink>(sink), options }, v);
      }
    }
  }

  template<typename Allocator>
  void serializeToStream(std::ostream &s, const BasicValue<Allocator> &v, const SerializationOptions &o = {}) {
    impl::serialize(detail::StdOStreamSink{ s }, v, o);
  }

  template<typename Allocator>
  [[nodiscard]] typename BasicValue<Allocator>::String serializeToString(const BasicValue<Allocator> &v,
                                                                         const SerializationOptions &o = {}) {
    typename BasicValue<Allocator>::String s;
    impl::serialize(detail::StringSink<typename BasicValue<Allocator>::String>{ s }, v, o);
    return s;
  }


  enum class UnescapeMode {
    Relaxed,
    Strict
  };

  namespace detail {
    struct Utf8Encoder final {
      std::string_view encode123(uint32_t codePoint) {
        if (codePoint < 0x80u) {
          *buf = static_cast<char>(codePoint);
          return { buf, 1 };
        }
        else if (codePoint < 0x800u) {
          encodeCodePoint<5, 6>(buf, codePoint);
          return { buf, 2 };
        }
        else {
          encodeCodePoint<4, 6, 6>(buf, codePoint);
          return { buf, 3 };
        }
      }
      std::string_view encode4(uint32_t codePoint) {
        encodeCodePoint<3, 6, 6, 6>(buf, codePoint);
        return { buf, 4 };
      }
      std::string_view encode(uint32_t highSurrogate, uint32_t lowSurrogate) {
        return encode4((gatherBits<uint32_t, 10, 10>(highSurrogate, lowSurrogate)) + 0x10000u);
      }
      std::string_view encodeSurrogateCodeUnitAsCodePoint(uint32_t codeUnit, char32_t surrogateReplacement) {
        if (surrogateReplacement == ~char32_t{ 0 })
          return encode123(codeUnit);
        return surrogateReplacement >= U'\x10000' ? encode4(surrogateReplacement) : encode123(surrogateReplacement);
      }

    private:
      template<size_t Size0, size_t... Size1>
      static void encodeCodePoint(char *buf, uint32_t codePoint) {
        static_assert((Size0 + ... + Size1) <= 32);
        constexpr uint32_t mark = 0xfffffffeu << Size0;
        if constexpr (sizeof...(Size1) == 0) {
          *buf = static_cast<char>(codePoint & LowerBitsMask<Size0> | mark);
        }
        else {
          *buf = static_cast<char>((codePoint >> (Size1 + ...)) & LowerBitsMask<Size0> | mark);
          encodeCodePoint<Size1...>(buf + 1, codePoint);
        }
      }
      char buf[4];
    };
    inline bool isDecimalDigit(char c) { return '0' <= c && c <= '9'; }
    enum class EscapedStringParsingResult {
      Skip,
      Unescaped,
      HighSurrogate,
      LowSurrogate,
      JsonStringEnd,
      InvalidCharacter,
      UnexpectedEndOfInput
    };
    struct EscapedStringParser final {
      struct Result final {
        EscapedStringParsingResult result;
        std::string_view unescaped;
        uint32_t surrogate;
      };
      template<UnescapeMode mode>
      Result parseMore(const char *&begin, const char *end) {
        switch (*begin) {
        case '\\':
          if (++begin == end)
            return { EscapedStringParsingResult::UnexpectedEndOfInput };
          return parseEscape(begin, end);

        case '\"':
          return { EscapedStringParsingResult::JsonStringEnd };
        }
        if constexpr (mode == UnescapeMode::Strict)
          if (isControlCharacter(*begin))
            return { EscapedStringParsingResult::InvalidCharacter };
        ++begin;
        return { EscapedStringParsingResult::Skip };
      }

    private:
      Result parseEscape(const char *&begin, const char *end) {
        using namespace std::string_view_literals;
        switch (*begin) {
        default:
          return { EscapedStringParsingResult::InvalidCharacter };

        case '\"': ++begin; return { EscapedStringParsingResult::Unescaped, "\""sv };
        case '\\': ++begin; return { EscapedStringParsingResult::Unescaped, "\\"sv };
        case '/': ++begin; return { EscapedStringParsingResult::Unescaped, "/"sv };
        case 'b': ++begin; return { EscapedStringParsingResult::Unescaped, "\b"sv };
        case 'f': ++begin; return { EscapedStringParsingResult::Unescaped, "\f"sv };
        case 'n': ++begin; return { EscapedStringParsingResult::Unescaped, "\n"sv };
        case 'r': ++begin; return { EscapedStringParsingResult::Unescaped, "\r"sv };
        case 't': ++begin; return { EscapedStringParsingResult::Unescaped, "\t"sv };

        case 'u':
          break;
        }
        ++begin;
        uint32_t c = 0;
        const char *expectedEnd = begin + 4;
        for (; begin != expectedEnd; ++begin) {
          if (begin == end)
            return { EscapedStringParsingResult::UnexpectedEndOfInput };
          c <<= 4;
          if (isDecimalDigit(*begin)) {
            c |= *begin - '0';
          }
          else if (const char x = *begin | '\x20'; 'a' <= x && x <= 'f') {
            c |= x - ('a' - 10);
          }
          else {
            return { EscapedStringParsingResult::InvalidCharacter };
          }
        }
        switch (c & 0xfc00u) {
        case 0xd800u: return { EscapedStringParsingResult::HighSurrogate, {}, c };
        case 0xdc00u: return { EscapedStringParsingResult::LowSurrogate, {}, c };
        }
        return { EscapedStringParsingResult::Unescaped, encoder.encode123(c) };
      }
      Utf8Encoder encoder;
    };
    template<typename Sink>
    struct UnescapeParser final {
      UnescapeParser(Sink &&sink) : sink{ std::forward<Sink>(sink) } {}
      template<UnescapeMode mode>
      const char *unescape(std::string_view input, char32_t surrogateReplacement) {
        EscapedStringParser parser;
        Utf8Encoder encoder;
        const char *i = pendingBegin = input.data();
        const char *const end = i + input.size();
        do {
          const char *const pendingEnd = i;
          auto [result, unescaped, surrogate] = parser.parseMore<mode>(i, end);
          switch (result) {
          case EscapedStringParsingResult::JsonStringEnd:
            if constexpr (mode == UnescapeMode::Strict) {
              return i;
            }
            else {
              ++i;
            }
            break;

          case EscapedStringParsingResult::Skip:
            break;

          case EscapedStringParsingResult::Unescaped:
            writePending(pendingEnd);
            sink(unescaped);
            pendingBegin = i;
            break;

          case EscapedStringParsingResult::HighSurrogate:
            writePending(pendingEnd);
            for (;;) {
              if (i == end) {
                sink(encoder.encodeSurrogateCodeUnitAsCodePoint(surrogate, surrogateReplacement));
                return end;
              }
              pendingBegin = i;
              const auto [result2, unescaped2, surrogate2] = parser.parseMore<mode>(i, end);
              switch (result2) {
              case EscapedStringParsingResult::JsonStringEnd:
                if constexpr (mode == UnescapeMode::Strict) {
                  return i;
                }
                else {
                  ++i;
                }
                [[fallthrough]];
              case EscapedStringParsingResult::Skip:
                sink(encoder.encodeSurrogateCodeUnitAsCodePoint(surrogate, surrogateReplacement));
                goto BreakForLoop;

              case EscapedStringParsingResult::Unescaped:
                sink(encoder.encodeSurrogateCodeUnitAsCodePoint(surrogate, surrogateReplacement));
                sink(unescaped2);
                pendingBegin = i;
                goto BreakForLoop;

              case EscapedStringParsingResult::HighSurrogate:
                sink(encoder.encodeSurrogateCodeUnitAsCodePoint(surrogate, surrogateReplacement));
                surrogate = surrogate2;
                break;

              case EscapedStringParsingResult::LowSurrogate:
                sink(encoder.encode(surrogate, surrogate2));
                pendingBegin = i;
                goto BreakForLoop;

              case EscapedStringParsingResult::UnexpectedEndOfInput:
                return nullptr;

              default:
                return i;
              }
            } // for (;;)
          BreakForLoop:
            break;

          case EscapedStringParsingResult::LowSurrogate:
            writePending(pendingEnd);
            sink(encoder.encodeSurrogateCodeUnitAsCodePoint(surrogate, surrogateReplacement));
            pendingBegin = i;
            break;

          case EscapedStringParsingResult::UnexpectedEndOfInput:
            return nullptr;

          default:
            return i;
          }
        } while (i != end);
        writePending(end);
        return end;
      }

    private:
      void writePending(const char *pendingEnd) {
        if (pendingBegin != pendingEnd)
          sink(std::string_view{ pendingBegin, static_cast<size_t>(pendingEnd - pendingBegin) });
      }

      Sink &&sink;
      const char *pendingBegin;
    };
  }

  namespace impl {
    inline constexpr size_t NPos = ~size_t{ 0 };
    inline constexpr char32_t DoNotReplaceSurrogates = ~char32_t{ 0 };
    template<typename Sink>
    size_t unescape(Sink &&sink,
                    std::string_view input,
                    UnescapeMode mode,
                    char32_t surrogateReplacement = DoNotReplaceSurrogates) {
      if (input.empty())
        return 0;
      detail::UnescapeParser<Sink> parser{ std::forward<Sink>(sink) };
      const auto result =
        mode == UnescapeMode::Relaxed ? parser.template unescape<UnescapeMode::Relaxed>(input, surrogateReplacement) :
        /*mode == UnescapeMode::Strict*/parser.template unescape<UnescapeMode::Strict>(input, surrogateReplacement);
      return result == nullptr ? NPos : result - input.data();
    }
  }

  template<typename String_t = String>
  [[nodiscard]] String_t unescape(std::string_view input, UnescapeMode unescapeMode = {}) {
    String_t s;
    if (impl::unescape(detail::StringSink<String_t>{ s }, input, unescapeMode) == input.size())
      return s;
    return {};
  }
  template<typename String_t = String>
  [[nodiscard]] String_t unescape(std::string_view input,
                                  UnescapeMode unescapeMode,
                                  char32_t unpairedSurrogateReplacement) {
    String_t s;
    if (impl::unescape(detail::StringSink<String_t>{ s }, input, unescapeMode, unpairedSurrogateReplacement) == input.size())
      return s;
    return {};
  }


  struct ParsingOptions {
    enum class Option {
      Ignore,
      Report,
      Fail
    };
    Option duplicateObjectKeys = Option::Fail;
    Option unpairedUtf16Surrogates = Option::Ignore;
    bool replaceInvalidUtf16Surrogates = false;
    char32_t replacement = U'\xfffd'; // replacement character �
  };

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

  namespace detail {
    inline bool skipWhitespaces(const char *&begin, const char *end) {
      while (begin != end) {
        switch (*begin) {
        default:
          return false;

        case '\t':
        case '\n':
        case '\r':
        case ' ':
          ++begin;
        }
      }
      return true;
    }
    struct ParserImplBase {
      ParserImplBase(std::string_view input, const ParsingOptions &options) : input{ input }, options{ options },
        surrogateReplacement{ options.replaceInvalidUtf16Surrogates ? options.replacement : ~char32_t{ 0 } },
        i{ input.data() }, end{ input.data() + input.size() } {}
      size_t parsedSize() const { return static_cast<size_t>(i - input.data()); }

    protected:
      bool isEmpty() const { return i == end; }
      void skipDigits() {
        while (!isEmpty() && isDecimalDigit(*i))
          ++i;
      }

      const std::string_view input;
      const ParsingOptions &options;
      const char32_t surrogateReplacement;
      const char *i;
      const char *const end;
    };
    template<typename Allocator>
    struct ParserImpl final : ParserImplBase {
      using Value = BasicValue<Allocator>;
      using String = typename Value::String;
      using Variant = typename Value::Variant;

      ParserImpl(std::string_view input, const ParsingOptions &options, const Allocator &allocator) :
        ParserImplBase{ input, options }, issues{ allocator }, allocator{ allocator } {}
      bool parse(Value &v) {
        return !detectEndOfInputAfterSkippingWhitespaces() && parseImpl(v.variant());
      }

      std::vector<ParsingIssue, detail::ReboundAllocator<Allocator, ParsingIssue>> issues;

    private:
      bool parseImpl(Variant &v) {
        switch (*i) {
        case 'n': return parseLiteral<Null>(v, NullLiteral);
        case 'f': return parseLiteral<bool, false>(v, FalseLiteral);
        case 't': return parseLiteral<bool, true>(v, TrueLiteral);
        case '\"': return parseString(v);
        case '[': return parseArray(v);
        case '{': return parseObject(v);
        default: return parseNumber(v);
        }
      }

      bool detectEndOfInputAfterSkippingWhitespaces() {
        if (skipWhitespaces(i, end)) {
          addUnexpectedEndOfInputIssue();
          return true;
        }
        return false;
      }
      bool detectEndOfInput() {
        if (isEmpty()) {
          addUnexpectedEndOfInputIssue();
          return true;
        }
        return false;
      }
      bool advanceAndDetectEndOfInput() {
        ++i;
        return detectEndOfInput();
      }

      bool match(std::string_view pattern) {
        // first char should already be matched
        ++i;
        auto p = pattern.begin() + 1;
        for (;;) {
          if (detectEndOfInput())
            return false;
          if (*i != *p) {
            addInvalidCharacterIssue();
            return false;
          }
          ++i;
          if (++p == pattern.end())
            return true;
        }
      }
      template<typename T, T... value>
      bool parseLiteral(Variant &v, std::string_view pattern) {
        if (match(pattern)) {
          v.template emplace<T>(value...);
          return true;
        }
        return false;
      }

      bool parseString(String &s) {
        ++i; // opening '"' is already matched
        const char *pendingBegin = i;
        EscapedStringParser parser;
        Utf8Encoder encoder;
        while (!detectEndOfInput()) {
          const char *const pendingEnd = i;
          auto [result, unescaped, surrogate] = parser.parseMore<UnescapeMode::Strict>(i, end);
          using ParsingResult = EscapedStringParsingResult;
          switch (result) {
          case ParsingResult::Skip:
            break;

          case ParsingResult::JsonStringEnd:
            s.append(pendingBegin, i);
            ++i;
            return true;

          case ParsingResult::Unescaped:
            s.append(pendingBegin, pendingEnd);
            s += unescaped;
            pendingBegin = i;
            break;

          case ParsingResult::HighSurrogate:
            s.append(pendingBegin, pendingEnd);
            for (;;) {
              if (i == end) {
                addUnexpectedEndOfInputIssue();
                return false;
              }
              pendingBegin = i;
              auto [result2, unescaped2, surrogate2] = parser.parseMore<UnescapeMode::Strict>(i, end);
              switch (result2) {
              case ParsingResult::Skip:
                if (!checkInvalidUtf16HighSurrogateOptionAndEncode(s, surrogate, pendingBegin - 6))
                  return false;
                goto BreakForLoop;

              case ParsingResult::JsonStringEnd:
                ++i;
                return checkInvalidUtf16HighSurrogateOptionAndEncode(s, surrogate, pendingBegin - 6);

              case ParsingResult::Unescaped:
                if (!checkInvalidUtf16HighSurrogateOptionAndEncode(s, surrogate, pendingBegin - 6))
                  return false;
                s += unescaped2;
                pendingBegin = i;
                goto BreakForLoop;

              case ParsingResult::HighSurrogate:
                if (!checkInvalidUtf16HighSurrogateOptionAndEncode(s, surrogate, pendingBegin - 6))
                  return false;
                surrogate = surrogate2;
                break;

              case ParsingResult::LowSurrogate:
                s += encoder.encode(surrogate, surrogate2);
                pendingBegin = i;
                goto BreakForLoop;

              case ParsingResult::InvalidCharacter: goto InvalidCharacter;
              case ParsingResult::UnexpectedEndOfInput: goto UnexpectedEndOfInput;
              }
            }
          BreakForLoop:
            break;

          case ParsingResult::LowSurrogate:
            s.append(pendingBegin, pendingEnd);
            if (!checkInvalidUtf16LowSurrogateOptionAndEncode(s, surrogate, pendingEnd))
              return false;
            pendingBegin = i;
            break;

          case ParsingResult::InvalidCharacter:
          InvalidCharacter:
            addInvalidCharacterIssue("invalid character while parsing string");
            return false;

          case ParsingResult::UnexpectedEndOfInput:
          UnexpectedEndOfInput:
            addUnexpectedEndOfInputIssue();
            return false;
          }
        }
        return false;
      }
      bool parseString(Variant &v) {
        return parseString(v.template emplace<String>(allocator));
      }
      bool parseArray(Variant &v) {
        ++i; // opening '[' is already matched
        if (detectEndOfInputAfterSkippingWhitespaces())
          return false;
        auto &a = v.template emplace<typename Value::Array>(allocator);
        if (*i == ']') {
          ++i;
          return true;
        }
        Value u;
        if (parseArrayMembers<IterationMode::Recursion>(a, &u, size_t{ 1 })) {
          a[0] = std::move(u);
          return true;
        }
        return false;
      }
      enum class IterationMode { Recursion, Loop };
      template<IterationMode mode, typename... Size>
      bool parseArrayMembers(typename Value::Array &a, Value *v, Size... size_) {
        static_assert(sizeof...(size_) == (mode == IterationMode::Recursion ? 1 : 0));
        for (;;) {
          if (!parseImpl(v->variant()) || detectEndOfInputAfterSkippingWhitespaces())
            return false;
          switch (*i) {
          default:
            addInvalidCharacterIssue("invalid character, ',' or ']' expected");
            return false;

          case ']':
            ++i;
            if constexpr (mode == IterationMode::Recursion)
              a.resize((size_ + ... + 0));
            return true;

          case ',':
            ++i;
            if (detectEndOfInputAfterSkippingWhitespaces())
              return false;
            if constexpr (mode == IterationMode::Recursion) {
              const size_t size = (size_ + ... + 0);
              if (size > 42) { // too deep recursion, parsing the rest iteratively
                a.resize(size + 1);
                return parseArrayMembers<IterationMode::Loop>(a, &a.back());
              }
              Value u;
              if (parseArrayMembers<IterationMode::Recursion>(a, &u, size + 1)) {
                a[size] = std::move(u);
                return true;
              }
              return false;
            }
            else {
              v = &a.emplace_back();
            }
          }
        }
      }
      bool parseObject(Variant &v) {
        ++i; // opening '{' is already matched
        if (detectEndOfInputAfterSkippingWhitespaces())
          return false;
        auto &o = v.template emplace<typename Value::Object>(allocator);
        if (*i == '}') {
          ++i;
          return true;
        }
        for (;;) {
          if (*i != '\"') {
            addInvalidCharacterIssue("invalid character, JSON string expected");
            return false;
          }
          const char *keyBegin = i;
          String key{ allocator };
          if (!parseString(key) || detectEndOfInputAfterSkippingWhitespaces())
            return false;
          if (*i != ':') {
            addInvalidCharacterIssue("invalid character, ':' expected");
            return false;
          }
          ++i;
          if (detectEndOfInputAfterSkippingWhitespaces())
            return false;
          const auto [it, isInserted] = o.try_emplace(std::move(key));
          if (!isInserted && options.duplicateObjectKeys != ParsingOptions::Option::Ignore) {
            addIssue(keyBegin, "JSON object contains duplicate keys", ParsingIssue::Code::DuplicateKeys);
            if (options.duplicateObjectKeys == ParsingOptions::Option::Fail)
              return false;
          }
          if (!parseImpl(it->second.variant()) || detectEndOfInputAfterSkippingWhitespaces())
            return false;
          switch (*i) {
          default:
            addInvalidCharacterIssue("invalid character, ',' or '}' expected");
            return false;

          case '}':
            ++i;
            return true;

          case ',':
            ++i;
            break;
          }
          if (detectEndOfInputAfterSkippingWhitespaces())
            return false;
        }
      }

      bool detectDigits() {
        if (!isDecimalDigit(*i)) { // should be at least one digit after decimal point
          addInvalidCharacterIssue("invalid character, decimal digit expected");
          return false;
        }
        ++i;
        skipDigits();
        return true;
      }
      bool parseNumber(Variant &v) {
        using namespace std::string_view_literals;
        const char *const begin = i;
        // detecting integer part
        const size_t isNegative = *i == '-' ? 1 : 0;
        if (isNegative && advanceAndDetectEndOfInput())
          return false;
        switch (*i) {
        case '0':
          ++i;
          break; // leading zero can't be followed by more digits

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          ++i;
          skipDigits();
          break;

        default:
          addInvalidCharacterIssue();
          return false;
        }
        bool isDecimal = false;
        // detecting fraction part
        if (!isEmpty() && *i == '.') {
          if (advanceAndDetectEndOfInput() || !detectDigits())
            return false;
          isDecimal = true;
        }
        // detecting exponent part
        if (!isEmpty() && (*i | '\x20') == 'e') {
          if (advanceAndDetectEndOfInput())
            return false;
          switch (*i) {
          case '+':
          case '-':
            if (advanceAndDetectEndOfInput())
              return false;
          }
          if (!detectDigits())
            return false;
          isDecimal = true;
        }
        if (!isDecimal) {
          const size_t length = i - begin - isNegative;
          constexpr std::string_view maxValue[] = { "9223372036854775807"sv, "9223372036854775808"sv };
          if (length < 19 || length == 19 && std::string_view{ begin + isNegative, 19 } <= maxValue[isNegative]) {
            int64_t integer;
            const auto result = std::from_chars(begin, i, integer);
            if (result.ec != std::errc{}) {
              addIssue(result.ptr, "failed to parse number value"sv, ParsingIssue::Code::FailedToParseNumber);
              return false;
            }
            v.template emplace<int64_t>(integer);
            return true;
          }
        }
        double decimal;
#if defined(_LIBCPP_VERSION) // libc++ does not implement std::from_chars() for double
        char buf[1024];
        if (static_cast<size_t>(i - begin) > sizeof(buf) - 1) {
          addIssue(begin, "parsed number value is out of range"sv, ParsingIssue::Code::ParsedNumberOutOfRange);
          return false;
        }
        std::memcpy(buf, begin, static_cast<size_t>(i - begin));
        buf[i - begin] = '\0';
        char *bufEnd;
        decimal = std::strtod(buf, &bufEnd);
        if (decimal == HUGE_VAL || decimal == -HUGE_VAL) {
          addIssue(begin, "parsed number value is out of range"sv, ParsingIssue::Code::ParsedNumberOutOfRange);
          return false;
        }
        if (bufEnd != buf + static_cast<size_t>(i - begin)) {
          addIssue(begin + (bufEnd - buf), "failed to parse number value"sv, ParsingIssue::Code::FailedToParseNumber);
          return false;
        }
#else
        const auto result = std::from_chars(begin, i, decimal);
        if (result.ec == std::errc::result_out_of_range) {
          addIssue(begin, "parsed number value is out of range"sv, ParsingIssue::Code::ParsedNumberOutOfRange);
          return false;
        }
        else if (result.ec != std::errc{}) {
          addIssue(result.ptr, "failed to parse number value"sv, ParsingIssue::Code::FailedToParseNumber);
          return false;
        }
#endif
        v.template emplace<double>(decimal);
        return true;
      }

      void addUnexpectedEndOfInputIssue() {
        issues.push_back({ input.size(), "unexpected end of input", ParsingIssue::Code::UnexpectedEndOfInput });
      }
      void addIssue(const char *p, std::string_view description, ParsingIssue::Code code) {
        issues.push_back({ static_cast<size_t>(p - input.data()), description, code });
      }
      void addIssue(std::string_view description, ParsingIssue::Code code) {
        addIssue(i, description, code);
      }
      void addInvalidCharacterIssue(std::string_view msg = "invalid character") {
        addIssue(msg, ParsingIssue::Code::InvalidCharacter);
      }
      bool checkInvalidUtf16SurrogateOptionAndEncode(String &s, uint32_t surrogate,
                                                     const char *p,
                                                     std::string_view description,
                                                     ParsingIssue::Code code) {
        if (options.unpairedUtf16Surrogates != ParsingOptions::Option::Ignore) {
          addIssue(p, description, code);
          if (options.unpairedUtf16Surrogates == ParsingOptions::Option::Fail)
            return false;
        }
        s += Utf8Encoder{}.encodeSurrogateCodeUnitAsCodePoint(surrogate, surrogateReplacement);
        return true;
      }
      bool checkInvalidUtf16HighSurrogateOptionAndEncode(String &s, uint32_t surrogate, const char *p) {
        return checkInvalidUtf16SurrogateOptionAndEncode(s, surrogate,
                                                         p, "string contains unpaired UTF-16 high surrogate",
                                                         ParsingIssue::Code::StringContainsUnpairedUtf16HighSurrogate);
      }
      bool checkInvalidUtf16LowSurrogateOptionAndEncode(String &s, uint32_t surrogate, const char *p) {
        return checkInvalidUtf16SurrogateOptionAndEncode(s, surrogate,
                                                         p, "string contains unpaired UTF-16 low surrogate",
                                                         ParsingIssue::Code::StringContainsUnpairedUtf16LowSurrogate);
      }

      Allocator allocator;
    };
  }

  enum class ParsingResultStatus {
    Failure,
    Success,
    PartialSuccess
  };
  template<typename Allocator>
  struct BasicParsingResult {
    using Status = ParsingResultStatus;
    using Issues = std::vector<ParsingIssue, detail::ReboundAllocator<Allocator, ParsingIssue>>;

    BasicValue<Allocator> value;
    Status status;
    size_t parsedSize;
    Issues issues;
  };
  using ParsingResult = BasicParsingResult<std::allocator<char>>;

  namespace impl {
    template<typename Allocator>
    [[nodiscard]] BasicParsingResult<Allocator> parse(std::string_view input,
                                                      const ParsingOptions &options,
                                                      const Allocator &allocator) {
      detail::ParserImpl<Allocator> parser{ input, options, allocator };
      BasicParsingResult<Allocator> result{ {}, {}, {}, typename BasicParsingResult<Allocator>::Issues{ allocator } };
      result.status = parser.parse(result.value) ? ParsingResultStatus::Success : ParsingResultStatus::Failure;
      result.issues = std::move(parser.issues);
      result.parsedSize = parser.parsedSize();
      return result;
    }
  }

  enum class ParsingMode {
    VerifyTrailingWhitespace,
    StopAfterValueEnds
  };

  template<typename Allocator = std::allocator<char>>
  [[nodiscard]] BasicParsingResult<Allocator> parse(std::string_view input,
                                                    const ParsingOptions &options = {},
                                                    ParsingMode parsingMode = ParsingMode::VerifyTrailingWhitespace,
                                                    const Allocator &allocator = {}) {
    BasicParsingResult<Allocator> result = impl::parse<Allocator>(input, options, allocator);
    if (result.status == ParsingResultStatus::Success && parsingMode == ParsingMode::VerifyTrailingWhitespace) {
      const char *begin = input.data() + result.parsedSize;
      const bool reachedEnd = detail::skipWhitespaces(begin, input.data() + input.size());
      result.parsedSize = static_cast<size_t>(begin - input.data());
      if (!reachedEnd) {
        result.status = ParsingResultStatus::PartialSuccess;
        result.issues.push_back({ result.parsedSize, "non-whitespace characters after a valid JSON value" });
      }
    }
    return result;
  }
}

namespace std {
  template<typename A1, typename A2> // uses-allocator machinery support
  struct uses_allocator<minjson::BasicValue<A1>, A2> : true_type {};
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(GCC)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
