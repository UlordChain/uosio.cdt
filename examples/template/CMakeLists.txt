cmake_minimum_required(VERSION 3.5)
project(template_example VERSION 1.0.0)

find_package(uosio.cdt)

# add contract
add_contract( <contract name>, <target name>, <source files....> )
