#include <minjsoncpp.h>

#include <iostream>

using namespace std::string_view_literals;

int main(int argc, const char *argv[]) {
  if (argc > 1) {
    const auto input = std::string_view{ argv[1] };
    if (!input.empty()) {
      // storage for unescaped string
      std::string unescapedString;
      // a sink repeatedly receives parts of unescaped string as 'std::string_view' instances
      const auto stringSink = [&unescapedString](std::string_view s) { unescapedString += s; };
      // 'minjson::impl::unescape()' can be called with any type of sink
      // that is able to receive parts as 'std::string_view' instances
      //
      // 'minjson::impl::unescape()' returns size of successfully unescaped input
      const auto unescapedSize = minjson::impl::unescape(stringSink, input, minjson::UnescapeMode::Relaxed);
      // if returned size != input size, input contains an invalid character or ends unexpectedly
      if (unescapedSize != input.size()) {
        if (unescapedSize == minjson::impl::NPos) {
          std::cerr << "failed to unescape string, unexpected end of input\n";
        } else {
          std::cerr << "failed to unescape string, invalid character at offset " << unescapedSize << '\n';
        }
        return 1;
      }
      std::cout << unescapedString;
    }
    return 0;
  }

  const auto string = R"(string containing escaped characters: \t \\ \" \n (new line))"sv;

  std::cout << "unescaping string:\n\n\'" << string << "\'\n\ngives:\n\n\'";

  const auto stdOutSink = [](std::string_view s) { std::cout << s; };
  minjson::impl::unescape(stdOutSink, string, minjson::UnescapeMode::Relaxed);

  std::cout << "\'\n";
}
