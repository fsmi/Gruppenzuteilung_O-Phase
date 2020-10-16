#include "definitions.h"

#include <assert.h>

static const size_t NUM_RATINGS = 5;
static uint32_t RATING_VAL_TABLE[NUM_RATINGS] = {100, 120, 150, 180, 200};
static const char *RATING_NAME_TABLE[NUM_RATINGS] = {"--", "-", "O", "+", "++"};

// ####################################
// ########     Input Data     ########
// ####################################

Rating::Rating(uint32_t index) : index(index) { assert(index < NUM_RATINGS); }

uint32_t Rating::getValue() const { return RATING_VAL_TABLE[index]; }

const char *Rating::getName() const { return RATING_NAME_TABLE[index]; }

bool Rating::operator==(const Rating &other) const {
  return index == other.index;
}

bool Rating::operator!=(const Rating &other) const {
  return index != other.index;
}

GroupData::GroupData(std::string name, StudentID capacity, CourseType ct,
                     DegreeType dt)
    : name(name), capacity(capacity), course_type(ct), degree_type(dt) {}

StudentData::StudentData(std::string name, CourseType ct, DegreeType dt,
                         bool is_commuter)
    : name(name), course_type(ct), degree_type(dt), is_commuter(is_commuter) {}

TeamData::TeamData(std::string name, std::vector<StudentID> members)
    : name(name), members(std::move(members)) {}

size_t TeamData::size() const { return members.size(); }

// #########################################
// ########     Assignment Data     ########
// #########################################

bool ratingsEqual(const std::vector<Rating> &r1,
                  const std::vector<Rating> &r2) {
  if (r1.size() != r2.size()) {
    return false;
  }

  for (size_t i = 0; i < r1.size(); ++i) {
    if (r1[i] != r2[i]) {
      return false;
    }
  }
  return true;
}

Participant::Participant(uint32_t index, bool is_team)
    : index(index), is_team(is_team), assignment(-1) {}

State::State(const Input &data)
    : _data(data), _group_capacities(),
      _group_enabled(data.groups.size(), true),
      _group_assignments(data.groups.size(), std::vector<StudentID>()),
      _group_weights(data.groups.size(), 0), _participants() {
  assert(data.students.size() == data.ratings.size());
  std::vector<bool> is_in_team(data.students.size(), false);
  for (ParticipantID team_id = 0; team_id < data.teams.size(); ++team_id) {
    const TeamData &team = data.teams[team_id];
    assert(!team.members.empty());
    for (const StudentID &student : team.members) {
      assert(student < is_in_team.size() && !is_in_team[student]);
      assert(
          ratingsEqual(data.ratings[student], data.ratings[team.members[0]]));
      is_in_team[student] = true;
    }
    _participants.emplace_back(team_id, true);
  }
  for (size_t i = 0; i < data.students.size(); ++i) {
    if (!is_in_team[i]) {
      _participants.emplace_back(i, false);
    }
  }
  for (const GroupData &group : data.groups) {
    _group_capacities.push_back(group.capacity);
  }
}

const Input &State::data() const { return _data.get(); }

GroupID State::numGroups() const { return data().groups.size(); }

GroupID State::numActiveGroups() const {
  GroupID num = 0;
  for (bool is_active : _group_enabled) {
    if (is_active) {
      num++;
    }
  }
  return num;
}

StudentID State::totalActiveGroupCapacity() const {
  StudentID cap = 0;
  for (GroupID group = 0; group < numGroups(); ++group) {
    if (groupIsEnabled(group)) {
      cap += groupCapacity(group);
    }
  }
  return cap;
}

const GroupData &State::groupData(GroupID id) const {
  assert(id < data().groups.size());
  return data().groups[id];
}

StudentID State::groupCapacity(GroupID id) const {
  assert(id < data().groups.size());
  return _group_capacities[id];
}

bool State::groupIsEnabled(GroupID id) const {
  assert(id < data().groups.size());
  return _group_enabled[id];
}

const std::vector<StudentID> &State::groupAssignmentList(GroupID id) const {
  assert(id < data().groups.size());
  return _group_assignments[id];
}

StudentID State::groupSize(GroupID id) const {
  assert(id < data().groups.size());
  return _group_assignments[id].size();
}

uint32_t State::groupWeight(GroupID id) const {
  assert(id < data().groups.size());
  return _group_weights[id];
}

ParticipantID State::numParticipants() const { return _participants.size(); }

bool State::isTeam(ParticipantID id) const {
  assert(id < _participants.size());
  return _participants[id].is_team;
}

bool State::isAssigned(ParticipantID id) const {
  assert(id < _participants.size());
  return _participants[id].assignment >= 0;
}

const StudentData &State::studentData(ParticipantID id) const {
  assert(!isTeam(id));
  StudentID student_id = _participants[id].index;
  assert(student_id < data().students.size());
  return data().students[student_id];
}

const TeamData &State::teamData(ParticipantID id) const {
  assert(isTeam(id));
  uint32_t team_id = _participants[id].index;
  assert(team_id < data().teams.size());
  return data().teams[team_id];
}

GroupID State::assignment(ParticipantID id) const {
  assert(isAssigned(id));
  return static_cast<GroupID>(_participants[id].assignment);
}

const std::vector<Rating> &State::rating(ParticipantID id) const {
  StudentID student_id;
  if (isTeam(id)) {
    student_id = teamData(id).members[0];
  } else {
    student_id = _participants[id].index;
  }
  return data().ratings[student_id];
}

void State::disableGroup(GroupID id) {
  assert(id < data().groups.size());
  _group_enabled[id] = false;
}

bool State::assignParticipant(ParticipantID participant, GroupID target) {
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
    _group_weights[target] +=
        data.size() * rating(participant)[target].getValue();
  } else {
    if (groupCapacity(target) == 0) {
      return false;
    }
    _group_capacities[target]--;
    _group_assignments[target].push_back(_participants[participant].index);
    _group_weights[target] += rating(participant)[target].getValue();
  }
  _participants[participant].assignment = target;
  return true;
}

// groups are still disabled
void State::reset() {
  for (GroupID group = 0; group < numGroups(); ++group) {
    _group_capacities[group] = groupData(group).capacity;
  }
  for (std::vector<StudentID> &assigned : _group_assignments) {
    assigned.clear();
  }
  for (uint32_t &weight : _group_weights) {
    weight = 0;
  }
  for (Participant &part : _participants) {
    part.assignment = -1;
  }
}

void State::decreaseCapacity(GroupID id, StudentID val) {
  assert(id < data().groups.size());
  if (_group_capacities[id] <= val) {
    _group_capacities[id] = 0;
  } else {
    _group_capacities[id] -= val;
  }
}
