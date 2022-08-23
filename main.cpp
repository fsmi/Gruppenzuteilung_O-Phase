#include <fstream>
#include <iostream>

#include "boost/property_tree/json_parser.hpp"
#include "boost/program_options.hpp"

#include "algorithms.h"
#include "moves_local_search.h"
#include "parse.h"
#include "config.h"

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
  for (uint32_t i = 0; i < state.numGroups(); ++i) {
    std::cout << "Number of " << Rating(i).getName() << ": "
              << num_ratings.at(i) << std::endl;
  }
}

// parse command line arguments and (if provided) config file,
// using the boost program options library
void parseCmdAndConfig(int argc, const char *argv[], std::string& in_filename,
                       std::string& out_filename, std::string& groups_filename) {
  po::options_description config_options = Config::getConfigOptions();

  std::string config;
  po::options_description cmd_options("Primary Options");
  cmd_options.add_options()
          ("input,i",
            po::value<std::string>(&in_filename)->value_name("<string>")->required(),
            "Input filename")
          ("output,o",
            po::value<std::string>(&out_filename)->value_name("<string>")->required(),
            "Output filename")
          ("config,c",
            po::value<std::string>(&config)->value_name("<string>"),
            "Config filename")
          ("groups,g",
            po::value<std::string>(&groups_filename)->value_name("<string>"),
            "Groups filename: If present, creates for each group a "
            "file with all assigned students of the group")
          ("help", "show help message");
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
      std::cerr << "Error opening config file" << std::endl;
      std::exit(-1);
    }

    po::store(po::parse_config_file(config_file, config_options, true), cmd_vm);
    po::notify(cmd_vm);
  }
}

int main(int argc, const char *argv[]) {
  std::string in_filename, out_filename, groups_filename;
  parseCmdAndConfig(argc, argv, in_filename, out_filename, groups_filename);

  std::ifstream in_file(in_filename);
  if (!in_file) {
    std::cerr << "Error opening input file" << std::endl;
    std::exit(-1);
  }
  std::ofstream out_file(out_filename);
  if (!out_file) {
    std::cerr << "Error opening output file" << std::endl;
    std::exit(-1);
  }

  // Definitions for different types of students
  auto is_info_ba = [](const StudentData &data) {
    return data.course_type == CourseType::Info &&
           data.degree_type != DegreeType::Master;
  };
  auto is_math_ba = [](const StudentData &data) {
    return data.course_type == CourseType::Mathe &&
           data.degree_type != DegreeType::Master;
  };
  auto is_lehramt_ba = [](const StudentData &data) {
    return data.course_type == CourseType::Lehramt &&
           data.degree_type != DegreeType::Master;
  };
  auto is_ersti = [](const StudentData &data) {
    return data.semester == Semester::Ersti &&
           data.degree_type != DegreeType::Master;
  };
  auto is_dritti = [](const StudentData &data) {
    return data.semester == Semester::Dritti &&
           data.degree_type != DegreeType::Master;
  };
  auto is_master = [](const StudentData &data) {
    return data.degree_type == DegreeType::Master;
  };
  auto is_info_ersti = [=](const StudentData &data) {
    return is_info_ba(data) && is_ersti(data);
  };
  auto is_math_ersti = [=](const StudentData &data) {
    return is_math_ba(data) && is_ersti(data);
  };
  auto is_lehramt_ersti = [=](const StudentData &data) {
    return is_lehramt_ba(data) && is_ersti(data);
  };
  auto is_info_dritti = [=](const StudentData &data) {
    return is_info_ba(data) && is_dritti(data);
  };
  auto is_math_dritti = [=](const StudentData &data) {
    return is_math_ba(data) && is_dritti(data);
  };
  auto is_lehramt_dritti = [=](const StudentData &data) {
    return is_lehramt_ba(data) && is_dritti(data);
  };
  auto is_info_master = [=](const StudentData &data) {
    return data.course_type == CourseType::Info && is_master(data);
  };
  auto is_math_master = [=](const StudentData &data) {
    return data.course_type == CourseType::Mathe && is_master(data);
  };
  auto is_lehramt_master = [=](const StudentData &data) {
    return data.course_type == CourseType::Lehramt && is_master(data);
  };
  auto is_lehramt = [](const StudentData &data) {
    return data.course_type == CourseType::Lehramt;
  };

  // the main code
  PTree pt;
  boost::property_tree::json_parser::read_json(in_file, pt);
  Input input = parseInput(pt);
  std::cout << "Input file successfully parsed." << std::endl;
  std::cout << "Number of students: " << input.students.size() << std::endl;
  State state(input);
  assignWithMinimumNumberPerGroup(state, Config::get().min_group_size);

  printNumberPerRating(state);

  // Problems: Lehramt + Dritti, Master + Lehramt, Mathe + Master

  assertMinimumNumberPerGroupForSpecificType(state, {
    {is_info_ersti, 5, "Info (Ersti)"},
    {is_math_ersti, 5, "Mathe (Ersti)"},
    {is_lehramt_ba, 5, "Lehramt (BA)"},
    {is_ersti, 8, "Ersti"},
    {is_info_dritti, 4, "Info-Dritti"},
    {is_math_dritti, 3, "Mathe-Dritti"},
    {is_master, 5, "Master"},
  });

  printNumberPerRating(state);

  PTree result = writeOutputToTree(state);
  boost::property_tree::json_parser::write_json(out_file, result);
  if (groups_filename != "") {
    writeOutputToFiles(state, groups_filename, {
      {is_info_ersti, "Info-Ersti"},
      {is_math_ersti, "Mathe-Ersti"},
      {is_lehramt_ersti, "Lehramt-Ersti"},
      {is_info_dritti, "Info-Dritti"},
      {is_math_dritti, "Mathe-Dritti"},
      {is_info_master, "Info-Master"},
      {is_math_master, "Mathe-Master"},
      {is_lehramt, "Lehramt (gesamt)"},
    });
  }
}
