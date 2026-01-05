# --- Variables ---

CC = g++
CC_FLAGS = -w
TEST_CC_FLAGS_COMMON = -I../JMoria/src -std=c++14 -Wno-comment -Wno-delete-non-virtual-dtor

OS := $(shell uname -s)

ifeq ($(OS),Darwin)
# macOS specific flags (Frameworks and libc++)
# Homebrew search paths for both Intel (/usr/local) and Apple Silicon (/opt/homebrew) Macs
LOCAL_INCLUDE_PATHS = -I/usr/local/include -I/opt/homebrew/include
LOCAL_LIB_PATHS = -L/usr/local/lib -L/opt/homebrew/lib
TEST_CC_FLAGS = $(TEST_CC_FLAGS_COMMON) -framework OpenGL $(LOCAL_INCLUDE_PATHS)
LD_FLAGS = $(LOCAL_LIB_PATHS) -lSDL2 -lSDL2_image -framework OpenGL
TEST_LD_FLAGS = $(LOCAL_LIB_PATHS) -lcucumber-cpp -lboost_program_options -lboost_regex -lboost_filesystem -Wl,-force_load,/opt/homebrew/lib/libgtest.a /opt/homebrew/lib/libgtest_main.a -lpthread
else
# Linux/Other specific flags
TEST_CC_FLAGS = $(TEST_CC_FLAGS_COMMON) -I/usr/local/include
LD_FLAGS = -L/usr/local/lib -L/lib/x86_64-linux-gnu -lGL -lSDL2 -lSDL2_image
TEST_LD_FLAGS = -L/usr/local/lib -lcucumber-cpp -lgtest
endif

EXEC = jmoria
TEST_DIR = test/bin
TEST_EXEC = AllSteps
SCORE_FILE = Resources/Scores.txt
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TEST_SOURCES = test/features/step_definitions/AllSteps.cpp
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
	-rmdir $(TEST_DIR)

