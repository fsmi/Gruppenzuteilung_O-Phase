#include "config.h"

int getTerminalWidth() {
  struct winsize w = { };
  ioctl(0, TIOCGWINSZ, &w);
  return w.ws_col;
}

po::options_description Config::getConfigOptions() {
  po::options_description options("Further Options", getTerminalWidth());
  options.add_options()
          ("verbosity,v",
            po::value<uint32_t>(&get_mut().verbosity_level)->value_name("<int>"),
            "Output verbosity from 0 to 4 (default: 2)")
          ("rating-input-type",
            po::value<std::string>()->notifier([&](const std::string& type) {
              if (type == "mapping") {
                get_mut().rating_input_type = RatingInputType::Mapping;
              } else if (type == "ordered_list") {
                get_mut().rating_input_type = RatingInputType::OrderedList;
              } else {
                FATAL_ERROR("--rating-input-type must be `mapping` or `ordered_list`");
              }
            })->value_name("<string>"),
            "Input format for ratings:\n"
            " - mapping: mapping of group ids to priority (0 is highest)\n"
            " - ordered_list: list of group ids, where the first has the highest priority (and so on)")
          ("output-per-team",
            po::value<bool>(&get_mut().output_per_team)->value_name("<bool>"),
            "Outputs the mapping per team instead of per student. Requires that each student is member of a team (default: false).")
          ("disabled-groups-per-step,d",
            po::value<GroupID>(&get_mut().disabled_groups_per_step)->value_name("<int>"),
            "When reassigning students of specific types, determines how many groups are disabled per step. "
            "A larger value will speed up the algorithm but possibly decrease the quality (default: 3).")
          ("min-group-size",
            po::value<StudentID>(&get_mut().min_group_size)->value_name("<int>"),
            "Minimum group size. The algorithm will try to enforce that each group has at least this number "
            "of students, possibly by disabling some of the groups.")
          ("max-group-size",
            po::value<StudentID>(&get_mut().max_group_size)->value_name("<int>"),
            "Maximum allowed group size.")
          ("max-team-size",
            po::value<StudentID>(&get_mut().max_team_size)->value_name("<int>"),
            "Maximum allowed team size.")
          ("allow-default-ratings",
            po::value<bool>(&get_mut().allow_default_ratings)->value_name("<bool>"),
            "If true, the rating list of a student might be incomplete and missing entries are replaced "
            "with the worst possible rating.")
          ("capacity-buffer-factor",
            po::value<double>(&get_mut().capacity_buffer)->value_name("<double>"),
            "When disabling groups, ensures that the available capacity is at least by this factor "
            "above the number of students (higher capacity generally allows for better solution quality).")
          ("search-steps-per-value",
            po::value<int64_t>(&get_mut().search_steps_per_value)->value_name("<int>"),
            "When using local search based reassignment, determines the number of steps used by the "
            "search algorithm (default: 1000). [Note: probably shouldn't be changed]");
  // TODO: remaining options
  return options;
}

void Config::check() {
    ASSERT_WITH(get().verbosity_level <= 4,
                "--verbosity must be between 0 and 4");
    ASSERT_WITH(get().capacity_buffer > 1,
                "--capacity-buffer-factor must be > 1");
    ASSERT_WITH(get().search_steps_per_value >= 10,
                "--search-steps-per-value must be at least 10");
}
