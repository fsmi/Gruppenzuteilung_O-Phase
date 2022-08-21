#include "config.h"

po::options_description Config::getConfigOptions() {
  po::options_description options("Further Options");
  options.add_options()
          ("verbosity,v",
            po::value<uint32_t>(&get_mut().verbosity_level)->value_name("<int>"),
            "Output verbosity from 0 to 3 (default: 1)");
  return options;
}
