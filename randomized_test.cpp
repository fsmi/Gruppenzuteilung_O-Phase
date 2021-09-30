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
  std::string name("Group ");
  name += std::to_string(++counter);
  std::uniform_int_distribution<StudentID> capacity(15, 60);
  StudentID cap = capacity(GENERATOR);
  return GroupData(name, name, cap, is_math ? CourseType::Mathe : CourseType::Any);
}

StudentData createErsti(int &counter) {
  std::uniform_int_distribution<int> random(0, 19);
  int course_rand = random(GENERATOR);
  CourseType course = course_rand == 0 ? CourseType::Lehramt : (
    course_rand < 3 ? CourseType::Mathe : CourseType::Info
  );
  bool is_master = random(GENERATOR) < 2;
  bool is_ersti = random(GENERATOR) < 16;
  std::string name("Ersti ");
  name += std::to_string(++counter);
  return StudentData(name, name, course,
                     is_master ? DegreeType::Master : DegreeType::Bachelor,
                     is_ersti ? Semester::Ersti : Semester::Dritti);
}

std::vector<Rating> createRating(GroupID num_groups) {
  std::vector<Rating> result;
  for (GroupID i = 0; i < num_groups; ++i) {
    result.push_back(i);
  }
  std::shuffle(result.begin(), result.end(), GENERATOR);
  return result;
}

int main() {
  Input input;
  std::uniform_int_distribution<int> random(2, 3);
  int num_students = 600;
  int num_teams = 50;

  int group_counter = 0;
  int capacity = 0;
  while (capacity < 5 * num_students / 4) {
    input.groups.push_back(createGroup(group_counter));
    capacity += input.groups.back().capacity;
  }
  int student_counter = 0;
  while (student_counter < num_students) {
    input.students.push_back(createErsti(student_counter));
    input.ratings.push_back(createRating(input.groups.size()));
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
  assignWithMinimumNumberPerGroup(s, MIN_GROUP_SIZE);
  auto is_math_and_no_ma = [](const StudentData &data) {
    return data.course_type == CourseType::Mathe &&
           data.degree_type != DegreeType::Master;
  };
  auto is_lehramt_and_no_ma = [](const StudentData &data) {
    return data.course_type == CourseType::Lehramt &&
           data.degree_type != DegreeType::Master;
  };
  auto is_dritti_and_no_ma = [](const StudentData &data) {
    return data.semester == Semester::Dritti &&
           data.degree_type != DegreeType::Master;
  };
  auto is_master = [](const StudentData &data) {
    return data.degree_type == DegreeType::Master;
  };

  std::cout << std::endl << "Reassingning specific students." << std::endl;
  assertMinimumNumberPerGroupForSpecificType(s, {
    {is_math_and_no_ma, 5, "Mathe (nicht MA)"},
    {is_lehramt_and_no_ma, 5, "Lehramt (nicht MA)"},
    {is_dritti_and_no_ma, 7, "Dritti (nicht MA)"},
    {is_master, 5, "Master"}
  });
}
