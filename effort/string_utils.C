#include "string_utils.h"
using namespace std;

namespace stringutils {
  void split(const string& str, const string& delim, vector<string>& parts) {
    size_t start, end = 0;
    
    while (end < str.size()) {
      start = end;
      while (start < str.size() && (delim.find(str[start]) != string::npos))
        start++;  // skip initial whitespace
      
      end = start;
      while (end < str.size() && (delim.find(str[end]) == string::npos))
        end++; // skip to end of word
      
      if (end-start != 0) {  // just ignore zero-length strings.
        parts.push_back(string(str, start, end-start));
      }
    }
  }

}  // namespace

