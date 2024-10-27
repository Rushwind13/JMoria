#!/bin/bash

# Check if the first argument is "--build"
if [[ "$1" == "--build" ]]; then
  # If it is, build the project and return
  cd ..
  make clean
  make test
  cd -
  shift
fi

# Set the executable path
EXE_UNDER_TEST="./bin/AllSteps"

# Find and kill any existing instances of the executable
pid=$(ps -fe | grep "$EXE_UNDER_TEST" | grep -v grep | awk '{print $2}')
if [[ -n "$pid" ]]; then
  echo "found old pid running... killing"
  kill "$pid"
fi

# Start the executable in the background
"$EXE_UNDER_TEST" &

# Sleep to allow the executable to start
sleep 1

# Set the test to run based on the first argument
TEST_TO_RUN="*"
if [[ -n "$1" ]]; then
  TEST_TO_RUN="$1"
fi

# Run the Cucumber tests
cucumber features/$TEST_TO_RUN.feature
