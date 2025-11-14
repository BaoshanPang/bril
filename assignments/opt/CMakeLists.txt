cmake_minimum_required(VERSION 3.10)
project(opt_project)

# Create the executable from opt.cpp
add_executable(opt opt.cpp cfg.cpp)

# Custom command: run the test
add_custom_target(test
    COMMAND bril2json < ${CMAKE_CURRENT_SOURCE_DIR}/../../examples/test/tdce/simple.bril | ./opt | brili -p
    DEPENDS opt
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
