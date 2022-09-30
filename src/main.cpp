#include <fstream>
#include <iostream>
#include <chrono>

#include "boost/property_tree/json_parser.hpp"
#include "boost/program_options.hpp"

#include "algorithms.h"
#include "io.h"
#include "config.h"
#include "student_types.h"

namespace po = boost::program_options;

// print number of ratings for different rating levels
void printNumberPerRating(const State& state) {
  std::vector<int> num_ratings(state.numGroups(), 0);
  for (ParticipantID part = 0; part < state.numParticipants(); ++part) {
    Rating r = state.rating(part).at(state.assignment(part));
    int num = 1;
    if (state.isTeam(part)) {
      num = state.teamData(part).size();
    }
    num_ratings.at(r.index) += num;
  }
  while (!num_ratings.empty() && num_ratings.back() == 0) {
    num_ratings.pop_back();
  }
  LOG(TRACE_START << "### Resulting assignment ###", 1);
  for (size_t i = 0; i < num_ratings.size(); ++i) {
    LOG(TRACE_START << "Participants with rating "
                    << Rating(i).getName() << ": " << num_ratings.at(i), 1);
  }
}

void printStudentsPerGroup(const State& state) {
  for (GroupID group = 0; group < state.numGroups(); ++group) {
    LOG(TRACE_START << state.groupSize(group) << "/" << state.groupData(group).capacity
                    << " - " << state.groupData(group).name, 1);
  }
}

// parse command line arguments and (if provided) config file,
// using the boost program options library
void parseCmdAndConfig(int argc, const char *argv[], std::string& in_filename,
                       std::string& out_filename, std::string& groups_filename, std::string& types_filename) {
  po::options_description config_options = Config::getConfigOptions();

  std::string config;
  po::options_description cmd_options("Primary Options", getTerminalWidth());
  cmd_options.add_options()
          ("help", "show help message")
          ("input,i",
            po::value<std::string>(&in_filename)->value_name("<string>")->required(),
            "Input filename (required)")
          ("output,o",
            po::value<std::string>(&out_filename)->value_name("<string>")->required(),
            "Output filename (required)")
          ("config,c",
            po::value<std::string>(&config)->value_name("<string>"),
            "Config filename")
          ("types,t",
            po::value<std::string>(&types_filename)->value_name("<string>"),
            "Types filename")
          ("groups,g",
            po::value<std::string>(&groups_filename)->value_name("<string>"),
            "Groups directory: If present, creates for each group a file "
            "with all assigned students of the group in the specified directory");
  cmd_options.add(config_options);

  po::variables_map cmd_vm;
  po::store(po::parse_command_line(argc, argv, cmd_options), cmd_vm);

  if (cmd_vm.count("help") != 0 || argc == 1) {
    std::cout << cmd_options << std::endl;
    exit(0);
  }
  po::notify(cmd_vm);

  if ( config != "" ) {
    std::ifstream config_file(config.c_str());
    if (!config_file) {
      FATAL_ERROR("Error opening config file");
    }

    po::store(po::parse_config_file(config_file, config_options, true), cmd_vm);
    po::notify(cmd_vm);
  }
  Config::check();
}

int main(int argc, const char *argv[]) {
  std::chrono::time_point<std::chrono::system_clock> timer_start = std::chrono::system_clock::now();

  std::string in_filename, out_filename, groups_filename, types_filename;
  parseCmdAndConfig(argc, argv, in_filename, out_filename, groups_filename, types_filename);
  TRACE("Reading arguments and config completed.", true);

  std::ifstream in_file(in_filename);
  if (!in_file) {
    FATAL_ERROR("Error opening input file");
  }
  std::ofstream out_file(out_filename);
  if (!out_file) {
    FATAL_ERROR("Error opening output file");
  }

  std::vector<std::tuple<FilterFn, StudentID, std::string>> type_filters;
  if (types_filename != "") {
    std::ifstream types_file(types_filename);
    if (!types_file) {
      FATAL_ERROR("Error opening types file");
    }
    type_filters = parseTypesFile(types_file);
    PROGRESS("Types file successfully parsed.", true);
  }

  // the main code
  PTree pt;
  boost::property_tree::json_parser::read_json(in_file, pt);
  Input input = parseInput(pt);
  PROGRESS("Input file successfully parsed.", true);
  INFO("Number of students: " << input.students.size(), true);

  State state(input);
  assignWithMinimumNumberPerGroup(state, Config::get().min_group_size);

  if (Config::get().verbosity_level >= 3) {
    printNumberPerRating(state);
    printStudentsPerGroup(state);
  }

  // Problems: Lehramt + Dritti, Master + Lehramt, Mathe + Master

  assertMinimumNumberPerGroupForSpecificType(state, type_filters);

  const double total_time = std::chrono::duration<double>(std::chrono::system_clock::now() - timer_start).count();
  INFO("Total time required: " <<  total_time << " s", true);

  if (Config::get().verbosity_level >= 1) {
    printNumberPerRating(state);
  }
  if (Config::get().verbosity_level >= 2) {
    printStudentsPerGroup(state);
  }

  PTree result = writeOutputToTree(state);
  boost::property_tree::json_parser::write_json(out_file, result);
  if (groups_filename != "") {
    writeOutputToFiles(state, groups_filename, type_filters);
  }
}
