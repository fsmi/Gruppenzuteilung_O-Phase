#include <fstream>
#include <iostream>

#include "boost/property_tree/json_parser.hpp"

#include "algorithms.h"
#include "moves_local_search.h"
#include "parse.h"

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
  assignWithMinimumNumberPerGroup(state, MIN_GROUP_SIZE);

  printNumberPerRating(state);

  // Problems: Lehramt + Dritti, Master + Lehramt, Mathe + Master

  std::cout << "Calculating reassignments to assert minimum number." << std::endl << std::endl;
  assertMinimumNumberPerGroupForSpecificType(state, {
    {is_info_ba, 5, "Info (BA)"},
    {is_math_ba, 5, "Mathe (BA)"},
    {is_lehramt_ba, 5, "Lehramt (BA)"},
    {is_ersti, 10, "Ersti"},
    {is_dritti, 5, "Dritti"},
    {is_master, 5, "Master"},
  });

  printNumberPerRating(state);

  PTree result = writeOutputToTree(state);
  boost::property_tree::json_parser::write_json(out_file, result);
  if (argc == 4) {
    writeOutputToFiles(state, argv[3], {
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
