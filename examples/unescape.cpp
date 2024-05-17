#include <minjsoncpp.h>

#include <iostream>

using namespace std::string_view_literals;

int main(int argc, const char *argv[]) {
  if (argc > 1) {
    const auto input = std::string_view{ argv[1] };
    if (!input.empty()) {
      const auto unescaped = minjson::unescape(input);
      if (unescaped.empty()) {
        std::cerr << "failed to unescape string";
        return 1;
      }
      std::cout << unescaped;
    }
    return 0;
  }

  const auto string = R"(string containing escaped characters: \t \\ \" \n (new line))"sv;

  const auto unescapedString = minjson::unescape(string);

  std::cout << "unescaping string:\n\n\'" << string << "\'\n\ngives:\n\n\'" << unescapedString << "\'\n";
}
