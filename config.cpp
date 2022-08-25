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
