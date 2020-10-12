#include <iostream>
#include <assert.h>

#include "algorithms.h"

int main() {
  Input input;
  input.groups.emplace_back("First Contact", "FC Team Gecko", 3, CourseType::Any, DegreeType::Any);
  input.groups.emplace_back("Team Gecko", "FC Team Gecko", 3, CourseType::Any, DegreeType::Any);
  input.groups.emplace_back("Mathe", "Project Pi", 3, CourseType::Mathe, DegreeType::Any);
  input.groups.emplace_back("Master", "Second Contact", 3, CourseType::Any, DegreeType::Master);
  input.students.emplace_back("Ersti 1", CourseType::Info, DegreeType::Bachelor, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(4), Rating(3), Rating(2), Rating(1) });
  input.students.emplace_back("Ersti 2", CourseType::Info, DegreeType::Bachelor, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(1), Rating(2), Rating(3), Rating(4) });
  input.students.emplace_back("Ersti 3", CourseType::Info, DegreeType::Bachelor, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(3), Rating(3), Rating(4), Rating(4) });
  input.students.emplace_back("Ersti 4", CourseType::Info, DegreeType::Bachelor, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(2), Rating(1), Rating(1), Rating(1) });
  input.students.emplace_back("Mathe 1", CourseType::Mathe, DegreeType::Bachelor, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(1), Rating(1), Rating(4), Rating(1) });
  input.students.emplace_back("Mathe 2", CourseType::Mathe, DegreeType::Bachelor, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(1), Rating(2), Rating(3), Rating(4) });
  input.students.emplace_back("Mathe 3", CourseType::Mathe, DegreeType::Bachelor, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(4), Rating(3), Rating(2), Rating(3) });
  input.students.emplace_back("Master", CourseType::Info, DegreeType::Master, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(1), Rating(2), Rating(3), Rating(4) });
  input.students.emplace_back("Lerngruppenteilnehmer 1", CourseType::Any, DegreeType::Any, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(3), Rating(2), Rating(1), Rating(1) });
  input.teams.emplace_back("Lerngruppe A", std::vector<StudentID> { 8 });

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

  std::cout << "Disable group \"First Contact\" and assign \"Ersti 1\" to \"Master\"."
            << std::endl << std::endl;

  for (GroupData& group : input.groups) {
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

  input.students.emplace_back("Lerngruppenteilnehmer 2", CourseType::Any, DegreeType::Any, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(3), Rating(2), Rating(1), Rating(1) });
  input.students.emplace_back("Lerngruppenteilnehmer 3", CourseType::Any, DegreeType::Any, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(3), Rating(2), Rating(1), Rating(1) });
  input.students.emplace_back("Lerngruppenteilnehmer 4", CourseType::Any, DegreeType::Any, false);
  input.ratings.emplace_back(std::vector<Rating> { Rating(3), Rating(2), Rating(1), Rating(1) });
  input.teams[0].members.push_back(9);
  input.teams[0].members.push_back(10);
  input.teams[0].members.push_back(11);

  for (GroupData& group : input.groups) {
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

  for (GroupData& group : input.groups) {
    group.capacity = 5;
  }

  s = State(input);
  assignWithMinimumNumberPerGroup(s, 2);
  printCurrentAssignment(s);
  // Erstis
  for (ParticipantID part = 1; part < 5; ++part) {
    assert(s.assignment(part) < 2);
  }
  // Mathes
  for (ParticipantID part = 6; part < 8; ++part) {
    assert(s.assignment(part) < 3);
  }
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    assert(s.groupSize(group) == 0 || s.groupSize(group) >= 2);
  }

  for (GroupData& group : input.groups) {
    group.capacity = 6;
  }

  input.groups[1].capacity = 7;

  s = State(input);
  assignWithMinimumNumberPerGroup(s, 3);
  printCurrentAssignment(s);
  // Erstis
  for (ParticipantID part = 1; part < 5; ++part) {
    assert(s.assignment(part) < 2);
  }
  // Mathes
  for (ParticipantID part = 6; part < 8; ++part) {
    assert(s.assignment(part) < 3);
  }
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    assert(s.groupSize(group) == 0 || s.groupSize(group) >= 3);
  }
}
