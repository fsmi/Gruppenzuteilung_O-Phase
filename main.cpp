#include <fstream>
#include <iostream>

#include "boost/property_tree/json_parser.hpp"

#include "algorithms.h"
#include "parse.h"

int main(int argc, const char *argv[]) {
  if (argc != 3 && argc != 4) {
    std::cout << "Usage: ./Main <input> <output> [<out_path>]" << std::endl;
    std::exit(-1);
  }
  std::ifstream in_file(argv[1]);
  if (!in_file) {
    std::cerr << "Error opening input file" << std::endl;
    std::exit(-1);
  }
  std::ofstream out_file(argv[2]);
  if (!out_file) {
    std::cerr << "Error opening output file" << std::endl;
    std::exit(-1);
  }
  PTree pt;
  boost::property_tree::json_parser::read_json(in_file, pt);
  Input input = parseInput(pt);
  std::cout << "Input file successfully parsed." << std::endl;
  State state(input);
  assignWithMinimumNumberPerGroup(state, MIN_GROUP_SIZE);
  PTree result = writeOutputToTree(state);
  boost::property_tree::json_parser::write_json(out_file, result);
  if (argc == 4) {
    writeOutputToFiles(state, argv[3]);
  }
}
