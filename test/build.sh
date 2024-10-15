#!/bin/sh -x
# Compile
g++ -c -o bin/AllSteps.o features/step_definitions/AllSteps.cpp -I../../JMoria/src -std=c++11 -Wno-comment -Wno-delete-non-virtual-dtor

# Link
g++ -o bin/AllSteps bin/AllSteps.o ../../JMoria/src/*.o -L/usr/local/lib -lcucumber-cpp -lc++ -lboost_program_options -lboost_regex -lboost_filesystem -lboost_system -lgtest
# gcc -o bin/AllSteps features/step_definitions/AllSteps.cpp ../../JMoria/Util.cpp ../../JMoria/Tileset.cpp -L/usr/local/lib -lcucumber-cpp -lc++ -lboost_program_options -lboost_regex -lboost_filesystem -lboost_system -lgtest -I../../JMoria -std=c++11 -Wno-comment -Wno-delete-non-virtual-dtor
# gcc -o bin/FirstSteps features/step_definitions/FirstSteps.cpp -L/usr/local/lib -lcucumber-cpp -lc++ -lboost_program_options -lboost_regex -lboost_filesystem -lboost_system -lgtest -I../../JMoria -std=c++11 -Wno-comment -Wno-delete-non-virtual-dtor
