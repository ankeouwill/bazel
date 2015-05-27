// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "src/main/cpp/util/strings.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <cassert>

#include "src/main/cpp/util/exit_code.h"

using std::vector;

namespace blaze_util {

static const char kSeparator[] = " \n\t\r";

// # Table generated by this Python code (bit 0x02 is currently unused):
// def Hex2(n):
//   return '0x' + hex(n/16)[2:] + hex(n%16)[2:]
// def IsPunct(ch):
//   return (ord(ch) >= 32 and ord(ch) < 127 and
//           not ch.isspace() and not ch.isalnum())
// def IsBlank(ch):
//   return ch in ' \t'
// def IsCntrl(ch):
//   return ord(ch) < 32 or ord(ch) == 127
// def IsXDigit(ch):
//   return ch.isdigit() or ch.lower() in 'abcdef'
// for i in range(128):
//   ch = chr(i)
//   mask = ((ch.isalpha() and 0x01 or 0) |
//           (ch.isalnum() and 0x04 or 0) |
//           (ch.isspace() and 0x08 or 0) |
//           (IsPunct(ch) and 0x10 or 0) |
//           (IsBlank(ch) and 0x20 or 0) |
//           (IsCntrl(ch) and 0x40 or 0) |
//           (IsXDigit(ch) and 0x80 or 0))
//   print Hex2(mask) + ',',
//   if i % 16 == 7:
//     print ' //', Hex2(i & 0x78)
//   elif i % 16 == 15:
//     print
const unsigned char kAsciiPropertyBits[256] = {
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  // 0x00
  0x40, 0x68, 0x48, 0x48, 0x48, 0x48, 0x40, 0x40,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  // 0x10
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x28, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,  // 0x20
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84,  // 0x30
  0x84, 0x84, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x05,  // 0x40
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,  // 0x50
  0x05, 0x05, 0x05, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x05,  // 0x60
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,  // 0x70
  0x05, 0x05, 0x05, 0x10, 0x10, 0x10, 0x10, 0x40,
};


bool starts_with(const string &haystack, const string &needle) {
  return (haystack.length() >= needle.length()) &&
      (memcmp(haystack.c_str(), needle.c_str(), needle.length()) == 0);
}

bool ends_with(const string &haystack, const string &needle) {
  return ((haystack.length() >= needle.length()) &&
          (memcmp(haystack.c_str() + (haystack.length()-needle.length()),
                  needle.c_str(), needle.length()) == 0));
}

void JoinStrings(
    const vector<string> &pieces, const char delimeter, string *output) {
  bool first = true;
  for (const auto &piece : pieces) {
    if (first) {
      *output = piece;
      first = false;
    } else {
      *output += delimeter + piece;
    }
  }
}

vector<string> Split(const string &contents, const char delimeter) {
  vector<string> result;
  SplitStringUsing(contents, delimeter, &result);
  return result;
}

void SplitStringUsing(
    const string &contents, const char delimeter, vector<string> *result) {
  assert(result);

  size_t start = 0;
  while (start < contents.length() && contents[start] == delimeter) {
    ++start;
  }

  size_t newline = contents.find(delimeter, start);
  while (newline != string::npos) {
    result->push_back(string(contents, start, newline - start));
    start = newline;
    while (start < contents.length() && contents[start] == delimeter) {
      ++start;
    }
    newline = contents.find(delimeter, start);
  }

  // If there is a trailing line, add that.
  if (start != newline && start != contents.size()) {
    result->push_back(string(contents, start));
  }
}

void SplitQuotedStringUsing(const string &contents, const char delimeter,
                            std::vector<string> *output) {
  size_t len = contents.length();
  size_t start = 0;
  size_t quote = string::npos;  // quote position

  for (size_t pos = 0; pos < len; ++pos) {
    if (start == pos && contents[start] == delimeter) {
      ++start;
    } else if (contents[pos] == '\\') {
      ++pos;
    } else if (quote != string::npos && contents[pos] == contents[quote]) {
      quote = string::npos;
    } else if (quote == string::npos &&
               (contents[pos] == '"' || contents[pos] == '\'')) {
      quote = pos;
    } else if (quote == string::npos && contents[pos] == delimeter) {
      output->push_back(string(contents, start, pos - start));
      start = pos + 1;
    }
  }

  // A trailing element
  if (start < len) {
    output->push_back(string(contents, start));
  }
}

void Replace(const string &oldsub, const string &newsub, string *str) {
  size_t start = 0;
  // This is O(n^2) (the complexity of erase() is actually unspecified, but
  // usually linear).
  while ((start = str->find(oldsub, start)) != string::npos) {
    str->erase(start, oldsub.length());
    str->insert(start, newsub);
    start += newsub.length();
  }
}

void StripWhitespace(string *str) {
  int str_length = str->length();

  // Strip off leading whitespace.
  int first = 0;
  while (first < str_length && ascii_isspace(str->at(first))) {
    ++first;
  }
  // If entire string is white space.
  if (first == str_length) {
    str->clear();
    return;
  }
  if (first > 0) {
    str->erase(0, first);
    str_length -= first;
  }

  // Strip off trailing whitespace.
  int last = str_length - 1;
  while (last >= 0 && ascii_isspace(str->at(last))) {
    --last;
  }
  if (last != (str_length - 1) && last >= 0) {
    str->erase(last + 1, string::npos);
  }
}

static void GetNextToken(const string &str, const char &comment,
                  string::const_iterator *iter, vector<string> *words) {
  string output;
  auto last = *iter;
  char quote = '\0';
  // While not a delimiter.
  while (last != str.end() && (quote || strchr(kSeparator, *last) == nullptr)) {
    // Absorb escapes.
    if (*last == '\\') {
      ++last;
      if (last == str.end()) {
        break;
      }
      output += *last++;
      continue;
    }

    if (quote) {
      if (*last == quote) {
        // Absorb closing quote.
        quote = '\0';
        ++last;
      } else {
        output += *last++;
      }
    } else {
      if (*last == comment) {
        last = str.end();
        break;
      }
      if (*last == '\'' || *last == '"') {
        // Absorb opening quote.
        quote = *last++;
      } else {
        output += *last++;
      }
    }
  }

  if (!output.empty()) {
    words->push_back(output);
  }

  *iter = last;
}

void Tokenize(const string &str, const char &comment, vector<string> *words) {
  assert(words);
  words->clear();

  string::const_iterator i = str.begin();
  while (i != str.end()) {
    // Skip whitespace.
    while (i != str.end() && strchr(kSeparator, *i) != nullptr) {
      i++;
    }
    if (i != str.end() && *i == comment) {
      break;
    }
    GetNextToken(str, comment, &i, words);
  }
}


// Evaluate a format string and store the result in 'str'.
void StringPrintf(string *str, const char *format, ...) {
  assert(str);

  // Determine the required buffer size. vsnpritnf won't account for the
  // terminating '\0'.
  va_list args;
  va_start(args, format);
  int output_size = vsnprintf(nullptr, 0, format, args);
  if (output_size < 0) {
    fprintf(stderr, "Fatal error formatting string: %d", output_size);
    exit(blaze_exit_code::INTERNAL_ERROR);
  }
  va_end(args);

  // Allocate a buffer and format the input.
  int buffer_size = output_size + sizeof '\0';
  char *buf = new char[buffer_size];
  va_start(args, format);
  int print_result = vsnprintf(buf, buffer_size, format, args);
  if (print_result < 0) {
    fprintf(stderr, "Fatal error formatting string: %d", print_result);
    exit(blaze_exit_code::INTERNAL_ERROR);
  }
  va_end(args);

  *str = buf;
  delete[] buf;
}

void ToLower(string *str) {
  assert(str);
  if (str->empty()) {
    return;
  }

  string temp = "";
  for (auto ch : *str) {
    temp += tolower(ch);
  }
  *str = temp;
}

}  // namespace blaze_util
