#include <minjsoncpp.h>

#include <iostream>
#include <iterator>

using namespace std::string_view_literals;

// auxiliary stuff defined below
std::string readStdin();
void print(const minjson::Value &value);
void printIssues(std::string_view input, const minjson::ParsingResult::Issues &issues);

int main(int argc, const char *argv[]) {
  if (argc <= 1) {
    const auto string = R"({
      "decimal": 3.14,
      "null": null,
      "boolean": true,
      "integer": 42,
      "string": "hello there",
      "array": [ 1, 2, 3 ],
      "object": {
        "nested number": 23,
        "nested string": "General Kenobi",
        "nested array": [ 4, 5, 6 ],
        "nested object": {
          "foo": "bar"
        }
      }
    })"sv;

    // parsing JSON
    const auto value = minjson::parse(string).value;

    std::cout << "parsed JSON:\n\n";
    print(value);
    std::cout << '\n';
    return 0;
  }

  std::string_view input;
  // read stdin if '-i' switch is provided in the command line
  std::string cin;
  if (argv[1] == "-i"sv) {
    cin = readStdin();
    input = cin;  // using input from stdin to parse
  } else {
    input = argv[1];  // using command line argument to parse
  }

  // parsing input JSON
  const auto [value, status, parsedSize, issues] = minjson::parse(input);
  // 'status' tells if the parsing succeeded
  // 'parsedSize' tells how many bytes are successfully parsed from the input
  // 'issues' contain information about different issues and errors encountered during parsing

  switch (status) {
  case minjson::ParsingResultStatus::Success:
    break;

  case minjson::ParsingResultStatus::PartialSuccess:
    std::cerr << "partial success";
    printIssues(input, issues);
    std::cerr << '\n';
    break;

  case minjson::ParsingResultStatus::Failure:
    std::cerr <<
#if defined(USE_TERMINAL_COLOR_SEQUENCES)
      "\x1b[91m"
#endif
      "failure"
#if defined(USE_TERMINAL_COLOR_SEQUENCES)
      "\x1B[0m"
#endif
      ;
    printIssues(input, issues);
    std::cerr << '\n';
    return 1;
  }
  std::cout << "parsed JSON:\n\n";
  print(value);
  std::cout << '\n';
}

// auxiliary stuff
std::string readStdin() {
  std::cin >> std::noskipws;
  return std::string{ std::istream_iterator<char>{ std::cin }, std::istream_iterator<char>{} };
}

void print(const minjson::Value &value) {
  minjson::SerializationOptions options;
  options.indent = 2;
  options.objectKeyValueSeparator = ": "sv;
  minjson::serializeToStream(std::cout, value, options);
}
size_t printChar(char c) {
  constexpr auto hexDigits = "0123456789ABCDEF"sv;
  if ('\0' <= c && c < '\x20') {
    std::cerr <<
#if defined(USE_TERMINAL_COLOR_SEQUENCES)
      "\x1b[4m"
#endif
      "\\x" << hexDigits[static_cast<uint8_t>(c) >> 4]
              << hexDigits[c & '\xf'];
#if defined(USE_TERMINAL_COLOR_SEQUENCES)
    std::cerr << "\x1B[0m";
#endif
    return 4;
  } else {
    std::cerr << c;
    return 1;
  }
}
size_t printString(std::string_view s) {
  size_t size = 0;
  for (char c : s) {
    size += printChar(c);
  }
  return size;
}
void printIssues(std::string_view input, const minjson::ParsingResult::Issues &issues) {
  if (issues.empty())
    return;

  std::cerr << "\n*** issues: ***";
  for (const auto &i : issues) {
    std::cerr << "\n  " << i.description;
    // also 'i.code' can be used to distinguish issues and errors
    if (i.offset != input.size())
      std::cerr << " @ " << i.offset;
    std::cerr << "\n    ";
    const size_t start = std::max(i.offset, size_t{ 9 }) - 9;
    if (start > 0)
      std::cerr << "...";
    size_t offset = printString(input.substr(start, i.offset - start));
    const size_t count = std::min(i.offset + 9, input.size()) - i.offset;
    if (i.offset != input.size()) {
#if defined(USE_TERMINAL_COLOR_SEQUENCES)
      std::cerr << "\x1b[91m";
#endif
      printChar(input[i.offset]);
#if defined(USE_TERMINAL_COLOR_SEQUENCES)
      std::cerr << "\x1B[0m";
#endif
      printString(input.substr(i.offset + 1, count - 1));
      if (i.offset + count < input.size())
        std::cerr << "...";
    }
    std::cerr << '\n';
    for (; offset; --offset)
      std::cerr << ' ';
    if (start > 0)
      std::cerr << "   ";
    std::cerr << "    ^";
  }
  std::cerr << "\n***************\n";
}
