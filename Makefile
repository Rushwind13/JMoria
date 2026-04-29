# --- Variables ---

CC = g++
CC_FLAGS = -w

# Build mode: ascii, opengl, or both (default)
# Set via: make ascii, make opengl, or make (both)
RENDER_MODE ?= both

ifeq ($(RENDER_MODE),ascii)
  RENDER_DEFINES = -DRENDER_ASCII
else ifeq ($(RENDER_MODE),opengl)
  RENDER_DEFINES = -DRENDER_OPENGL
else
  RENDER_DEFINES =
endif

COMMON_CC_FLAGS = -I../JMoria/src -std=c++17 -Wno-comment -Wno-delete-non-virtual-dtor
TEST_CC_FLAGS_EXTRA = -DGTEST_HAS_PTHREAD=1
LD_FLAGS_LINUX = -L/lib/x86_64-linux-gnu -lGL -lSDL2 -lSDL2_image -lncurses
TEST_LD_FLAGS_LINUX = -L/usr/local/lib -lcucumber-cpp -lboost_program_options -lboost_regex -lboost_filesystem -lgtest -lgtest_main

OS := $(shell uname -s)

ifeq ($(OS),Darwin)
# Standard Homebrew search paths for headers
LOCAL_INCLUDE_PATHS = -I/usr/local/include -I /opt/homebrew/include -I/opt/homebrew/Cellar/googletest/1.17.0/include
# Standard Homebrew search paths for libraries
LOCAL_LIB_PATHS = -L/usr/local/lib -L /opt/homebrew/lib -L/opt/homebrew/opt/boost/lib -L/opt/homebrew/Cellar/googletest/1.17.0/lib
# Game compile flags (no OpenGL framework for ASCII-only)
ifeq ($(RENDER_MODE),ascii)
  GAME_CC_FLAGS = $(COMMON_CC_FLAGS) $(LOCAL_INCLUDE_PATHS)
else
  GAME_CC_FLAGS = $(COMMON_CC_FLAGS) -framework OpenGL $(LOCAL_INCLUDE_PATHS)
endif
TEST_CC_FLAGS = $(TEST_CC_FLAGS_EXTRA)
# Linker flags per build mode
ifeq ($(RENDER_MODE),ascii)
  LD_FLAGS = $(LOCAL_LIB_PATHS) -lncurses
else ifeq ($(RENDER_MODE),opengl)
  LD_FLAGS = $(LOCAL_LIB_PATHS) -lSDL2 -lSDL2_image -framework OpenGL
else
  LD_FLAGS = $(LOCAL_LIB_PATHS) -lSDL2 -lSDL2_image -lncurses -framework OpenGL
endif
# Update TEST_LD_FLAGS to use LOCAL_LIB_PATHS for robustness
ifeq ($(RENDER_MODE),ascii)
  TEST_LD_FLAGS = $(LOCAL_LIB_PATHS) -lcucumber-cpp -lboost_program_options -lboost_regex -lboost_filesystem /opt/homebrew/Cellar/googletest/1.17.0/lib/libgtest.a /opt/homebrew/Cellar/googletest/1.17.0/lib/libgtest_main.a -lncurses
else ifeq ($(RENDER_MODE),opengl)
  TEST_LD_FLAGS = $(LOCAL_LIB_PATHS) -lcucumber-cpp -lboost_program_options -lboost_regex -lboost_filesystem /opt/homebrew/Cellar/googletest/1.17.0/lib/libgtest.a /opt/homebrew/Cellar/googletest/1.17.0/lib/libgtest_main.a -lSDL2 -lSDL2_image -framework OpenGL
else
  TEST_LD_FLAGS = $(LOCAL_LIB_PATHS) -lcucumber-cpp -lboost_program_options -lboost_regex -lboost_filesystem /opt/homebrew/Cellar/googletest/1.17.0/lib/libgtest.a /opt/homebrew/Cellar/googletest/1.17.0/lib/libgtest_main.a -lSDL2 -lSDL2_image -lncurses -framework OpenGL
endif
else
# Linux/Other specific flags
GAME_CC_FLAGS = $(COMMON_CC_FLAGS)
TEST_CC_FLAGS = $(TEST_CC_FLAGS_EXTRA)
ifeq ($(RENDER_MODE),ascii)
  LD_FLAGS = -lncurses
else ifeq ($(RENDER_MODE),opengl)
  LD_FLAGS = -L/lib/x86_64-linux-gnu -lGL -lSDL2 -lSDL2_image
else
  LD_FLAGS = $(LD_FLAGS_LINUX)
endif
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

# Build mode targets
ascii:
	$(MAKE) RENDER_MODE=ascii

opengl:
	$(MAKE) RENDER_MODE=opengl

ascii-test:
	$(MAKE) RENDER_MODE=ascii test

.PHONY: ascii opengl ascii-test test clean

$(TEST_EXEC): $(TEST_DIR) $(TEST_OBJECTS) $(filter-out src/main.o, $(OBJECTS))
	$(CC) $(TEST_OBJECTS) $(filter-out src/main.o, $(OBJECTS)) $(TEST_LD_FLAGS) -o $(TEST_DIR)/$(TEST_EXEC)

$(SCORE_FILE):
	touch $(SCORE_FILE)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

test: $(TEST_EXEC)

src/%.o: src/%.cpp
	$(CC) -c $(CC_FLAGS) $(RENDER_DEFINES) $(GAME_CC_FLAGS) $< -o $@

test/%.o: test/%.cpp
	$(CC) -c $(CC_FLAGS) $(RENDER_DEFINES) $(GAME_CC_FLAGS) $(TEST_CC_FLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS) $(TEST_DIR)/$(TEST_EXEC) $(TEST_OBJECTS)
	rm -rf $(TEST_DIR)
	find . -name '* [0-9].*' -not -path './.git/*' -delete

