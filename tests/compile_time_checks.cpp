#include <minjsoncpp.h>

static_assert(std::is_nothrow_default_constructible_v<minjson::Value>);

void testVariantAccess() {
  minjson::Value v;
  {
    auto &&lvalueRef = static_cast<minjson::Value &>(v).variant();
    static_assert(std::is_same_v<decltype(lvalueRef), minjson::Variant &>);
  }
  {
    auto &&constLValueRef = static_cast<const minjson::Value &>(v).variant();
    static_assert(std::is_same_v<decltype(constLValueRef), const minjson::Variant &>);
  }
  {
    auto &&rvalueRef = static_cast<minjson::Value &&>(v).variant();
    static_assert(std::is_same_v<decltype(rvalueRef), minjson::Variant &&>);
  }
  {
    auto &&constRValueRef = static_cast<const minjson::Value &&>(v).variant();
    static_assert(std::is_same_v<decltype(constRValueRef), const minjson::Variant &&>);
  }

  [[maybe_unused]] minjson::Variant &lvalueRef = v;
  [[maybe_unused]] const minjson::Variant &constLValueRef = v;
  [[maybe_unused]] minjson::Variant &&rvalueRef = std::move(v);
  [[maybe_unused]] const minjson::Variant &&constRValueRef = std::move(v);
}


void testValueAccess() {
  minjson::Value v;
  // string
  {
    auto &&lvalueRef = static_cast<minjson::Value &>(v).asString();
    static_assert(std::is_same_v<decltype(lvalueRef), minjson::String &>);
  }
  {
    auto &&constLValueRef = static_cast<const minjson::Value &>(v).asString();
    static_assert(std::is_same_v<decltype(constLValueRef), const minjson::String &>);
  }
  {
    auto &&rvalueRef = static_cast<minjson::Value &&>(v).asString();
    static_assert(std::is_same_v<decltype(rvalueRef), minjson::String &&>);
  }
  {
    auto &&constLValueRef = static_cast<const minjson::Value &&>(v).asString();
    static_assert(std::is_same_v<decltype(constLValueRef), const minjson::String &>);
  }
  // array
  {
    auto &&lvalueRef = static_cast<minjson::Value &>(v).asArray();
    static_assert(std::is_same_v<decltype(lvalueRef), minjson::Array &>);
  }
  {
    auto &&constLValueRef = static_cast<const minjson::Value &>(v).asArray();
    static_assert(std::is_same_v<decltype(constLValueRef), const minjson::Array &>);
  }
  {
    auto &&rvalueRef = static_cast<minjson::Value &&>(v).asArray();
    static_assert(std::is_same_v<decltype(rvalueRef), minjson::Array &&>);
  }
  {
    auto &&constLValueRef = static_cast<const minjson::Value &&>(v).asArray();
    static_assert(std::is_same_v<decltype(constLValueRef), const minjson::Array &>);
  }
  // object
  {
    auto &&lvalueRef = static_cast<minjson::Value &>(v).asObject();
    static_assert(std::is_same_v<decltype(lvalueRef), minjson::Object &>);
  }
  {
    auto &&constLValueRef = static_cast<const minjson::Value &>(v).asObject();
    static_assert(std::is_same_v<decltype(constLValueRef), const minjson::Object &>);
  }
  {
    auto &&rvalueRef = static_cast<minjson::Value &&>(v).asObject();
    static_assert(std::is_same_v<decltype(rvalueRef), minjson::Object &&>);
  }
  {
    auto &&constLValueRef = static_cast<const minjson::Value &&>(v).asObject();
    static_assert(std::is_same_v<decltype(constLValueRef), const minjson::Object &>);
  }
}


