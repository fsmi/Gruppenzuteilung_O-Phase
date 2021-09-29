#include <fstream>
#include <iostream>
#include <unordered_map>

#include "parse.h"

template <typename T, typename F>
std::vector<T> parseList(const PTree &tree, F fn) {
  std::vector<T> result;
  for (auto &element : tree) {
    result.push_back(fn(element.second));
  }
  return std::move(result);
}

template <typename T>
std::unordered_map<std::string, size_t> createMapping(const std::vector<T> &list) {
  std::unordered_map<std::string, size_t> result;
  for (size_t i = 0; i < list.size(); ++i) {
    result.insert({list[i].id, i});
  }
  return std::move(result);
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

GroupData parseGroup(const PTree &tree) {
  std::string id = tree.get<std::string>("id");
  std::string name = tree.get<std::string>("name");
  StudentID capacity = tree.get<StudentID>("capacity");
  CourseType course_type = parseCourseType(tree.get<std::string>("course_type"));
  return GroupData(id, name, capacity, course_type);
}

StudentData parseStudent(const PTree &tree) {
  std::string id = tree.get<std::string>("id");
  std::string name = tree.get<std::string>("name");
  CourseType course_type = parseCourseType(tree.get<std::string>("course_type"));
  DegreeType degree_type = parseDegreeType(tree.get<std::string>("degree_type"));
  Semester semester = parseSemester(tree.get<std::string>("semester"));
  return StudentData(id, name, course_type, degree_type, semester);
}

TeamData parseTeam(const PTree &tree, const std::unordered_map<std::string, size_t>& student_id_to_index) {
  std::string id = tree.get<std::string>("id");
  std::vector<StudentID> members =
    parseList<StudentID>(tree.find("members")->second, [&](const PTree &t) {
      return student_id_to_index.at(t.get_value<std::string>());
    });
  return TeamData(id, members);
}

std::vector<Rating> parseRatings(const PTree &tree, const std::unordered_map<std::string, size_t>& group_id_to_index, size_t num_groups) {
  std::vector<std::string> group_order = parseList<std::string>(tree,
    [&](const PTree &t) {
      return t.get_value<std::string>();
    });
  if (group_order.size() != num_groups) {
    std::cerr << "Rating list has different size (" << group_order.size()
              << ") then number of groups (" << num_groups << ")." << std::endl;
    std::exit(-1);
  }
  std::vector<Rating> result(num_groups);
  for (size_t i = 0; i < group_order.size(); ++i) {
    size_t group_index = group_id_to_index.at(group_order[i]);
    result[group_index] = Rating(i);
  }
  return std::move(result);
}

Input parseInput(const PTree &tree) {
  Input input;
  input.groups = parseList<GroupData>(tree.find("groups")->second, &parseGroup);
  auto group_mapping = createMapping(input.groups);
  input.students = parseList<StudentData>(tree.find("students")->second, &parseStudent);
  auto student_mapping = createMapping(input.students);
  input.teams = parseList<TeamData>(tree.find("teams")->second,
    [&](const PTree &t) {
      return parseTeam(t, student_mapping);
    });
  assert(tree.find("ratings")->second.size() == input.students.size());
  std::vector<std::vector<Rating>> ratings(input.students.size());
  for (auto &element : tree.find("ratings")->second) {
    auto rating_list = parseRatings(element.second, group_mapping, input.groups.size());
    ratings[student_mapping.at(element.first)] = rating_list;
  }
  input.ratings = std::move(ratings);
  return std::move(input);
}

PTree writeStudent(const Input &input, GroupID group, StudentID student) {
  // TODO
  StudentData data = input.students[student];
  PTree tree;
  tree.put<std::string>("name", data.name);
  tree.put<int>("course_type", static_cast<int>(data.course_type));
  tree.put<int>("degree_type", static_cast<int>(data.degree_type));
  // tree.put<uint32_t>("rating", input.ratings[student][group].index);
  return std::move(tree);
}

PTree writeOutputToTree(const State &s) {
  // TODO
  PTree root;
  PTree groups;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    PTree group_tree;
    const GroupData &gd = s.groupData(group);
    group_tree.put<int>("course_type", static_cast<int>(gd.course_type));
    group_tree.put<uint32_t>("size", s.groupSize(group));
    PTree student_array;
    for (const auto &pair : s.groupAssignmentList(group)) {
      student_array.push_back(
          std::make_pair("", writeStudent(s.data(), group, pair.first)));
    }
    group_tree.add_child("students", std::move(student_array));
    groups.push_back(std::make_pair("", group_tree));
  }
  root.add_child("groups", std::move(groups));
  return std::move(root);
}

// Writes the output in (more) human-readable form to the specified path
void writeOutputToFiles(const State &s, std::string path) {
  // TODO
  std::string removed_path = path + "/RemovedGroups";
  std::ofstream removed(removed_path);
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    assert(group < s.numGroups());
    std::string group_path = path + "/" + s.groupData(group).name;
    if (s.groupAssignmentList(group).empty()) {
      removed << s.groupData(group).name << std::endl;
    } else {
      std::ofstream file(group_path);
      for (const auto &pair : s.groupAssignmentList(group)) {
        StudentData data = s.data().students[pair.first];
        const std::string &rating = s.rating(pair.second)[group].getName();
        std::string course;
        switch (data.course_type) {
        case CourseType::Info:
          course = "Info";
          break;
        case CourseType::Mathe:
          course = "Mathe";
          break;
        case CourseType::Any:
          course = "None";
          break;
        }
        std::string degree;
        switch (data.degree_type) {
        case DegreeType::Bachelor:
          degree = "Bachelor";
          break;
        case DegreeType::Master:
          degree = "Master";
          break;
        case DegreeType::Any:
          degree = "None";
          break;
        }
        file << data.name << "," << course << "," << degree << "," << " [" << rating << "]" << std::endl;
      }
    }
  }
}
