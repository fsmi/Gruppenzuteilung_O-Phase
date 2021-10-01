#pragma once

#include <functional>
#include <string>
#include <vector>

// TODO: Max team size
static const size_t DEGREE_PER_GROUP = 7;
static const size_t DISABLED_GROUPS_PER_STEP = 1;
static const uint32_t MIN_GROUP_SIZE = 12;
static const uint32_t MAX_GROUP_SIZE = 200;
static const double CAPACITY_BUFFER = 1.05;
static const bool VERBOSE = false;

// ####################################
// ########     Input Data     ########
// ####################################

using GroupID = uint32_t;
using StudentID = uint32_t;
using ParticipantID = uint32_t;

struct Rating {
  uint32_t index;

  Rating(uint32_t index);

  uint32_t getValue(GroupID num_groups) const;

  std::string getName() const;

  bool operator==(const Rating &other) const;

  bool operator!=(const Rating &other) const;

  Rating() = default;
};

enum class CourseType {
  Info = 0,
  Mathe = 1,
  Lehramt = 2,
  Any = 3,
};

enum class DegreeType {
  Bachelor = 0,
  Master = 1,
  Any = 2,
};

enum class Semester {
  Ersti = 0,
  Dritti = 1,
};

struct GroupData {
  std::string id;
  std::string name;
  StudentID capacity;
  CourseType course_type;

  GroupData(std::string id, std::string name, StudentID capacity, CourseType ct);
};

struct StudentData {
  std::string id;
  std::string name;
  CourseType course_type;
  DegreeType degree_type;
  Semester semester;

  StudentData(std::string id, std::string name, CourseType ct, DegreeType dt, Semester s);
};

struct TeamData {
  std::string id;
  std::vector<StudentID> members;

  TeamData(std::string id, std::vector<StudentID> members);

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

struct GroupState {
  StudentID capacity = 0;
  bool enabled = true;
  uint32_t weight = 0;
  // TODO
  std::vector<std::function<bool(const StudentData&)>> participant_filters;
};

// the state of the complete calculation
class State {
  std::reference_wrapper<const Input> _data;
  std::vector<GroupState> _group_states;
  std::vector<std::vector<std::pair<StudentID, ParticipantID>>>
      _group_assignments;
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

  const std::vector<std::pair<StudentID, ParticipantID>> &
  groupAssignmentList(GroupID id) const;

  StudentID groupSize(GroupID id) const;

  uint32_t groupWeight(GroupID id) const;

  ParticipantID numParticipants() const;

  bool isTeam(ParticipantID id) const;

  bool isAssigned(ParticipantID id) const;

  GroupID getAssignment(ParticipantID id) const;

  const StudentData &studentData(ParticipantID id) const;

  const TeamData &teamData(ParticipantID id) const;

  GroupID assignment(ParticipantID id) const;

  const std::vector<Rating> &rating(ParticipantID id) const;

  void disableGroup(GroupID id);

  void addFilterToGroup(GroupID id, std::function<bool(const StudentData&)> filter);

  bool isExludedFromGroup(ParticipantID participant, GroupID group) const;

  bool assignParticipant(ParticipantID participant, GroupID target);

  void unassignParticipant(ParticipantID participant, GroupID group);

  // groups are still disabled
  void resetWithCapacity(StudentID capacity);

  void reset();

  void setCapacity(GroupID id, uint32_t val);

 private:
  bool studentIsExludedFromGroup(StudentID participant, GroupID group) const;
};