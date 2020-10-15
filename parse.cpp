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
  std::string main_group = tree.get<std::string>("main_group");
  StudentID capacity = tree.get<StudentID>("capacity");
  CourseType course_type =
      static_cast<CourseType>(tree.get<int>("course_type"));
  DegreeType degree_type =
      static_cast<DegreeType>(tree.get<int>("degree_type"));
  return GroupData(name, main_group, capacity, course_type, degree_type);
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

PTree writeOutput(const State &s) {
  PTree root;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    PTree group_tree;
    const GroupData &gd = s.groupData(group);
    group_tree.put<std::string>("name", gd.name);
    group_tree.put<std::string>("main_group", gd.main_group);
    group_tree.put<int>("course_type", static_cast<int>(gd.course_type));
    group_tree.put<int>("degree_type", static_cast<int>(gd.degree_type));
    group_tree.put<uint32_t>("size", s.groupSize(group));
    for (const StudentID &student : s.groupAssignmentList(group)) {
      group_tree.push_back(
          std::make_pair("", writeStudent(s.data(), group, student)));
    }
    root.push_back(std::make_pair("", std::move(group_tree)));
  }
  return std::move(root);
}
