#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <fstream>
#include <iostream>

inline
void show_coordinates_in_file(const std::string& filename,
                              std::size_t line_number,
                              std::size_t column_number) {
  std::ifstream file(filename.c_str(), std::ios::in);
  if (not file)
    throw std::string("could not open ") + filename;
  
  std::string line;
  while (line_number > 0) {
    std::getline(file, line);
    --line_number;
  }

  std::cout << line << std::endl;
  std::cout << std::string(column_number, ' ') << "^ here" << std::endl;
}

#endif /* FILE_UTILS_H */
