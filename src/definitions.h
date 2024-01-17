#pragma once

#include <functional>
#include <limits>
#include <string>
#include <vector>
#include <random>

// ####################################
// ########   Logging Macros   ########
// ####################################

#define GREEN "\033[1;92m"
#define CYAN "\033[1;96m"
#define YELLOW "\033[1;93m"
#define RED "\033[1;91m"
#define END "\033[0m"

#define LOG(msg, verbosity) \
  do { \
    if (Config::get().verbosity_level > 0 \
        && (verbosity) <= Config::get().verbosity_level) { \
      std::cout << msg << std::endl; \
    } \
  } while (false)

#define ERROR(msg, top_level) LOG(RED << "[ERROR]" << END << " " << msg, top_level ? 0 : 1)
#define INFO(msg, top_level) LOG(GREEN << "[INFO]" << END << " " << msg, top_level ? 1 : 2)
#define WARNING(msg, top_level) LOG(YELLOW << "[WARNING]" << END << " " << msg, top_level ? 1 : 2)

#define MAJOR_PROGRESS(msg, top_level) LOG(GREEN << ">>" << END << " " << msg, top_level ? 1 : 2)
#define PROGRESS(msg, top_level) LOG(GREEN << ">>" << END << " " << msg, top_level ? 2 : 3)
#define TRACE_START CYAN << ">" << END << "  "
#define MAJOR_TRACE(msg, top_level) LOG(TRACE_START << msg, top_level ? 2 : 3)
#define TRACE(msg, top_level) LOG(TRACE_START << msg, top_level ? 3 : 4)
#define DEBUG(msg) LOG(msg, 5)

#define FATAL_ERROR(msg) std::cout << RED << "[ERROR]" << END << " " << msg << std::endl; std::exit(-1)

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define ASSERT_WITH(cond, msg) \
  do { \
    if (!(cond)) { \
      std::cout << RED << "[ERROR]" << END << " Assertion at: " \
                << __FILENAME__ << ":" << __LINE__ \
                << " - `ASSERT(" << #cond << ")`" << std::endl; \
      std::cout << msg << std::endl; \
      std::exit(-1); \
    } \
  } while (false)
#define ASSERT(cond) ASSERT_WITH(cond, "")


// ####################################
// ########     Input Data     ########
// ####################################

using GroupID = uint32_t;
using StudentID = uint32_t;
using ParticipantID = uint32_t;

struct Rating {
  uint32_t index;

  Rating(): index(std::numeric_limits<uint32_t>::max()) {};

  Rating(uint32_t index);

  uint32_t getValue(GroupID num_groups) const;

  std::string getName() const;

  bool operator==(const Rating &other) const;

  bool operator!=(const Rating &other) const;

  bool isValid() const;

  static Rating minRating(GroupID num_groups);
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
  StudentID min_target_size;
  CourseType course_type;
  DegreeType degree_type;

  GroupData(std::string id, std::string name, StudentID capacity,
            StudentID min_target_size, CourseType ct, DegreeType dt);
};

struct StudentData {
  std::string id;
  std::string name;
  CourseType course_type;
  DegreeType degree_type;
  Semester semester;
  bool type_specific_assignment;

  StudentData(std::string id, std::string name, CourseType ct, DegreeType dt, Semester s, bool ts);
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
  std::unordered_map<std::string, std::string> student_id_to_team_id;
};

// #########################################
// ########     Assignment Data     ########
// #########################################

using FilterFn = std::function<bool(const StudentData&)>;

struct Filter {
  explicit Filter(std::vector<std::pair<FilterFn, uint32_t>>&& filters, std::string&& name);

  bool apply(const StudentData& data) const;

  uint32_t id() const;

  std::vector<std::pair<FilterFn, uint32_t>> filters;
  std::string name;
};

bool ratingsEqual(const std::vector<Rating> &r1, const std::vector<Rating> &r2);

struct Participant {
  uint32_t index;
  bool is_team;
  int32_t assignment;

  Participant(uint32_t index, bool is_team);
};

struct GroupState {
  StudentID reduced_capacity = 0;
  bool enabled = true;
  uint32_t weight = 0;
  std::vector<Filter> participant_filters;
};

// the state of the complete calculation
class State {
  std::reference_wrapper<const Input> _data;
  std::vector<GroupState> _group_states;
  std::vector<std::vector<std::pair<StudentID, ParticipantID>>>
      _group_assignments;
  std::vector<Participant> _participants;
  std::vector<bool> _type_specific_assignment;
  std::reference_wrapper<std::mt19937_64> _random_gen;

public:
  State(Input &data, std::mt19937_64& random_gen);

  const Input &data() const;

  GroupID numGroups() const;

  GroupID numActiveGroups() const;

  StudentID totalActiveGroupCapacity() const;

  const GroupData &groupData(GroupID id) const;

  // remaining capacity of the group
  StudentID groupCapacity(GroupID id) const;

  StudentID groupMinSize(GroupID id) const;

  bool groupIsEnabled(GroupID id) const;

  const std::vector<std::pair<StudentID, ParticipantID>> &
  groupAssignmentList(GroupID id) const;

  // number of students in the group
  StudentID groupSize(GroupID id) const;

  uint32_t groupWeight(GroupID id) const;

  StudentID numStudents() const;

  ParticipantID numParticipants() const;

  bool isTeam(ParticipantID id) const;

  bool isAssigned(ParticipantID id) const;

  GroupID getAssignment(ParticipantID id) const;

  const StudentData &studentData(ParticipantID id) const;

  const TeamData &teamData(ParticipantID id) const;

  GroupID assignment(ParticipantID id) const;

  const std::vector<Rating> &rating(ParticipantID id) const;

  void disableGroup(GroupID id);

  void addFilterToGroup(GroupID id, Filter filter);

  bool groupContainsFilter(GroupID id, const Filter& filter) const;

  bool filterAppliesToParticipant(ParticipantID participant, const Filter& filter) const;

  bool isExludedFromGroup(ParticipantID participant, GroupID group) const;

  bool assignParticipant(ParticipantID participant, GroupID target);

  void unassignParticipant(ParticipantID participant, GroupID group);

  void reset();

  void setCapacity(GroupID id, uint32_t val);

  void disableTypeSpecificAssignment(StudentID student);

  bool typeSpecificAssignment(StudentID student) const;

  StudentID partIDToStudentID(ParticipantID id) const;

  std::mt19937_64& getRandomness();

 private:
  bool studentIsExludedFromGroup(StudentID participant, GroupID group) const;
};