#include <fstream>
#include <iostream>
#include <chrono>
#include <ios>
#include <csignal>

#include "boost/property_tree/json_parser.hpp"
#include "boost/program_options.hpp"

#include "algorithms.h"
#include "io.h"
#include "config.h"
#include "student_types.h"

namespace po = boost::program_options;

template<typename T, typename F>
void printTabularLine(const std::vector<T>& data, const std::string& head, F f) {
  std::cout << std::left << TRACE_START << std::setw(7) << head;
  for (const auto& val: data) {
    std::string word = f(val);
    const int width = std::max<int>(15, word.size());
    int left = (width - word.size()) / 2 + word.size();
    int right = (width - word.size() + 1) / 2;
    std::cout << std::right << " " << std::setw(left) << f(val) << std::setw(right) << "";
  }
  std::cout << std::endl;
}

// print number of ratings for different rating levels
void printNumberPerRating(const State& state, const std::vector<std::pair<Filter, StudentID>>& filters) {
  // first, collect stats
  std::vector<std::tuple<std::vector<int>, int, std::string>> ratings_per_filter;
  size_t required_ratings = 0;
  for (size_t i = 0; i <= filters.size(); ++i) {
    StudentID total = 0;
    std::vector<int> num_ratings(state.numGroups(), 0);
    const Filter& filter = (i == 0) ? Filter({}, "Total") : filters[i - 1].first;

    for (ParticipantID part = 0; part < state.numParticipants(); ++part) {
      Rating r = state.rating(part).at(state.assignment(part));
      auto count_if_applies = [&] (const StudentData& data) {
        if (filter.apply(data)) {
          ++num_ratings.at(r.index);
          ++total;
        }
      };
      if (state.isTeam(part)) {
        for (StudentID member: state.teamData(part).members) {
          count_if_applies(state.data().students[member]);
        }
      } else {
        count_if_applies(state.studentData(part));
      }
    }

    if (i == 0) {
      while (!num_ratings.empty() && num_ratings.back() == 0) {
        num_ratings.pop_back();
      }
      required_ratings = num_ratings.size();
    } else {
      num_ratings.resize(required_ratings);
    }
    ratings_per_filter.emplace_back(std::move(num_ratings), total, filter.name);
  }

  // second, print into tabular
  printTabularLine(ratings_per_filter, "Rating",
                   [](const auto& val) { return std::get<2>(val); });
  bool skip = false;
  for (size_t i = 0; i < required_ratings; ++i) {
    bool empty = (std::get<0>(ratings_per_filter[0]).at(i) == 0);
    if (empty && !skip) {
      printTabularLine(ratings_per_filter, "..", [i](const auto&) { return ""; });
      skip = true;
    } else if (!empty) {
      printTabularLine(ratings_per_filter, Rating(i).getName(), [i](const auto& val) {
        int count = std::get<0>(val).at(i);
        return (count == 0) ? "" : std::to_string(count);
      });
      skip = false;
    }
  }
  printTabularLine(ratings_per_filter, "Sum",
                   [](const auto& val) { return std::to_string(std::get<1>(val)); });
}

void printStudentsPerGroup(const State& state) {
  LOG(TRACE_START, 1);
  for (GroupID group = 0; group < state.numGroups(); ++group) {
    LOG(TRACE_START << std::right << std::setw(3) << state.groupSize(group) << " /"
        << std::setw(3) << state.groupData(group).capacity << "   " << state.groupData(group).name, 1);
  }
}

// first value is only with type specifiic assignment, second value is total
std::vector<std::pair<StudentID, StudentID>>
groupSizesForType(const State &s, const Filter& filter) {
  std::vector<std::pair<GroupID, StudentID>> result;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    StudentID num = 0;
    StudentID type_specific = 0;
    for (const auto &pair : s.groupAssignmentList(group)) {
      const StudentData& data = s.data().students[pair.first];
      if (filter.apply(data)) {
        num++;
        if (data.type_specific_assignment) {
          type_specific++;
        }
      }
    }
    if (num > 0) {
      result.emplace_back(type_specific, num);
    }
  }
  std::sort(result.begin(), result.end(), [&](const auto &p1, const auto &p2) {
    return p1.first < p2.first || (p1.first == p2.first && p1.second < p2.second);
  });
  return result;
}

void printGroupSizes(const State& state, const std::vector<std::pair<Filter, StudentID>>& filters) {
  LOG(TRACE_START, 1);
  LOG(TRACE_START << "# Group Sizes #  type_specific[total]", 1);
  for (const auto& [filter, _]: filters) {
    auto group_sizes = groupSizesForType(state, filter);
    std::cout << std::left << TRACE_START << std::setw(17) << filter.name;
    for (size_t i = 0; i < group_sizes.size(); ++i) {
      auto [ts, total] = group_sizes[i];
      if (i > 0) {
        std::cout << ", ";
      }
      std::cout << ts << "[" << total << "]";
    }
    std::cout << std::endl;
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

  std::vector<std::pair<Filter, StudentID>> type_filters;
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

  StudentID n_disabled = 0;
  for (const StudentData& s: input.students){
    if (!s.type_specific_assignment) {
      n_disabled++;
    }
  }
  INFO("Number of students: " << input.students.size() << "     "
       << "Type specific assignment disabled: " << n_disabled, true);

  // register signal handler
  signal(SIGINT, signalHandler);

  State state(input);
  assignWithMinimumNumberPerGroup(state, Config::get().min_group_size);

  if (Config::get().verbosity_level >= 3) {
    printNumberPerRating(state, type_filters);
    printStudentsPerGroup(state);
  }

  assertMinimumNumberPerGroupForSpecificType(state, type_filters);

  if (Config::get().verbosity_level >= 1) {
    printNumberPerRating(state, type_filters);
  }
  if (Config::get().verbosity_level >= 2) {
    printStudentsPerGroup(state);
    printGroupSizes(state, type_filters);
  }

  PTree result = writeOutputToTree(state);
  boost::property_tree::json_parser::write_json(out_file, result);

  const double total_time = std::chrono::duration<double>(std::chrono::system_clock::now() - timer_start).count();
  INFO("Total time required: " <<  total_time << " s", false);
  INFO("Output written to: " << out_filename, true);

  if (groups_filename != "") {
    writeOutputToFiles(state, groups_filename, type_filters);
  }
}
