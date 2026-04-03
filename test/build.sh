#!/bin/sh -x
# Compile (C++14 required for gtest)
g++ -c -o bin/AllSteps.o features/step_definitions/AllSteps.cpp -I../../JMoria/src -std=c++14 -Wno-comment -Wno-delete-non-virtual-dtor

# Link (exclude main.o to avoid duplicate globals when linking test runner)
OBJFILES=$(ls ../../JMoria/src/*.o | grep -v '/main.o' || true)
g++ -o bin/AllSteps bin/AllSteps.o $OBJFILES -L/usr/local/lib -lcucumber-cpp -lboost_program_options -lboost_regex -lboost_filesystem -lboost_system -lgtest -lSDL2 -lSDL2_image -lGL
# gcc -o bin/AllSteps features/step_definitions/AllSteps.cpp ../../JMoria/Util.cpp ../../JMoria/Tileset.cpp -L/usr/local/lib -lcucumber-cpp -lc++ -lboost_program_options -lboost_regex -lboost_filesystem -lboost_system -lgtest -I../../JMoria -std=c++11 -Wno-comment -Wno-delete-non-virtual-dtor
# gcc -o bin/FirstSteps features/step_definitions/FirstSteps.cpp -L/usr/local/lib -lcucumber-cpp -lc++ -lboost_program_options -lboost_regex -lboost_filesystem -lboost_system -lgtest -I../../JMoria -std=c++11 -Wno-comment -Wno-delete-non-virtual-dtor
