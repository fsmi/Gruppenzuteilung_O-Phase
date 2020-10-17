#include <fstream>
#include <iostream>

#include "parse.h"

template <typename T, typename F>
std::vector<T> parseList(const PTree &tree, F fn) {
  std::vector<T> result;
  for (auto &element : tree) {
    result.push_back(fn(element.second));
  }
  return std::move(result);
}

GroupData parseGroup(const PTree &tree) {
  std::string name = tree.get<std::string>("name");
  StudentID capacity =
      std::min(tree.get<StudentID>("capacity"), StudentID(MAX_GROUP_SIZE));
  CourseType course_type =
      static_cast<CourseType>(tree.get<int>("course_type"));
  DegreeType degree_type =
      static_cast<DegreeType>(tree.get<int>("degree_type"));
  return GroupData(name, capacity, course_type, degree_type);
}

StudentData parseStudent(const PTree &tree) {
  std::string name = tree.get<std::string>("name");
  CourseType course_type =
      static_cast<CourseType>(tree.get<int>("course_type"));
  DegreeType degree_type =
      static_cast<DegreeType>(tree.get<int>("degree_type"));
  bool is_commuter = tree.get<bool>("is_commuter");
  return StudentData(name, course_type, degree_type, is_commuter);
}

TeamData parseTeam(const PTree &tree) {
  std::string name = tree.get<std::string>("name");
  std::vector<StudentID> members =
      parseList<StudentID>(tree.find("members")->second, [](const PTree &t) {
        return t.get_value<StudentID>();
      });
  return TeamData(name, members);
}

Rating parseRating(const PTree &tree) {
  return Rating(tree.get_value<uint32_t>());
}

Input parseInput(const PTree &tree) {
  Input input;
  input.groups = parseList<GroupData>(tree.find("groups")->second, &parseGroup);
  input.students =
      parseList<StudentData>(tree.find("students")->second, &parseStudent);
  input.teams = parseList<TeamData>(tree.find("teams")->second, &parseTeam);
  input.ratings = parseList<std::vector<Rating>>(
      tree.find("ratings")->second,
      [](const PTree &t) { return parseList<Rating>(t, &parseRating); });
  return std::move(input);
}

PTree writeStudent(const Input &input, GroupID group, StudentID student) {
  StudentData data = input.students[student];
  PTree tree;
  tree.put<std::string>("name", data.name);
  tree.put<int>("course_type", static_cast<int>(data.course_type));
  tree.put<int>("degree_type", static_cast<int>(data.degree_type));
  tree.put<bool>("is_commuter", data.is_commuter);
  tree.put<uint32_t>("rating", input.ratings[student][group].index);
  return std::move(tree);
}

PTree writeOutputToTree(const State &s) {
  PTree root;
  PTree groups;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    PTree group_tree;
    const GroupData &gd = s.groupData(group);
    group_tree.put<int>("course_type", static_cast<int>(gd.course_type));
    group_tree.put<int>("degree_type", static_cast<int>(gd.degree_type));
    group_tree.put<uint32_t>("size", s.groupSize(group));
    PTree student_array;
    for (const StudentID &student : s.groupAssignmentList(group)) {
      student_array.push_back(
          std::make_pair("", writeStudent(s.data(), group, student)));
    }
    group_tree.add_child("students", std::move(student_array));
    groups.push_back(std::make_pair("", group_tree));
  }
  root.add_child("groups", std::move(groups));
  return std::move(root);
}

void writeOutputToFiles(const State &s, std::string path) {
  std::string removed_path = path + "/RemovedGroups";
  std::ofstream removed(removed_path);
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    assert(group < s.numGroups());
    std::string group_path = path + "/" + s.groupData(group).name;
    if (s.groupAssignmentList(group).empty()) {
      removed << s.groupData(group).name << std::endl;
    } else {
      std::ofstream file(group_path);
      for (const StudentID &student : s.groupAssignmentList(group)) {
        StudentData data = s.data().students[student];
        const std::string &rating = s.data().ratings[student][group].getName();
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
        file << data.name << "," << course << "," << degree << ","
             << data.is_commuter << " [" << rating << "]" << std::endl;
      }
    }
  }
}
