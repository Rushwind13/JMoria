# JMoria Test Suite

## Architecture

JMoria uses **Cucumber-CPP** with **Google Test (gtest)** for BDD-style testing.

### Components

| Component | Purpose |
|-----------|---------|
| `features/*.feature` | Cucumber feature files (Gherkin syntax) |
| `features/step_definitions/*.cpp` | Step definition implementations |
| `features/step_definitions/TestContext.hpp` | Shared test context and includes |
| `features/step_definitions/AllSteps.cpp` | Master include file for all steps |
| `bin/AllSteps` | Compiled test binary |

### Test Flow

1. `AllSteps` binary starts and listens on a socket (wire protocol)
2. Cucumber (Ruby) parses `.feature` files
3. Cucumber connects to AllSteps and requests step execution
4. gtest assertions verify expected behavior

## Running Tests

```bash
./runtests.sh
```

Or manually:
```bash
cd ..
make test       # Builds test/bin/AllSteps
cd test
./bin/AllSteps &
bundle exec cucumber features/
```

## Adding New Tests

### 1. Create a feature file

```gherkin
# features/myfeature.feature
Feature: My Feature
    So that I can test something
    As a game engine
    I want to verify behavior

    Scenario: Basic test
        Given I have some precondition
        When I perform an action
        Then I expect a result
```

### 2. Create step definitions

```cpp
// features/step_definitions/MyFeatureSteps.cpp
#include "TestContext.hpp"
using cucumber::ScenarioScope;

GIVEN("^I have some precondition$") {
    ScenarioScope<TestCtx> context;
    // Setup code
}

WHEN("^I perform an action$") {
    ScenarioScope<TestCtx> context;
    // Action code
}

THEN("^I expect a result$") {
    ScenarioScope<TestCtx> context;
    EXPECT_TRUE(/* assertion */);
}
```

### 3. Add include to AllSteps.cpp

```cpp
#include "MyFeatureSteps.cpp"
```

### 4. Add context if needed

If you need shared state between steps, add fields to `TestCtx` struct in `TestContext.hpp`.

## Test Coverage

| Feature File | Tests |
|--------------|-------|
| `airender.feature` | RLE encoding, IsAllWalls |
| `brains.feature` | AI brain state management |
| `dungeonmap.feature` | Dungeon generation, rooms, hallways |
| `equipment.feature` | Equipping items, cursed items |
| `game.feature` | Game initialization |
| `items.feature` | Item creation and properties |
| `light.feature` | Lighting system |
| `monsters.feature` | Monster creation and properties |
| `rects.feature` | JRect geometry |
| `stringtable.feature` | String table loading |
| `textentry.feature` | Text/config parsing |
| `tilesets.feature` | Tileset loading |
| `utils.feature` | RNG, dice rolls |
| `vectors.feature` | JVector/JIVector operations |

## Dependencies (Ubuntu)

```bash
# Cucumber-CPP requirements
sudo apt-get install --no-install-recommends \
    clang-15 cmake g++ git libasio-dev libboost-test-dev \
    libgl1-mesa-dev libtclap-dev ninja-build \
    nlohmann-json3-dev qt6-base-dev

# JMoria requirements
sudo apt-get install libsdl2-dev libsdl2-image-dev cucumber

# Fix OpenGL include path
sudo ln -s /usr/include/GL /usr/include/OpenGL

# Ruby gems
bundle install
```

## Dependencies (macOS)

```bash
brew install boost@1.60
# Then build gtest and cucumber-cpp from source
```

## Notes

- Tests run without a display (SDL2 still required for compilation)
- Feature files use Gherkin syntax
- Step definitions use regex patterns to match feature file text
- `ScenarioScope<TestCtx>` provides shared context across steps in a scenario
