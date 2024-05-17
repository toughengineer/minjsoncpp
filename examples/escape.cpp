#include <minjsoncpp.h>

#include <iostream>

using namespace std::string_view_literals;

int main(int argc, const char *argv[]) {
  if (argc > 1) {
    std::cout << minjson::escape(argv[1]);
    return 0;
  }

  const auto string = "string containing special characters: \t \\ \" \n (new line)"sv;

  const auto escapedString = minjson::escape(string);

  std::cout << "escaping string:\n\n\'" << string << "\'\n\ngives:\n\n\'" << escapedString << "\'\n";
}
