CC = g++
CC_FLAGS = -w
LD_FLAGS = -L/lib/x86_64-linux-gnu -lGL -lSDL2 -lSDL2_image
TEST_CC_FLAGS = -I../JMoria/src -std=c++14 -Wno-comment -Wno-delete-non-virtual-dtor
TEST_LD_FLAGS = -L/usr/local/lib -lcucumber-cpp -lgtest
OS := $(shell uname -s)
ifeq ($(OS),Darwin)
  LD_FLAGS = -lSDL2 -lSDL2_image -framework OpenGL
  TEST_LD_FLAGS = -L/usr/local/lib -lcucumber-cpp -lc++ -lboost_program_options -lboost_regex -lboost_filesystem -lboost_system -lgtest
endif

EXEC = jmoria
TEST_DIR = test/bin
TEST_EXEC = AllSteps
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TEST_SOURCES = test/features/step_definitions/AllSteps.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) $(LD_FLAGS) -o $(EXEC)

$(TEST_EXEC): $(TEST_DIR) $(TEST_OBJECTS) $(filter-out src/main.o, $(OBJECTS))
	$(CC) $(TEST_OBJECTS) $(filter-out src/main.o, $(OBJECTS)) $(TEST_LD_FLAGS) $(LD_FLAGS) -o $(TEST_DIR)/$(TEST_EXEC)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

test: $(TEST_EXEC)

%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $(TEST_CC_FLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS) $(TEST_DIR)/$(TEST_EXEC) $(TEST_OBJECTS)
	rmdir $(TEST_DIR)
