#include <utility>  // dummy include to get _LIBCPP_VERSION
// libc++ version 15 and earlier does not provide <memory_resource>
#if !defined(_LIBCPP_VERSION) || _LIBCPP_VERSION >= 160000
#include <minjsoncpp.h>

#include "utils.h"

#include <catch2/catch_all.hpp>

#include <memory_resource>

using namespace std::string_view_literals;

#if defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE < 12 && defined(__clang__)
// for some reason clang needs this
static_assert(std::is_copy_constructible_v<minjson::BasicValue<std::pmr::polymorphic_allocator<char>>>);
#endif

namespace {
struct CountingMemoryResource : std::pmr::memory_resource {
  CountingMemoryResource(std::pmr::memory_resource *upstream) : upstream{ upstream } {}
  void *do_allocate(std::size_t bytes, std::size_t alignment) override {
    ++allocationCount;
    return upstream->allocate(bytes, alignment);
  }
  void do_deallocate(void *p, std::size_t bytes, std::size_t alignment) override {
    ++deallocationCount;
    upstream->deallocate(p, bytes, alignment);
  }
  bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override {
    return this == &other;
  }

  size_t allocationCount = 0;
  size_t deallocationCount = 0;

private:
  std::pmr::memory_resource *upstream;
};

struct TestFixture {
  TestFixture() {
    std::pmr::set_default_resource(&memoryResource);
  }
  ~TestFixture() {
    std::pmr::set_default_resource(std::pmr::new_delete_resource());
  }

  CountingMemoryResource memoryResource{ std::pmr::new_delete_resource() };
};
}  // namespace

using Allocator = std::pmr::polymorphic_allocator<char>;
using Json = minjson::BasicValue<Allocator>;


TEST_CASE_METHOD(TestFixture, "creating value using custom allocator", "[value][allocator]") {
  {
    Allocator allocator = std::pmr::new_delete_resource();
    Json::Object o{ allocator };
    INFO("creating object members...");
    o[std::pmr::string{ "foo", allocator }] = 42;
    o[std::pmr::string{ "bar", allocator }] = std::pmr::string{ "a longish string of text", allocator };
    o[std::pmr::string{ "baz", allocator }] = Json::Array{ { 1, 2, 3 }, allocator };
    Json v = std::move(o);
    CHECK(memoryResource.allocationCount == 0);

    INFO("copying value...");
    auto copy = Json{ v, allocator };
    CHECK(memoryResource.allocationCount == 0);

    INFO("moving value...");
    const auto moved = Json{ std::move(v) };
    const auto moved2 = Json{ std::move(v), allocator };
  }
  REQUIRE(memoryResource.allocationCount == 0);
  REQUIRE(memoryResource.deallocationCount == 0);
}


TEST_CASE_METHOD(TestFixture, "parsing value using custom allocator", "[value][allocator]") {
  const auto json = R"({
"foo": 42,
"bar": "a longish string of text",
"baz": [
  1, 2, "three", "another longish string of text"
]
})"sv;
  {
    Allocator allocator = std::pmr::new_delete_resource();
    auto [value, status, parsedSize, issues] = minjson::parse(json, {}, {}, allocator);
    CHECK(status == minjson::ParsingResultStatus::Success);
    CHECK(parsedSize == json.size());
    CHECK(issues.empty());
  }
  REQUIRE(memoryResource.allocationCount == 0);
  REQUIRE(memoryResource.deallocationCount == 0);
}
#endif  // !defined(_LIBCPP_VERSION) || _LIBCPP_VERSION >= 160000
