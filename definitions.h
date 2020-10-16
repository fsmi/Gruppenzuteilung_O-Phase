#pragma once

#include <functional>
#include <string>
#include <vector>

static const uint32_t MIN_GROUP_SIZE = 13;
static const double CAPACITY_BUFFER = 1.05;
static const bool VERBOSE = false;

// ####################################
// ########     Input Data     ########
// ####################################

struct Rating {
  uint32_t index;

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
  Info = 0,
  Mathe = 1,
  Any = 2,
};

enum class DegreeType {
  Bachelor = 0,
  Master = 1,
  Any = 2,
};

struct GroupData {
  std::string name;
  StudentID capacity;
  CourseType course_type;
  DegreeType degree_type;

  GroupData(std::string name, StudentID capacity, CourseType ct, DegreeType dt);
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
  std::vector<StudentID> _group_capacities;
  std::vector<bool> _group_enabled;
  std::vector<std::vector<StudentID>> _group_assignments;
  std::vector<uint32_t> _group_weights;
  std::vector<Participant> _participants;

public:
  State(const Input &data);

  const Input &data() const;

  GroupID numGroups() const;

  GroupID numActiveGroups() const;

  StudentID totalActiveGroupCapacity() const;

  const GroupData &groupData(GroupID id) const;

  StudentID groupCapacity(GroupID id) const;

  bool groupIsEnabled(GroupID id) const;

  const std::vector<StudentID> &groupAssignmentList(GroupID id) const;

  StudentID groupSize(GroupID id) const;

  uint32_t groupWeight(GroupID id) const;

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
  void resetWithCapacity(StudentID capacity);

  void reset();

  void decreaseCapacity(GroupID id, StudentID val);
};