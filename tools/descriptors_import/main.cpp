#include <fstream>

#include "ImportService.h"
#include "CSVParser.h"

int main(int argc, char* argv[]) {

   if (argc < 3) {
      // missing required arguments
      return 1;
   }

   std::string dbPath = argv[1];
   const char *csvFilePath = argv[2];

   std::ifstream csvFile{ csvFilePath };
   if (!csvFile.good()) {
      // invalid csv file
      return 2;
   }

   MatchTree bktree{ MatchTree::element_type::New(dbPath, dbPath + "_i") };
   if (!bktree) {
      // connection failed
      return 3;
   }

   auto entries = CSVParser::parse(std::istreambuf_iterator<char>{ csvFile }, std::istreambuf_iterator<char>{});

   ImportService service{ bktree };

   for (const auto& pair : entries) {
      service.put(pair[0], pair[1]);
   }

   return 0;
}
