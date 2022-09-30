#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "io.h"
#include "config.h"

template <typename T, typename F>
std::vector<T> parseList(const PTree &tree, F fn) {
  std::vector<T> result;
  for (auto &element : tree) {
    result.push_back(fn(element));
  }
  return result;
}

template <typename T>
std::unordered_map<std::string, size_t> createMapping(const std::vector<T> &list) {
  std::unordered_map<std::string, size_t> result;
  for (size_t i = 0; i < list.size(); ++i) {
    result.insert({list[i].id, i});
  }
  return result;
}

CourseType parseCourseType(const std::string& name) {
  if (name == "info") {
    return CourseType::Info;
  } else if (name == "mathe") {
    return CourseType::Mathe;
  } else if (name == "lehramt") {
    return CourseType::Lehramt;
  } else if (name == "any") {
    return CourseType::Any;
  } else {
    FATAL_ERROR("Invalid course type: " << name);
  }
}

DegreeType parseDegreeType(const std::string& name) {
  if (name == "bachelor") {
    return DegreeType::Bachelor;
  } else if (name == "master") {
    return DegreeType::Master;
  } else if (name == "any") {
    return DegreeType::Any;
  } else {
    FATAL_ERROR("Invalid degree type: " << name);
  }
}

Semester parseSemester(const std::string& name) {
  if (name == "ersti") {
    return Semester::Ersti;
  } else if (name == "dritti") {
    return Semester::Dritti;
  } else {
    FATAL_ERROR("Invalid semseter: " << name);
  }
}

TeamData parseTeam(const std::string team_id, const PTree &tree, const std::unordered_map<std::string, size_t>& student_id_to_index,
                   std::unordered_map<std::string, std::string>& student_id_to_team_id) {
  std::vector<StudentID> members =
    parseList<StudentID>(tree, [&](const auto &t) {
      std::string id = t.second.PTree::get_value<std::string>();
      ASSERT_WITH(student_id_to_index.find(id) != student_id_to_index.end(), "Invalid student id in team: " << id);
      ASSERT_WITH(student_id_to_team_id.find(id) == student_id_to_team_id.end(),
                  std::string("Student contained in more than one team: ") << id);
      student_id_to_team_id[id] = team_id;
      return student_id_to_index.at(t.second.PTree::get_value<std::string>());
    });
  return TeamData(team_id, members);
}

std::vector<Rating> parseRatings(const PTree &tree, const std::unordered_map<std::string, size_t>& group_id_to_index, size_t num_groups) {
  std::vector<Rating> result(num_groups);
  auto get_index = [&](const std::string& group_id) {
    ASSERT_WITH(group_id_to_index.find(group_id) != group_id_to_index.end(),
                "Invalid group id in rating: " << group_id << " - Is --rating-input-type correctly specified?");
    return group_id_to_index.at(group_id);
  };

  if (Config::get().rating_input_type == RatingInputType::OrderedList) {
    std::vector<std::string> group_order = parseList<std::string>(tree,
      [&](const auto &t) {
        return t.second.PTree::get_value<std::string>();
      });
    for (size_t i = 0; i < group_order.size(); ++i) {
      size_t group_index = get_index(group_order[i]);
      result[group_index] = Rating(i);
    }
  } else {
    ASSERT(Config::get().rating_input_type == RatingInputType::Mapping);
    for(const auto& element: tree) {
      size_t group_index = get_index(element.first);
      result[group_index] = Rating(element.second.PTree::get_value<size_t>());
    }
  }
  return result;
}

Input parseInput(const PTree &tree) {
  Input input;
  auto parseGroup = [](auto element){
    std::string id = element.first;
    PTree tree = element.second;
    std::string name = tree.get<std::string>("name");
    StudentID capacity = tree.get<StudentID>("capacity");
    CourseType course_type = parseCourseType(tree.get<std::string>("course_type", "any"));
    DegreeType degree_type = parseDegreeType(tree.get<std::string>("degree_type", "any"));
    return GroupData(id, name, capacity, course_type, degree_type);
  };
  auto parseStudent = [](auto element){
    std::string id = element.first;
    PTree tree = element.second;
    std::string name = tree.get<std::string>("name");
    CourseType course_type = parseCourseType(tree.get<std::string>("course_type"));
    DegreeType degree_type = parseDegreeType(tree.get<std::string>("degree_type"));
    Semester semester = parseSemester(tree.get<std::string>("semester", "ersti"));
    bool type_specific_assignment = tree.get<bool>("type_specific_assignment", true);
    return StudentData(id, name, course_type, degree_type, semester, type_specific_assignment);
  };
  input.groups = parseList<GroupData>(tree.find("groups")->second, parseGroup);
  auto group_mapping = createMapping(input.groups);
  input.students = parseList<StudentData>(tree.find("students")->second, parseStudent);
  auto student_mapping = createMapping(input.students);
  input.teams = parseList<TeamData>(tree.find("teams")->second,
    [&](const auto &t) {
      return parseTeam(t.first, t.second, student_mapping, input.student_id_to_team_id);
    });
  auto team_mapping = createMapping(input.teams);
  std::vector<std::vector<Rating>> ratings(input.students.size());
  for (auto &element : tree.find("ratings")->second) {
    auto rating_list = parseRatings(element.second, group_mapping, input.groups.size());
    if (Config::get().input_per_team) {
      ASSERT_WITH(team_mapping.find(element.first) != team_mapping.end(),
                  "Team \"" << element.first << "\" not found. Is this a student id? "
                  "If so, you probably want to use --input-per-team=false");
      const TeamData& team = input.teams[team_mapping.at(element.first)];
      for (StudentID student: team.members) {
        ratings[student] = rating_list;
      }
    } else {
      ASSERT_WITH(student_mapping.find(element.first) != student_mapping.end(),
                  "Student \"" << element.first << "\" not found. Is this a team id? "
                  "If so, you probably want to use --input-per-team=true");
      ratings[student_mapping.at(element.first)] = rating_list;
    }
  }
  input.ratings = std::move(ratings);
  return input;
}

