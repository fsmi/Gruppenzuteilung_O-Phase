/*
A test using randomly generated data of similar size to the real data.
*/


#include <iostream>
#include <random>

#include "algorithms.h"
#include "moves_local_search.h"

static std::mt19937 GENERATOR;

GroupData createGroup(int &counter) {
  std::uniform_int_distribution<int> random(0, 9);
  bool is_math = random(GENERATOR) == 0;
  bool is_master = random(GENERATOR) == 0;
  std::string name("Group ");
  name += std::to_string(++counter);
  return GroupData(name, 18, is_math ? CourseType::Mathe : CourseType::Any,
                   is_master ? DegreeType::Master : DegreeType::Any);
}

StudentData createErsti(int &counter) {
  std::uniform_int_distribution<int> random(0, 9);
  bool is_math = random(GENERATOR) == 0;
  bool is_master = random(GENERATOR) == 0;
  std::string name("Ersti ");
  name += std::to_string(++counter);
  return StudentData(name, is_math ? CourseType::Mathe : CourseType::Info,
                     is_master ? DegreeType::Master : DegreeType::Bachelor,
                     false);
}

Rating createRating() {
  std::uniform_int_distribution<int> random(0, 4);
  return Rating(random(GENERATOR));
}

int main() {
  Input input;
  std::uniform_int_distribution<int> random(2, 5);
  int num_groups = 70;
  int num_students = 1000;
  int num_teams = 80;

  int group_counter = 0;
  while (group_counter < num_groups) {
    input.groups.push_back(createGroup(group_counter));
  }
  int student_counter = 0;
  while (student_counter < num_students) {
    input.students.push_back(createErsti(student_counter));
    std::vector<Rating> rating;
    for (int i = 0; i < num_groups; ++i) {
      rating.push_back(createRating());
    }
    input.ratings.push_back(std::move(rating));
  }
  StudentID student_id = 0;
  for (int team_counter = 0; team_counter < num_teams; ++team_counter) {
    int team_size = random(GENERATOR);
    std::vector<StudentID> members;
    const std::vector<Rating> &rating = input.ratings[student_id];
    for (int i = 0; i < team_size; ++i) {
      input.ratings[student_id] = rating;
      members.push_back(student_id++);
    }
    std::string name("Team ");
    name += std::to_string(team_counter);
    input.teams.emplace_back(name, std::move(members));
  }

  State s(input);
  assignWithMinimumNumberPerGroup(s, 5);
  auto is_math_and_no_ma = [](const StudentData &data) {
    return data.course_type == CourseType::Mathe &&
           data.degree_type != DegreeType::Master;
  };
  assertMininumNumber(s, 4, is_math_and_no_ma);
}
