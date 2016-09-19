#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <vector>
#include <string>

class CSVParser {
public:
  // http://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c
  template<typename InputIterator>
  static auto parse(InputIterator begin, InputIterator end) {

    std::vector<std::vector<std::string>> result;

    bool inQuote = false;
    bool isNewLine = false;

    std::string field;
    std::vector<std::string> lineFields;

    while (begin != end) {
      char p = *begin++;

      switch (p) {
        case '"':
          isNewLine = false;
          inQuote = !inQuote;
          break;

        case ',':
          isNewLine = false;
          if (inQuote) {
            field += p;
          } else {
            lineFields.push_back(std::move(field));
            field.clear();
          }
          break;

        case '\n':
        case '\r':
          if (inQuote) {
            field += p;
          } else if (!isNewLine) {
            lineFields.push_back(std::move(field));
            result.push_back(std::move(lineFields));

            field.clear();
            lineFields.clear();
            isNewLine = true;
          }
          break;

        default:
          isNewLine = false;
          field += p;
          break;
      }
    }

    if (!field.empty())
      lineFields.push_back(std::move(field));

    if (!lineFields.empty())
      result.push_back(std::move(lineFields));

    return result;
  }
};

#endif // CSV_PARSER_H
