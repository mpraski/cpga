//
// Created by marcinpraski on 10/01/19.
//

#ifndef GENETIC_ACTOR_CSV_READER_H
#define GENETIC_ACTOR_CSV_READER_H

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

namespace cpga {
namespace utilities {
class csv_reader {
 public:
  static auto read_double(const std::string &file, size_t rows, size_t cols, char delim = ',') {
    std::vector<std::vector<double>> data(rows, std::vector<double>(cols));
    std::ifstream ifs(file);

    if (!ifs.is_open()) {
      throw std::runtime_error("Cannot open CSV file: " + file);
    }

    std::string s;
    for (size_t i = 0; i < rows; ++i) {
      if (!std::getline(ifs, s)) {
        throw std::runtime_error("No more rows to read");
      }
      std::istringstream iss(s);
      std::string token;
      for (size_t j = 0; j < cols; ++j) {
        if (!std::getline(iss, token, delim)) {
          throw std::runtime_error("No more columns to read");
        }
        data[i][j] = std::stod(token);
      }
    }

    return data;
  }
};
}
}

#endif //GENETIC_ACTOR_CSV_READER_H