PTree writeOutputToTree(const State &s) {
  PTree root;
  std::unordered_set<std::string> considered_students;
  for (StudentID participant = 0; participant < s.numParticipants(); ++participant) {
    std::string group_id = s.groupData(s.getAssignment(participant)).id;
    if (Config::get().output_per_team) {
      std::string team_id;
      if (s.isTeam(participant)) {
        team_id = s.teamData(participant).id;
      } else {
        const std::string student_id = s.studentData(participant).id;
        team_id = s.data().student_id_to_team_id.at(student_id);
      }
      root.put<std::string>(team_id, group_id);
    } else {
      if (s.isTeam(participant)) {
        for (StudentID member : s.teamData(participant).members) {
          const std::string student_id = s.data().students[member].id;
          root.put<std::string>(student_id, group_id);
          considered_students.insert(student_id);
        }
      } else {
        const std::string student_id = s.studentData(participant).id;
        root.put<std::string>(student_id, group_id);
        considered_students.insert(student_id);
      }
    }
  }
  if (!Config::get().output_per_team && considered_students.size() != s.data().students.size()) {
    WARNING("Output data points (" << considered_students.size()
            << ") don't match the number of students (" << s.data().students.size() << ")!", true);
  }
  return root;
}

void outputStudentDataToFile(const StudentData& data,  const std::string &rating, std::ofstream& file) {
  std::string course;
  switch (data.course_type) {
  case CourseType::Info:
    course = "Info";
    break;
  case CourseType::Mathe:
    course = "Mathe";
    break;
  case CourseType::Lehramt:
    course = "Lehramt";
    break;
  default:
    course = "-";
  }
  std::string degree;
  switch (data.degree_type) {
  case DegreeType::Bachelor:
    degree = "Bachelor";
    break;
  case DegreeType::Master:
    degree = "Master";
    break;
  default:
    degree = "-";
  }
  std::string semester;
  switch (data.semester) {
  case Semester::Ersti:
    semester = "Ersti";
    break;
  case Semester::Dritti:
    semester = "Dritti";
    break;
  default:
    semester = "-";
  }
  file << data.name << ", " << data.id << ", " << course << ", " << degree << ", " << semester
        << ", [" << rating << "]" << std::endl;
}

// Writes the output in (more) human-readable form to the specified path
void writeOutputToFiles(const State &s, std::string path,
    const std::vector<std::pair<Filter, StudentID>> &filters) {
  std::string removed_path = path + "/RemovedGroups";
  std::ofstream removed(removed_path);
  std::string stats_path = path + "/Stats.csv";
  std::ofstream stats(stats_path);

  // csv header
  std::string header = "Name, Size, ";
  for (size_t i = 0; i < filters.size(); ++i) {
    header += std::get<1>(filters[i]) + (i + 1 == filters.size() ? "" : ", ");
  }
  stats << header << std::endl;

  StudentID sum = 0;
  std::vector<StudentID> filter_sums(filters.size());

  // group data
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    const std::string group_name_id = s.groupData(group).name + "-" + s.groupData(group).id.substr(0, 5);
    const std::string group_path = path + "/" + group_name_id;
    if (s.groupAssignmentList(group).empty()) {
      removed << s.groupData(group).name << " (" << s.groupData(group).id << ")" << std::endl;
    } else {
      std::ofstream file(group_path);
      std::vector<size_t> num_per_type(filters.size());
      for (const auto &pair : s.groupAssignmentList(group)) {
        StudentData data = s.data().students[pair.first];
        const std::string &rating = s.rating(pair.second)[group].getName();
        outputStudentDataToFile(data, rating, file);
        for (size_t i = 0; i < filters.size(); ++i) {
          if (filters[i].first.apply(data)) {
            ++num_per_type[i];
          }
        }
      }
      std::string group_stats = group_name_id + ", " + std::to_string(s.groupSize(group)) + ", ";
      sum += s.groupSize(group);
      for (size_t i = 0; i < filters.size(); ++i) {
        group_stats += std::to_string(num_per_type[i]) + (i + 1 == filters.size() ? "" : ", ");
        filter_sums[i] += num_per_type[i];
      }
      stats << group_stats << std::endl;
    }
  }
  // Sums
  std::string sums = "Summe, " + std::to_string(sum) + ", ";
  for (size_t i = 0; i < filter_sums.size(); ++i) {
    sums += std::to_string(filter_sums[i]) + (i + 1 == filters.size() ? "" : ", ");
  }
  stats << sums << std::endl;
}
