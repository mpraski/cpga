//
// Created by marcinpraski on 10/01/19.
//

#ifndef GENETIC_ACTOR_CSV_READER_H
#define GENETIC_ACTOR_CSV_READER_H

#include <vector>
#include <fstream>

template<typename Data>
class csv_reader {
 public:
  static std::vector<std::vector<Data>> read(const std::string &file, size_t rows, size_t cols, char delim = ',') {
    std::vector<std::vector<Data>> data(rows, std::vector<Data>(cols));
    std::ifstream ifs(file);

    if (!ifs.is_open()) {
      throw std::runtime_error("Cannot open CSV file: " + file);
    }

    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        ifs >> data[i][j];
        ifs.get();
      }
    }
  }
};

#endif //GENETIC_ACTOR_CSV_READER_H