namespace {
template<typename... T>
std::false_type IsResolveAllowedImpl(...);
template<typename... T>
std::true_type
IsResolveAllowedImpl(std::void_t<decltype(std::declval<minjson::Value>().resolve(std::declval<T>()...))> *);
template<typename... T>
constexpr bool IsResolveAllowed = decltype(IsResolveAllowedImpl<T...>(nullptr))::value;

// test that `resolve()` is not allowed with 0 arguments
static_assert(!IsResolveAllowed<>);

struct MyString {
  operator std::string() const;
};
struct MyIndex {
  operator size_t() const;
};

// test that `resolve()` is allowed with arguments convertible to string or size_t
static_assert(IsResolveAllowed<const char *>);
static_assert(IsResolveAllowed<std::string>);
static_assert(IsResolveAllowed<MyString>);
static_assert(IsResolveAllowed<int>);
static_assert(IsResolveAllowed<size_t>);
static_assert(IsResolveAllowed<MyIndex>);

static_assert(IsResolveAllowed<const char *, std::string>);
static_assert(IsResolveAllowed<const char *, MyString>);
static_assert(IsResolveAllowed<std::string, MyString>);
static_assert(IsResolveAllowed<const char *, std::string, MyString>);

static_assert(IsResolveAllowed<int, size_t>);
static_assert(IsResolveAllowed<int, MyIndex>);
static_assert(IsResolveAllowed<size_t, MyIndex>);
static_assert(IsResolveAllowed<int, size_t, MyIndex>);

static_assert(IsResolveAllowed<const char *, int>);
static_assert(IsResolveAllowed<size_t, std::string>);
static_assert(IsResolveAllowed<MyString, MyIndex, MyIndex, MyString>);


// test that `resolve()` is not allowed with arguments _not_ convertible to string or size_t
static_assert(!IsResolveAllowed<void *>);
static_assert(!IsResolveAllowed<std::vector<char>>);
static_assert(!IsResolveAllowed<std::vector<size_t>>);


struct DummyVisitor {
  template<typename T>
  void operator()(T &&) const {}
};

template<typename, typename = void>
struct IsVisitationAllowed : std::false_type {};
template<typename T>
struct IsVisitationAllowed<T, std::void_t<decltype(minjson::visit(DummyVisitor{}, std::declval<T>()))>>
  : std::true_type {};

template<typename, typename = void>
struct IsVisitationAllowedViaAdl : std::false_type {};
template<typename T>
struct IsVisitationAllowedViaAdl<T, std::void_t<decltype(visit(DummyVisitor{}, std::declval<T>()))>> : std::true_type {
};

// test that visitation is allowed for `minjson::Value`
static_assert(IsVisitationAllowed<minjson::Value>::value);
static_assert(IsVisitationAllowed<const minjson::Value>::value);

static_assert(IsVisitationAllowed<minjson::Value &>::value);
static_assert(IsVisitationAllowed<const minjson::Value &>::value);

static_assert(IsVisitationAllowedViaAdl<minjson::Value>::value);
static_assert(IsVisitationAllowedViaAdl<const minjson::Value>::value);

static_assert(IsVisitationAllowedViaAdl<minjson::Value &>::value);
static_assert(IsVisitationAllowedViaAdl<const minjson::Value &>::value);

// type publicly derived from `minjson::Value` (and defining its own `variant()` method)
struct MyValue : minjson::Value {
  void variant() const {}
};

// test that visitation is allowed for properly derived type
static_assert(IsVisitationAllowed<MyValue>::value);
static_assert(IsVisitationAllowed<const MyValue>::value);

static_assert(IsVisitationAllowed<MyValue &>::value);
static_assert(IsVisitationAllowed<const MyValue &>::value);

static_assert(IsVisitationAllowedViaAdl<MyValue>::value);
static_assert(IsVisitationAllowedViaAdl<const MyValue>::value);

static_assert(IsVisitationAllowedViaAdl<MyValue &>::value);
static_assert(IsVisitationAllowedViaAdl<const MyValue &>::value);

// unrelated type
struct Dummy {
  void variant() const {}
};

// test that visitation is not allowed for unrelated type even with defined `variant()` method
static_assert(!IsVisitationAllowed<Dummy>::value);
static_assert(!IsVisitationAllowed<const Dummy>::value);

static_assert(!IsVisitationAllowed<Dummy &>::value);
static_assert(!IsVisitationAllowed<const Dummy &>::value);

static_assert(!IsVisitationAllowedViaAdl<Dummy>::value);
static_assert(!IsVisitationAllowedViaAdl<const Dummy>::value);

static_assert(!IsVisitationAllowedViaAdl<Dummy &>::value);
static_assert(!IsVisitationAllowedViaAdl<const Dummy &>::value);
}  // namespace

void testVisit() {
  // test that visit forwards the right reference type
  {
    minjson::Value v;
    visit(
      [](auto &&v) {
        using T = decltype(v) &&;
        static_assert(std::is_reference_v<T> && !std::is_rvalue_reference_v<T> &&
                      !std::is_const_v<std::remove_reference_t<T>>);
      },
      v);
    visit(
      [](auto &&v) {
        using T = decltype(v) &&;
        static_assert(std::is_rvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>);
      },
      static_cast<minjson::Value &&>(v));
  }
  {
    const minjson::Value v;
    visit(
      [](auto &&v) {
        using T = decltype(v) &&;
        static_assert(std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>);
      },
      v);
    visit(
      [](auto &&v) {
        using T = decltype(v) &&;
        static_assert(std::is_rvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>);
      },
      static_cast<const minjson::Value &&>(v));
  }
  // derived type
  {
    MyValue v;
    visit(
      [](auto &&v) {
        using T = decltype(v) &&;
        static_assert(std::is_reference_v<T> && !std::is_rvalue_reference_v<T> &&
                      !std::is_const_v<std::remove_reference_t<T>>);
      },
      v);
    visit(
      [](auto &&v) {
        using T = decltype(v) &&;
        static_assert(std::is_rvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>);
      },
      static_cast<MyValue &&>(v));
  }
  {
    const MyValue v;
    visit(
      [](auto &&v) {
        using T = decltype(v) &&;
        static_assert(std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>);
      },
      v);
    visit(
      [](auto &&v) {
        using T = decltype(v) &&;
        static_assert(std::is_rvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>);
      },
      static_cast<const MyValue &&>(v));
  }
}

namespace {
struct DummySink {
  template<typename T>
  void operator()(T &&) const {
    static_assert(std::is_same_v<std::remove_const_t<std::remove_reference_t<T>>, std::string_view>);
  }
};
}  // namespace

auto testSinkApi() {
  minjson::impl::escape(DummySink{}, {}, {}, {}, {});
  minjson::impl::serialize(DummySink{}, minjson::Value{}, {});
  minjson::impl::unescape(DummySink{}, {}, {}, U'\uFFFD');
  const auto result = minjson::impl::parse({}, {}, std::allocator<char>{});
  return result;
}
