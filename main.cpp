#include <iostream>
#include <string>
#include <vector>

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/maximum_weighted_matching.hpp>

using EdgeProperty = boost::property<boost::edge_weight_t, uint32_t>;
using Graph = boost::adjacency_matrix<boost::undirectedS, boost::no_property,
                                      EdgeProperty>;

static const uint32_t INITIAL_GROUP_CAPACITY = 18;
static const size_t NUM_RATINGS = 5;
static uint32_t RATING_VAL_TABLE[NUM_RATINGS] = {100, 120, 150, 180, 200};
static const char *RATING_NAME_TABLE[NUM_RATINGS] = {"--", "-", "O", "+", "++"};

// ####################################
// ########     Input Data     ########
// ####################################

class Rating {
  uint32_t _index;

public:
  Rating(uint32_t index) : _index(index) { assert(index < NUM_RATINGS); }

  uint32_t getValue() const { return RATING_VAL_TABLE[_index]; }

  const char *getName() const { return RATING_NAME_TABLE[_index]; }
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
};

struct StudentData {
  std::string name;
  CourseType course_type;
  DegreeType degree_type;
  bool is_commuter;
};

struct TeamData {
  std::string name;
  std::vector<StudentID> members;

  size_t size() const { return members.size(); }
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

struct Participant {
  uint32_t index;
  bool is_team;
  int32_t assignment;

  Participant(uint32_t index, bool is_team)
      : index(index), is_team(is_team), assignment(-1) {}
};

class State {
  std::reference_wrapper<const Input> _data;
  std::vector<uint32_t> _group_capacities;
  std::vector<bool> _group_enabled;
  std::vector<std::vector<StudentID>> _group_assignments;
  std::vector<Participant> _participants;

  const Input &data() const { return _data.get(); }

public:
  State(const Input &data, uint32_t group_capacity = INITIAL_GROUP_CAPACITY)
      : _data(data), _group_capacities(data.groups.size(), group_capacity),
        _group_enabled(data.groups.size(), true),
        _group_assignments(data.groups.size(), std::vector<StudentID>()),
        _participants() {
    assert(data.students.size() == data.ratings.size());
    std::vector<bool> is_in_team(data.students.size(), false);
    for (size_t i = 0; i < data.teams.size(); ++i) {
      const TeamData &team = data.teams[i];
      assert(!team.members.empty());
      for (const StudentID &student : team.members) {
        assert(student < is_in_team.size() && !is_in_team[student]);
        is_in_team[student] = true;
      }
    }
    for (size_t i = 0; i < data.students.size(); ++i) {
      if (!is_in_team[i]) {
        _participants.emplace_back(i, false);
      }
    }
  }

  GroupID numGroups() const { return data().groups.size(); }

  const GroupData &groupData(GroupID id) const {
    assert(id < data().groups.size());
    return data().groups[id];
  }

  uint32_t groupCapacity(GroupID id) const {
    assert(id < data().groups.size());
    return _group_capacities[id];
  }

  bool groupIsEnabled(GroupID id) const {
    assert(id < data().groups.size());
    return _group_enabled[id];
  }

  const std::vector<StudentID> &groupAssignmentList(GroupID id) const {
    assert(id < data().groups.size());
    return _group_assignments[id];
  }

  ParticipantID numParticipants() const { return _participants.size(); }

  bool isTeam(ParticipantID id) const {
    assert(id < _participants.size());
    return _participants[id].is_team;
  }

  bool isAssigned(ParticipantID id) const {
    assert(id < _participants.size());
    return _participants[id].assignment >= 0;
  }

  const StudentData &studentData(ParticipantID id) const {
    assert(!isTeam(id));
    StudentID student_id = _participants[id].index;
    assert(student_id < data().students.size());
    return data().students[student_id];
  }

  const TeamData &teamData(ParticipantID id) const {
    assert(isTeam(id));
    uint32_t team_id = _participants[id].index;
    assert(team_id < data().teams.size());
    return data().teams[team_id];
  }

  GroupID getAssignment(ParticipantID id) const {
    assert(isAssigned(id));
    return static_cast<GroupID>(_participants[id].assignment);
  }

  void disableGroup(GroupID id) {
    assert(id < data().groups.size());
    _group_enabled[id] = false;
  }

  bool assignParticipant(ParticipantID participant, GroupID target) {
    assert(!isAssigned(participant));
    assert(target < data().groups.size());

    if (isTeam(participant)) {
      const TeamData &data = teamData(participant);
      if (data.size() > groupCapacity(target)) {
        return false;
      }
      _group_capacities[target] -= data.size();
      for (StudentID id : data.members) {
        _group_assignments[target].push_back(id);
      }
    } else {
      if (groupCapacity(target) == 0) {
        return false;
      }
      _group_capacities[target]--;
      _group_assignments[target].push_back(_participants[participant].index);
    }
    _participants[participant].assignment = target;
    return true;
  }

  void resetWithCapacity(uint32_t capacity) {
    for (uint32_t &cap : _group_capacities) {
      cap = capacity;
    }
    for (size_t i = 0; i < _group_enabled.size(); ++i) {
      _group_enabled[i] = true;
    }
    for (std::vector<StudentID> &assigned : _group_assignments) {
      assigned.clear();
    }
    for (Participant &part : _participants) {
      part.assignment = -1;
    }
  }

  void reset() { resetWithCapacity(INITIAL_GROUP_CAPACITY); }
};

int main(int argc, const char *argv[]) {
}
