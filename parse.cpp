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
  input.students = parseList<StudentData>(tree.find("students")->second, &parseStudent);
  input.teams = parseList<TeamData>(tree.find("teams")->second, &parseTeam);
  input.ratings = parseList<std::vector<Rating>>(tree.find("ratings")->second, [](const PTree& t) {
      return parseList<Rating>(t, &parseRating);
  });
  return std::move(input);
}
