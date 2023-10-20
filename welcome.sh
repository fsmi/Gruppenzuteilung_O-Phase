#!/bin/sh

# print a welcome message with multiple lines
cat << EOF


# To rerun the 2022 assignment, run the following command:
GroupAssignment -v 5 -i /source/test_data/data_2022.json -o output.json -c /source/config/config_2022 -t /source/config/types_2022

# To rerun the 2022 assignment, but with an enforced min group size, run the following command:
GroupAssignment -v 5 -i /source/test_data/data_2022_min.json -o output.json -c /source/config/config_2022_min -t /source/config/types_2022

# To rerun the 2022 assignment, but with the default of half the groups capacity set as the min group size, run the following command:
GroupAssignment -v 5 -i /source/test_data/data_2022.json -o output.json -c /source/config/config_2022_min -t /source/config/types_2022 --allow-min-group-size-default yes

# Note that the min group size has been set to the .json input file and may significantly affect the output.
# Also note that an additional parameter 'min-group-size-effect' is used to controll the effect of the min group size on the final assignment.

EOF