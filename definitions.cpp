#include "definitions.h"

#include <algorithm>
#include <assert.h>

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

GroupData::GroupData(std::string id, std::string name, StudentID capacity, CourseType ct)
    : id(id), name(name), capacity(capacity), course_type(ct) {}

StudentData::StudentData(std::string id, std::string name, CourseType ct, DegreeType dt, Semester s)
    : id(id), name(name), course_type(ct), degree_type(dt), semester(s) {}

TeamData::TeamData(std::string id, std::vector<StudentID> members)
    : id(id), members(std::move(members)) {}

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
    : _data(data),
      _group_states(data.groups.size()),
      _group_assignments(data.groups.size(),
                         std::vector<std::pair<StudentID, ParticipantID>>()),
      _participants() {
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
  for (size_t i = 0; i < data.groups.size(); ++i) {
    _group_states[i].capacity = data.groups[i].capacity;
  }
}

const Input &State::data() const { return _data.get(); }

GroupID State::numGroups() const { return data().groups.size(); }

GroupID State::numActiveGroups() const {
  GroupID num = 0;
  for (GroupState state : _group_states) {
    if (state.enabled) {
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
  return _group_states[id].capacity;
}

bool State::groupIsEnabled(GroupID id) const {
  assert(id < data().groups.size());
  return _group_states[id].enabled;
}

const std::vector<std::pair<StudentID, ParticipantID>> &
State::groupAssignmentList(GroupID id) const {
  assert(id < data().groups.size());
  return _group_assignments[id];
}

StudentID State::groupSize(GroupID id) const {
  assert(id < data().groups.size());
  return _group_assignments[id].size();
}

uint32_t State::groupWeight(GroupID id) const {
  assert(id < data().groups.size());
  return _group_states[id].weight;
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
  _group_states[id].enabled = false;
}

void State::addFilterToGroup(GroupID id, std::function<bool(const StudentData&)> filter) {
  assert(id < data().groups.size());
  _group_states[id].participant_filters.push_back(filter);
}

bool State::studentIsExludedFromGroup(StudentID student, GroupID group) const {
  assert(group < data().groups.size());
  const StudentData& s_data = data().students[student];
  for (auto filter : _group_states[group].participant_filters) {
    if (filter(s_data)) {
      return true;
    }
  }
  return false;
}

bool State::isExludedFromGroup(ParticipantID participant, GroupID group) const {
  assert(participant < _participants.size());
  assert(group < data().groups.size());

  if (isTeam(participant)) {
    for (StudentID student : teamData(participant).members) {
      if (studentIsExludedFromGroup(student, group)) {
        return true;
      }
    }
    return false;
  } else {
    return studentIsExludedFromGroup(_participants[participant].index, group);
  }
}

bool State::assignParticipant(ParticipantID participant, GroupID target) {
  assert(!isAssigned(participant));
  assert(target < data().groups.size());

  if (isTeam(participant)) {
    const TeamData &data = teamData(participant);
    if (data.size() > groupCapacity(target)) {
      return false;
    }
    _group_states[target].capacity -= data.size();
    for (StudentID id : data.members) {
      _group_assignments[target].push_back(std::make_pair(id, participant));
    }
    _group_states[target].weight +=
        data.size() * rating(participant)[target].getValue();
  } else {
    if (groupCapacity(target) == 0) {
      return false;
    }
    _group_states[target].capacity--;
    _group_assignments[target].push_back(
        std::make_pair(_participants[participant].index, participant));
    _group_states[target].weight += rating(participant)[target].getValue();
  }
  _participants[participant].assignment = target;
  return true;
}

void State::unassignParticipant(ParticipantID participant, GroupID group) {
  assert(isAssigned(participant));
  assert(assignment(participant) == group);

  std::vector<std::pair<StudentID, ParticipantID>> &assign_list =
      _group_assignments[group];
  while (true) {
    auto to_remove = std::find_if(
        assign_list.begin(), assign_list.end(),
        [&](const auto &pair) { return pair.second == participant; });
    if (to_remove != assign_list.end()) {
      _group_states[group].capacity++;
      _group_states[group].weight += rating(participant)[group].getValue();
      assign_list.erase(to_remove);
    } else {
      break;
    }
  }
  _participants[participant].assignment = -1;
}

// groups are still disabled and keep their filters
void State::reset() {
  for (GroupID group = 0; group < numGroups(); ++group) {
    GroupState& state = _group_states[group];
    state.capacity = groupData(group).capacity;
    state.weight = 0;
  }
  for (auto &assigned : _group_assignments) {
    assigned.clear();
  }
  for (Participant &part : _participants) {
    part.assignment = -1;
  }
}

void State::decreaseCapacity(GroupID id, int32_t val) {
  assert(id < data().groups.size());
  StudentID& capacity = _group_states[id].capacity;
  if (static_cast<int32_t>(capacity) <= val) {
    capacity = 0;
  } else {
    capacity -= val;
  }
}
