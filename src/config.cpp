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
            "Output verbosity ranging from 0 [no output] to 5 [debug output] (default: 3)")
          ("random-seed,s",
            po::value<uint32_t>(&get_mut().random_seed)->value_name("<int>"),
            "Seed for pseudo-randomness used in the algorithm.")
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
          ("input-per-team",
            po::value<bool>(&get_mut().input_per_team)->value_name("<bool>"),
            "Input ratings per team instead of per student. Requires that each student is member of a team (default: false).")
          ("disabled-groups-per-step,d",
            po::value<GroupID>(&get_mut().disabled_groups_per_step)->value_name("<int>"),
            "When reassigning students of specific types, determines how many groups are disabled per step. "
            "A larger value will speed up the algorithm but possibly decrease the quality (default: 3).")
          ("type-specific-assignment-threshold",
            po::value<uint32_t>(&get_mut().type_specific_assignment_threshold)->value_name("<int>"),
            "When reassigning students of specific types, do not reassign students if this leads to a rating "
            "below this threshold. Set to 0 to disable (default: 0).")
          ("group-disable-threshold",
            po::value<StudentID>(&get_mut().group_disable_threshold)->value_name("<int>"),
            "Absolute minimum for group size. The algorithm will try to enforce that each group has at least "
            "this number of students, possibly by disabling some of the groups.")
          ("max-team-size",
            po::value<StudentID>(&get_mut().max_team_size)->value_name("<int>"),
            "Maximum allowed team size.")
          ("use-min-group-sizes",
            po::value<bool>(&get_mut().use_min_group_sizes)->value_name("<bool>"),
            "If true, try to distribute students more evenly based on the provided minimum group sizes.")
          ("allow-min-group-size-default",
            po::value<bool>(&get_mut().allow_min_group_size_default)->value_name("<bool>"),
            "Set reasonable default value for min group size if none is provided.")
          ("min-group-size-effect",
            po::value<StudentID>(&get_mut().min_group_size_effect)->value_name("<int>"),
            "Effect strength of min group size. Between 1 (small effect) and "
            "5 (effectively overrides student preferences).")
          ("allow-default-ratings",
            po::value<bool>(&get_mut().allow_default_ratings)->value_name("<bool>"),
            "If true, the rating list of a student might be incomplete and missing entries are replaced "
            "with the worst possible rating.")
          ("capacity-buffer-factor",
            po::value<double>(&get_mut().capacity_buffer)->value_name("<double>"),
            "When disabling groups, ensures that the available capacity is at least by this factor "
            "above the number of students (higher capacity generally allows for better solution quality).")
          ("edge-sparsification",
            po::value<bool>(&get_mut().edge_sparsification)->value_name("<bool>"),
            "If true, pseudo-randomly sparsifies the edges in the created graph to reduce memory usage.");
  // TODO: remaining options
  return options;
}

void Config::check() {
    ASSERT_WITH(get().verbosity_level <= 5,
                "--verbosity must be between 0 and 5");
    ASSERT_WITH(get().capacity_buffer > 1,
                "--capacity-buffer-factor must be > 1");
    ASSERT_WITH(get().min_group_size_effect > 0 && get().min_group_size_effect <= 5,
                "--min-group-size-effect must be between 1 and 5");
    if (!get().use_min_group_sizes) {
      get_mut().allow_min_group_size_default = true;
    }
}
