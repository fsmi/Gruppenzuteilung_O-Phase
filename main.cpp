#include <iostream>
#include <fstream>

#include "boost/property_tree/json_parser.hpp"

#include "algorithms.h"
#include "parse.h"

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: ./Main <input>" << std::endl;
    std::exit(-1);
  }
  std::ifstream file(argv[1]);
  PTree pt;
  boost::property_tree::json_parser::read_json(file, pt);
  Input input = parseInput(pt);
  State state(input);
  assignWithMinimumNumberPerGroup(state, 13);
  printCurrentAssignment(state);
}
