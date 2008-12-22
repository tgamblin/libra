#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

namespace stringutils {

  /// Breaks a string into substrings using any characters in <delim> as delimiters.
  /// For example, to split a string by commas and whitespace, use ", " for delim.
  void split(const std::string& str, const std::string& delim, 
             std::vector<std::string>& parts);

} // namespace

#endif //STRING_UTILS_H
