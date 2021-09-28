#include <assert.h>
#include <iostream>

#include "algorithms.h"
#include "moves_local_search.h"

int main() {
  Input input;
  input.groups.emplace_back("First Contact", 3, CourseType::Any,
                            DegreeType::Any);
  input.groups.emplace_back("Team Gecko", 3, CourseType::Any, DegreeType::Any);
  input.groups.emplace_back("Mathe", 3, CourseType::Mathe, DegreeType::Any);
  input.groups.emplace_back("Master", 3, CourseType::Any, DegreeType::Master);
  input.students.emplace_back("Ersti 1", CourseType::Info, DegreeType::Bachelor,
                              false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(4), Rating(3), Rating(2), Rating(1)});
  input.students.emplace_back("Ersti 2", CourseType::Info, DegreeType::Bachelor,
                              false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(1), Rating(2), Rating(3), Rating(4)});
  input.students.emplace_back("Ersti 3", CourseType::Info, DegreeType::Bachelor,
                              false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(3), Rating(3), Rating(4), Rating(4)});
  input.students.emplace_back("Ersti 4", CourseType::Info, DegreeType::Bachelor,
                              false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(2), Rating(1), Rating(1), Rating(1)});
  input.students.emplace_back("Mathe 1", CourseType::Mathe,
                              DegreeType::Bachelor, false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(1), Rating(1), Rating(4), Rating(1)});
  input.students.emplace_back("Mathe 2", CourseType::Mathe,
                              DegreeType::Bachelor, false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(1), Rating(2), Rating(3), Rating(4)});
  input.students.emplace_back("Mathe 3", CourseType::Mathe,
                              DegreeType::Bachelor, false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(4), Rating(3), Rating(2), Rating(3)});
  input.students.emplace_back("Master", CourseType::Info, DegreeType::Master,
                              false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(1), Rating(2), Rating(3), Rating(4)});
  input.students.emplace_back("Lerngruppenteilnehmer 1", CourseType::Any,
                              DegreeType::Any, false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(3), Rating(2), Rating(1), Rating(1)});
  input.teams.emplace_back("Lerngruppe A", std::vector<StudentID>{8});

  State s(input);
  auto assignment = calculateAssignment(s);
  applyAssignment(s, assignment);
  printCurrentAssignment(s);
  // Erstis
  for (ParticipantID part = 1; part < 5; ++part) {
    assert(s.assignment(part) < 2);
  }
  // Mathes
  for (ParticipantID part = 6; part < 7; ++part) {
    assert(s.assignment(part) == 2);
  }
  // Master
  assert(s.assignment(8) == 3);

  std::cout
      << "Disable group \"First Contact\" and assign \"Ersti 1\" to \"Master\"."
      << std::endl
      << std::endl;

  for (GroupData &group : input.groups) {
    group.capacity = 4;
  }

  s.disableGroup(0);
  s.reset();
  s.assignParticipant(1, 3);
  assignment = calculateAssignment(s);
  applyAssignment(s, assignment);
  printCurrentAssignment(s);
  // Erstis
  assert(s.assignment(1) == 3);
  for (ParticipantID part = 2; part < 5; ++part) {
    assert(s.assignment(part) < 2);
  }
  // Mathes
  for (ParticipantID part = 6; part < 7; ++part) {
    assert(s.assignment(part) == 2);
  }
  // Master
  assert(s.assignment(8) == 3);

  input.students.emplace_back("Lerngruppenteilnehmer 2", CourseType::Any,
                              DegreeType::Any, false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(3), Rating(2), Rating(1), Rating(1)});
  input.students.emplace_back("Lerngruppenteilnehmer 3", CourseType::Any,
                              DegreeType::Any, false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(3), Rating(2), Rating(1), Rating(1)});
  input.students.emplace_back("Lerngruppenteilnehmer 4", CourseType::Any,
                              DegreeType::Any, false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(3), Rating(2), Rating(1), Rating(1)});
  input.teams[0].members.push_back(9);
  input.teams[0].members.push_back(10);
  input.teams[0].members.push_back(11);

  for (GroupData &group : input.groups) {
    group.capacity = 4;
  }

  s = State(input);
  assignTeamsAndStudents(s);
  printCurrentAssignment(s);
  // Erstis
  for (ParticipantID part = 1; part < 5; ++part) {
    assert(s.assignment(part) < 2);
  }
  // Mathes
  for (ParticipantID part = 6; part < 7; ++part) {
    assert(s.assignment(part) == 2);
  }
  // Master
  assert(s.assignment(8) == 3);
  // Lerngruppe
  assert(s.groupSize(s.assignment(1)) >= 4);

  for (GroupData &group : input.groups) {
    group.capacity = 5;
  }

  input.students[4] =
      StudentData("Info 1", CourseType::Info, DegreeType::Bachelor, false);

  s = State(input);
  assignTeamsAndStudents(s);
  auto is_math = [](const StudentData &data) {
    return data.course_type == CourseType::Mathe;
  };
  printCurrentAssignment(s);
  assertMininumNumber(s, 2, is_math);
  std::vector<StudentID> num_per_group = numPerGroup(s, is_math);
  for (StudentID num : num_per_group) {
    assert(num == 0 || num >= 2);
  }

  // test filters
  input.students.emplace_back("Lerngruppenteilnehmer X", CourseType::Mathe,
                              DegreeType::Any, false);
  input.ratings.emplace_back(
      std::vector<Rating>{Rating(3), Rating(2), Rating(1), Rating(1)});
  input.teams.emplace_back("Lerngruppe X", std::vector<StudentID>{12});

  s = State(input);
  assert(!s.isExludedFromGroup(0, 0));
  s.addFilterToGroup(0, [](const StudentData& data) {
    return data.course_type == CourseType::Mathe;
  });
  // Lerngruppe A
  assert(!s.isExludedFromGroup(0, 0));
  // Lerngruppe X
  assert(s.isExludedFromGroup(1, 0));
  // students
  assert(!s.isExludedFromGroup(2, 0));
  assert(!s.isExludedFromGroup(3, 0));
  assert(s.isExludedFromGroup(7, 0));
  std::cout << "Basic test successful!" << std::endl;
}
