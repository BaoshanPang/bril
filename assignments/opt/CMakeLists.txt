cmake_minimum_required(VERSION 3.10)
project(opt_project)

# Create the executable from opt.cpp
add_executable(opt opt.cpp)

# Custom command: run the test
add_custom_target(test
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/../L2/L2.json | ./opt | brili
    DEPENDS opt
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running test: cat ../L2/L2.json | ./opt | brili"
)
