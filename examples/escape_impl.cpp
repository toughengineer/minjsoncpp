#include <minjsoncpp.h>

#include <iostream>

using namespace std::string_view_literals;

void writeEscapedToStdout(std::string_view s) {
  // a sink repeatedly receives parts of escaped string as 'std::string_view' instances
  const auto stdOutSink = [](std::string_view s) { std::cout << s; };
  // 'minjson::impl::escape()' can be called with any type of sink
  // that is able to receive parts as 'std::string_view' instances
  minjson::impl::escape(stdOutSink,
                        s,
                        minjson::Escape::Default,
                        minjson::Utf8Validation::IgnoreInvalidUtf8CodeUnits,
                        minjson::HexDigitsCase::Upper);
}

int main(int argc, const char *argv[]) {
  if (argc > 1) {
    writeEscapedToStdout(argv[1]);
    return 0;
  }

  const auto string = "string containing special characters: \t \\ \" \n (new line)"sv;
  std::cout << "escaping string:\n\n\'" << string << "\'\n\ngives:\n\n\'";

  writeEscapedToStdout(string);

  std::cout << "\'\n";
}
