#include <fstream>
#include <iostream>

#include "boost/property_tree/json_parser.hpp"

#include "algorithms.h"
#include "parse.h"

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: ./Main <input>" << std::endl;
    std::exit(-1);
  }
  std::ifstream file(argv[1]);
  if (!file) {
    std::cerr << "Error opening file" << std::endl;
    std::exit(-1);
  }
  PTree pt;
  boost::property_tree::json_parser::read_json(file, pt);
  Input input = parseInput(pt);
  std::cout << "Input file successfully parsed." << std::endl;
  State state(input);
  assignWithMinimumNumberPerGroup(state, MIN_GROUP_SIZE);
  printCurrentAssignment(state);
}
