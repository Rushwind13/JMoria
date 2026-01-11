# --- Variables ---

CC = g++
CC_FLAGS = -w
TEST_CC_FLAGS_COMMON = -I../JMoria/src -std=c++17 -Wno-comment -Wno-delete-non-virtual-dtor -DGTEST_HAS_PTHREAD=1
LD_FLAGS_LINUX = -L/lib/x86_64-linux-gnu -lGL -lSDL2 -lSDL2_image
TEST_LD_FLAGS_LINUX = -L/usr/local/lib -lcucumber-cpp -lboost_program_options -lboost_regex -lboost_filesystem -lgtest -lgtest_main

OS := $(shell uname -s)

ifeq ($(OS),Darwin)
# Standard Homebrew search paths for headers (MODIFIED: Removed /opt/homebrew)
LOCAL_INCLUDE_PATHS = -I/usr/local/include -I /opt/homebrew/include -I/opt/homebrew/Cellar/googletest/1.17.0/include
# Standard Homebrew search paths for libraries (MODIFIED: Removed /opt/homebrew)
LOCAL_LIB_PATHS = -L/usr/local/lib -L /opt/homebrew/lib -L/opt/homebrew/opt/boost/lib -L/opt/homebrew/Cellar/googletest/1.17.0/lib
# macOS specific flags (Frameworks and libc++)
TEST_CC_FLAGS = $(TEST_CC_FLAGS_COMMON) -framework OpenGL $(LOCAL_INCLUDE_PATHS)
# NEW: Add LOCAL_LIB_PATHS to the main application linker flags
LD_FLAGS = $(LOCAL_LIB_PATHS) -lSDL2 -lSDL2_image -framework OpenGL
# Update TEST_LD_FLAGS to use LOCAL_LIB_PATHS for robustness
TEST_LD_FLAGS = $(LOCAL_LIB_PATHS) -lcucumber-cpp -lboost_program_options -lboost_regex -lboost_filesystem /opt/homebrew/Cellar/googletest/1.17.0/lib/libgtest.a /opt/homebrew/Cellar/googletest/1.17.0/lib/libgtest_main.a
else
# Linux/Other specific flags
TEST_CC_FLAGS = $(TEST_CC_FLAGS_COMMON)
LD_FLAGS = $(LD_FLAGS_LINUX)
TEST_LD_FLAGS = $(TEST_LD_FLAGS_LINUX)
endif

# Optional: Enable dungeon generation diagnostics
# To enable: make DUNGEN_DEBUG=1
ifdef DUNGEN_DEBUG
  CC_FLAGS += -DDUNGEN_DEBUG
  TEST_CC_FLAGS += -DDUNGEN_DEBUG
endif

EXEC = jmoria
TEST_DIR = test/bin
TEST_EXEC = AllSteps
SCORE_FILE = Resources/Scores.txt
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TEST_SOURCES = test/features/step_definitions/AllSteps.cpp test/features/step_definitions/gtest_shim.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

$(EXEC): $(OBJECTS) $(SCORE_FILE)
	$(CC) $(OBJECTS) $(LD_FLAGS) -o $(EXEC)

$(TEST_EXEC): $(TEST_DIR) $(TEST_OBJECTS) $(filter-out src/main.o, $(OBJECTS))
	$(CC) $(TEST_OBJECTS) $(filter-out src/main.o, $(OBJECTS)) $(TEST_LD_FLAGS) $(LD_FLAGS) -o $(TEST_DIR)/$(TEST_EXEC)

$(SCORE_FILE):
	touch $(SCORE_FILE)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

test: $(TEST_EXEC)

%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $(TEST_CC_FLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS) $(TEST_DIR)/$(TEST_EXEC) $(TEST_OBJECTS)
	rm -rf $(TEST_DIR)

