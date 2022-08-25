#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "parse.h"

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
    std::cerr << "Invalider Typ des Studiengangs: " << name << std::endl;
    std::exit(-1);
  }
}

DegreeType parseDegreeType(const std::string& name) {
  if (name == "bachelor") {
    return DegreeType::Bachelor;
  } else if (name == "master") {
    return DegreeType::Master;
  } else {
    std::cerr << "Invalider Typ des Abschlusses: " << name << std::endl;
    std::exit(-1);
  }
}

Semester parseSemester(const std::string& name) {
  if (name == "ersti") {
    return Semester::Ersti;
  } else if (name == "dritti") {
    return Semester::Dritti;
  } else {
    std::cerr << "Invalides Semester: " << name << std::endl;
    std::exit(-1);
  }
}

TeamData parseTeam(const std::string id, const PTree &tree, const std::unordered_map<std::string, size_t>& student_id_to_index) {
  std::vector<StudentID> members =
    parseList<StudentID>(tree, [&](const auto &t) {
      std::string id = t.second.PTree::get_value<std::string>();
      if (student_id_to_index.find(id) == student_id_to_index.end()) {
        std::cout << "Student ID not found: " << id << std::endl;
      }
      return student_id_to_index.at(t.second.PTree::get_value<std::string>());
    });
  return TeamData(id, members);
}

std::vector<Rating> parseRatings(const PTree &tree, const std::unordered_map<std::string, size_t>& group_id_to_index, size_t num_groups) {
  std::vector<std::string> group_order = parseList<std::string>(tree,
    [&](const auto &t) {
      return t.second.PTree::get_value<std::string>();
    });
  if (group_order.size() != num_groups) {
    std::cerr << "Rating list has different size (" << group_order.size()
              << ") than number of groups (" << num_groups << ")." << std::endl;
    std::exit(-1);
  }
  std::vector<Rating> result(num_groups);
  for (size_t i = 0; i < group_order.size(); ++i) {
    size_t group_index = group_id_to_index.at(group_order[i]);
    result[group_index] = Rating(i);
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
    CourseType course_type = parseCourseType(tree.get<std::string>("course_type"));
    return GroupData(id, name, capacity, course_type);
  };
  auto parseStudent = [](auto element){
    std::string id = element.first;
    PTree tree = element.second;
    std::string name = tree.get<std::string>("name");
    CourseType course_type = parseCourseType(tree.get<std::string>("course_type"));
    DegreeType degree_type = parseDegreeType(tree.get<std::string>("degree_type"));
    Semester semester = parseSemester(tree.get<std::string>("semester"));
    return StudentData(id, name, course_type, degree_type, semester);
  };
  input.groups = parseList<GroupData>(tree.find("groups")->second, parseGroup);
  auto group_mapping = createMapping(input.groups);
  input.students = parseList<StudentData>(tree.find("students")->second, parseStudent);
  auto student_mapping = createMapping(input.students);
  input.teams = parseList<TeamData>(tree.find("teams")->second,
    [&](const auto &t) {
      return parseTeam(t.first, t.second, student_mapping);
    });
  std::vector<std::vector<Rating>> ratings(input.students.size());
  for (auto &element : tree.find("ratings")->second) {
    auto rating_list = parseRatings(element.second, group_mapping, input.groups.size());
    ratings[student_mapping.at(element.first)] = rating_list;
  }
  input.ratings = std::move(ratings);
  return input;
}

PTree writeOutputToTree(const State &s) {
  PTree root;
  std::unordered_set<std::string> considered_students;
  for (StudentID participant = 0; participant < s.numParticipants(); ++participant) {
    std::string group_id = s.groupData(s.getAssignment(participant)).id;
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
  if (considered_students.size() != s.data().students.size()) {
    std::cerr << "WARNING: Output data points (" << considered_students.size()
              << ") don't match the number of students (" << s.data().students.size() << ")!" << std::endl;
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
    const std::vector<std::pair<std::function<bool(const StudentData&)>, std::string>> &filters) {
  std::string removed_path = path + "/RemovedGroups";
  std::ofstream removed(removed_path);
  std::string stats_path = path + "/Stats.csv";
  std::ofstream stats(stats_path);

  // csv header
  std::string header = "Name, Size, ";
  for (size_t i = 0; i < filters.size(); ++i) {
    header += filters[i].second + (i + 1 == filters.size() ? "" : ", ");
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
          if (filters[i].first(data)) {
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
