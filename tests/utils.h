#include <minjsoncpp.h>

template<typename A>
std::ostream &operator<<(std::ostream &s, const minjson::BasicValue<A> &v) {
  static constexpr auto options = std::invoke([] {
    minjson::SerializationOptions o;
    o.objectKeyValueSeparator = ": ";
    o.indent = 2;
    o.escape = minjson::Escape::NonAscii;
    o.sortObjectKeys = true;
    return o;
  });
  minjson::serializeToStream(s, v, options);
  return s;
}

inline std::ostream &operator<<(std::ostream &s, minjson::ParsingResultStatus status) {
  switch (status) {
  case minjson::ParsingResultStatus::Failure:
    s << "failure";
    break;
  case minjson::ParsingResultStatus::Success:
    s << "success";
    break;
  case minjson::ParsingResultStatus::PartialSuccess:
    s << "partial success";
    break;
  default:
    s << "<unknown status>";
  }
  return s;
}


struct PrintIssues {
  const minjson::ParsingResult::Issues &issues;
  friend std::ostream &operator<<(std::ostream &s, PrintIssues i) {
    s << "issues:";
    for (const auto &issue : i.issues)
      s << "\n  " << issue.description << " @ " << issue.offset;
    return s;
  }
};

inline bool isPrintableAsciiChar(char c) {
  return '\x20' <= c && c < '\x7F';
}
inline void writeCharHex(std::ostream &s, char c) {
  constexpr char hexDigits[] = "0123456789ABCDEF";
  s << "\\x" << hexDigits[static_cast<uint8_t>(c) >> 4] << hexDigits[c & 0xf];
}

struct PrintCharHex {
  char c;

  friend std::ostream &operator<<(std::ostream &s, PrintCharHex c) {
    writeCharHex(s, c.c);
    if (isPrintableAsciiChar(c.c))
      s << " \'" << c.c << '\'';
    return s;
  }
};

struct NonPrintStr {
  std::string_view str;

  friend bool operator==(const NonPrintStr &a, const NonPrintStr &b) {
    return a.str == b.str;
  }
  friend bool operator!=(const NonPrintStr &a, const NonPrintStr &b) {
    return a.str != b.str;
  }

  friend std::ostream &operator<<(std::ostream &s, const NonPrintStr &str) {
    s << '\"';
    for (char c : str.str)
      if (isPrintableAsciiChar(c))
        s << c;
      else
        writeCharHex(s, c);
    s << '\"';
    return s;
  }
};

template<typename T1, typename T2>
std::enable_if_t<std::is_convertible_v<T1 &&, std::string_view> && std::is_convertible_v<T1 &&, std::string_view>,
                 std::string>
operator+(T1 &&s1, T2 &&s2) {
  std::string s;
  s.reserve(std::size(s1) + std::size(s2));
  s += s1;
  s += s2;
  return s;
}

inline std::string getQuoted(std::string_view s) {
  std::string q;
  q.reserve(s.size() + 2);
  q += '\"';
  q += s;
  q += '\"';
  return q;
}
