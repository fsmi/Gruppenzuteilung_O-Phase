#include <assert.h>
#include <iostream>

#include "src/algorithms.h"

std::vector<StudentID>
numPerGroup(const State &s,
            std::function<bool(const StudentData &)> predicate) {
  std::vector<StudentID> num_per_group(s.numGroups(), 0);
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    const auto &list = s.groupAssignmentList(group);
    for (const auto &pair : list) {
      StudentData data = s.data().students[pair.first];
      if (predicate(data)) {
        num_per_group[group]++;
      }
    }
  }
  return num_per_group;
}

int main() {
  Input input;
  input.groups.emplace_back("fc", "First Contact", 4, 0, CourseType::Any, DegreeType::Any);
  input.groups.emplace_back("tg", "Team Gecko", 3, 0, CourseType::Any, DegreeType::Any);
  input.groups.emplace_back("mathe", "Mathe", 3, 0, CourseType::Mathe, DegreeType::Any);
  input.students.emplace_back("e1", "Ersti 1", CourseType::Info, DegreeType::Bachelor, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(0), Rating(1), Rating(2)});
  input.students.emplace_back("e2", "Ersti 2", CourseType::Info, DegreeType::Bachelor, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(2), Rating(1), Rating(0)});
  input.students.emplace_back("e3", "Ersti 3", CourseType::Info, DegreeType::Bachelor, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(1), Rating(2), Rating(0)});
  input.students.emplace_back("e4", "Ersti 4", CourseType::Info, DegreeType::Bachelor, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(0), Rating(1), Rating(2)});
  input.students.emplace_back("m1", "Mathe 1", CourseType::Mathe, DegreeType::Bachelor, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(2), Rating(1), Rating(0)});
  input.students.emplace_back("m2", "Mathe 2", CourseType::Mathe, DegreeType::Bachelor, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(2), Rating(1), Rating(0)});
  input.students.emplace_back("m3", "Mathe 3", CourseType::Mathe, DegreeType::Bachelor, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(0), Rating(1), Rating(2)});
  input.students.emplace_back("ma", "Master", CourseType::Info, DegreeType::Master, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(2), Rating(1), Rating(0)});
  input.students.emplace_back("l1", "Lerngruppenteilnehmer 1", CourseType::Any, DegreeType::Any, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(0), Rating(1), Rating(2)});
  input.teams.emplace_back("Lerngruppe A", std::vector<StudentID>{8});

  State s(input);
  auto assignment = calculateAssignment(s, true);
  applyAssignment(s, assignment.first);
  printCurrentAssignment(s);
  // Erstis
  for (ParticipantID part = 0; part < 4; ++part) {
    assert(s.assignment(part) < 2);
  }
  // Mathes
  for (ParticipantID part = 4; part < 6; ++part) {
    assert(s.assignment(part) == 2);
  }

  for (GroupData &group : input.groups) {
    group.capacity = 5;
  }

  s.disableGroup(0);
  s.reset();
  s.assignParticipant(0, 2);
  assignment = calculateAssignment(s, true);
  applyAssignment(s, assignment.first);
  printCurrentAssignment(s);
  // Erstis
  for (ParticipantID part = 1; part < 4; ++part) {
    assert(s.assignment(part) < 2);
  }
  // Mathes
  for (ParticipantID part = 4; part < 6; ++part) {
    assert(s.assignment(part) == 2);
  }

  input.students.emplace_back("l2", "Lerngruppenteilnehmer 2", CourseType::Any, DegreeType::Any, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(0), Rating(1), Rating(2)});
  input.students.emplace_back("l3", "Lerngruppenteilnehmer 3", CourseType::Any, DegreeType::Any, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(0), Rating(1), Rating(2)});
  input.students.emplace_back("l4", "Lerngruppenteilnehmer 4", CourseType::Any, DegreeType::Any, Semester::Ersti, true);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(0), Rating(1), Rating(2)});
  input.teams[0].members.push_back(9);
  input.teams[0].members.push_back(10);
  input.teams[0].members.push_back(11);

  for (GroupData &group : input.groups) {
    group.capacity = 5;
  }

  s = State(input);
  assignTeamsAndStudents(s, true);
  printCurrentAssignment(s);
  // Erstis
  for (ParticipantID part = 1; part < 5; ++part) {
    assert(s.assignment(part) < 2);
  }
  // Mathes
  for (ParticipantID part = 6; part < 7; ++part) {
    assert(s.assignment(part) == 2);
  }
  // Lerngruppe
  assert(s.groupSize(s.assignment(1)) >= 4);

  for (GroupData &group : input.groups) {
    group.capacity = 5;
  }
  input.students[4] =
      StudentData("info1", "Info 1", CourseType::Info, DegreeType::Bachelor, Semester::Ersti, true);
  auto is_math = [](const StudentData &data) noexcept {
    return data.course_type == CourseType::Mathe;
  };

  // minimum number with filters
  s = State(input);
  assignTeamsAndStudents(s, true);
  assertMinimumNumberPerGroupForSpecificType(s, {{Filter({{is_math, 0}}, "Mathe"), 2}});
  printCurrentAssignment(s);
  std::vector<StudentID> num_per_group = numPerGroup(s, is_math);
  for (StudentID num : num_per_group) {
    assert(num == 0 || num >= 2);
  }
  std::cout << "Filter test done." << std::endl << std::endl;

  // test filters
  // input.students.emplace_back("lx", "Lerngruppenteilnehmer X", CourseType::Mathe, DegreeType::Any, Semester::Ersti, true);
  // input.ratings.emplace_back(
  //     std::vector<Rating>{Rating(2), Rating(1), Rating(0)});
  // input.teams.emplace_back("Lerngruppe X", std::vector<StudentID>{12});

  // s = State(input);
  // assert(!s.isExludedFromGroup(0, 0));
  // s.addFilterToGroup(0, [](const StudentData& data) noexcept {
  //   return data.course_type == CourseType::Mathe;
  // });
  // // Lerngruppe A
  // assert(!s.isExludedFromGroup(0, 0));
  // // Lerngruppe X
  // assert(s.isExludedFromGroup(1, 0));
  // // students
  // assert(!s.isExludedFromGroup(2, 0));
  // assert(!s.isExludedFromGroup(3, 0));
  // assert(s.isExludedFromGroup(7, 0));
  std::cout << "Basic test successful!" << std::endl;
}
