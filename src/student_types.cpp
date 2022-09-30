#include "student_types.h"

#include <boost/algorithm/string.hpp>

#include "config.h"

std::unordered_map<std::string, std::tuple<FilterFn, u_int32_t, std::string>> initializeTypeToFilterMapping() {
  std::unordered_map<std::string, std::tuple<FilterFn, u_int32_t, std::string>> mapping;

  // the key is used for retrieving the filter when parsing the types file
  mapping.insert({"inf", {[](const StudentData &data) noexcept {
    return data.course_type == CourseType::Info;
  }, 0, "Info"}});
  mapping.insert({"mat", {[](const StudentData &data) noexcept {
    return data.course_type == CourseType::Mathe;
  }, 1, "Mathe"}});
  mapping.insert({"leh", {[](const StudentData &data) noexcept {
    return data.course_type == CourseType::Lehramt;
  }, 2, "Lehramt"}});
  mapping.insert({"bac", {[](const StudentData &data) noexcept {
    return data.degree_type == DegreeType::Bachelor;
  }, 3, "Bachelor"}});
  mapping.insert({"mas", {[](const StudentData &data) noexcept {
    return data.degree_type == DegreeType::Master;
  }, 4, "Master"}});
  mapping.insert({"ers", {[](const StudentData &data) noexcept {
    return data.semester == Semester::Ersti;
  }, 5, "Ersti"}});
  mapping.insert({"dri", {[](const StudentData &data) noexcept {
    return data.semester == Semester::Dritti;
  }, 6, "Dritti"}});

  return mapping;
}

std::vector<std::pair<Filter, StudentID>> parseTypesFile(std::ifstream& file) {
  auto mapping = initializeTypeToFilterMapping();

  std::vector<std::pair<Filter, StudentID>> result;
  std::string line;
  size_t line_index = 0;
  while (std::getline(file, line)) {
    if (line != "") {
      std::vector<std::string> words;
      boost::split(words, line, [](char c) {return c == ' ';});
      ASSERT_WITH(words.size() > 1, "types, line " << line_index << ": each line needs to have the form 'TYPENAME+ LIMIT'");

      std::vector<std::pair<FilterFn, u_int32_t>> filters;
      std::string combined_name;
      for (size_t i = 0; i + 1 < words.size(); ++i) {
        std::string current = words[i];
        ASSERT_WITH(current.size() >= 3,
                    "types, line " << line_index << ": invalid name of student type: " << words[i]);
        current.resize(3);
        boost::algorithm::to_lower(current);
        ASSERT_WITH(mapping.find(current) != mapping.end(),
                    "types, line " << line_index << ": invalid name of student type: " << words[i]);

        auto [filter_fn, filter_id, filter_name] = mapping[current];
        filters.emplace_back(filter_fn, filter_id);
        if (!combined_name.empty()) {
          combined_name += "-";
        }
        combined_name += filter_name;
      }
      try {
        int limit = std::stoi(words.back());
        result.emplace_back(Filter(std::move(filters), std::move(combined_name)), limit);
      } catch (const std::invalid_argument& e) {
        FATAL_ERROR("types, line " << line_index << ": last word must be an integer (limit)");
      }
    }
    line_index++;
  }
  return result;
}
