#pragma once

#include <functional>
#include <string>
#include <vector>

static const uint32_t INITIAL_GROUP_CAPACITY = 18;

// ####################################
// ########     Input Data     ########
// ####################################

class Rating {
  uint32_t _index;

public:
  Rating(uint32_t index);

  uint32_t getValue() const;

  const char *getName() const;

  bool operator==(const Rating &other) const;

  bool operator!=(const Rating &other) const;
};

using GroupID = uint32_t;
using StudentID = uint32_t;
using ParticipantID = uint32_t;

enum class CourseType {
  Info,
  Mathe,
  Any,
};

enum class DegreeType {
  Bachelor,
  Master,
  Any,
};

struct GroupData {
  std::string name;
  std::string main_group;
  CourseType course_type;
  DegreeType degree_type;

  GroupData(std::string name, std::string main_group, CourseType ct,
            DegreeType dt);
};

struct StudentData {
  std::string name;
  CourseType course_type;
  DegreeType degree_type;
  bool is_commuter;

  StudentData(std::string name, CourseType ct, DegreeType dt, bool is_commuter);
};

struct TeamData {
  std::string name;
  std::vector<StudentID> members;

  TeamData(std::string name, std::vector<StudentID> members);

  size_t size() const;
};

struct Input {
  std::vector<GroupData> groups;
  std::vector<StudentData> students;
  std::vector<TeamData> teams;
  std::vector<std::vector<Rating>> ratings;
};

// #########################################
// ########     Assignment Data     ########
// #########################################

bool ratingsEqual(const std::vector<Rating> &r1, const std::vector<Rating> &r2);

struct Participant {
  uint32_t index;
  bool is_team;
  int32_t assignment;

  Participant(uint32_t index, bool is_team);
};

class State {
  std::reference_wrapper<const Input> _data;
  std::vector<uint32_t> _group_capacities;
  std::vector<bool> _group_enabled;
  std::vector<std::vector<StudentID>> _group_assignments;
  std::vector<Participant> _participants;

public:
  State(const Input &data, uint32_t group_capacity = INITIAL_GROUP_CAPACITY);

  const Input &data() const;

  GroupID numGroups() const;

  const GroupData &groupData(GroupID id) const;

  uint32_t groupCapacity(GroupID id) const;

  bool groupIsEnabled(GroupID id) const;

  const std::vector<StudentID> &groupAssignmentList(GroupID id) const;

  StudentID groupSize(GroupID id) const;

  ParticipantID numParticipants() const;

  bool isTeam(ParticipantID id) const;

  bool isAssigned(ParticipantID id) const;

  const StudentData &studentData(ParticipantID id) const;

  const TeamData &teamData(ParticipantID id) const;

  GroupID assignment(ParticipantID id) const;

  const std::vector<Rating> &rating(ParticipantID id) const;

  void disableGroup(GroupID id);

  bool assignParticipant(ParticipantID participant, GroupID target);

  // groups are still disabled
  void resetWithCapacity(uint32_t capacity);

  void reset();
};