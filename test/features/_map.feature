Feature: Map
    So that key/value stores will work properly
    As a Game engine
    I want to test lots of things about JMap

    Scenario: I can insert into an empty Map
        Given I have a Map
        And I insert entry <13,pizza> into the Map
        When I Get entry 13 from the Map
        Then Entry 13 is pizza
    Scenario: I can insert a smaller value into a Map
        Given I have a Map
        And I insert entry <13,pizza> into the Map
        And I insert entry <7,xyzzy> into the Map
        When I Get entry 7 from the Map
        Then Entry 7 is xyzzy
    Scenario: I can insert a larger value into a Map
        Given I have a Map
        And I insert entry <13,pizza> into the Map
        And I insert entry <42,dolphin> into the Map
        When I Get entry 42 from the Map
        Then Entry 42 is dolphin
    Scenario: I can insert an intermediate value into a Map
        Given I have a Map
        And I insert entry <13,pizza> into the Map
        And I insert entry <7,xyzzy> into the Map
        And I insert entry <10,coffee> into the Map
        When I Get entry 7 from the Map
        Then Entry 7 is xyzzy
        And the next index is <10>
    Scenario: Smaller values insert to prev
        Given I have a Map
        And I insert entry <13,pizza> into the Map
        And I insert entry <7,xyzzy> into the Map
        When I Get entry 13 from the Map
        Then the prev index is <7>
    Scenario: Larger values insert to next
        Given I have a Map
        And I insert entry <13,pizza> into the Map
        And I insert entry <42,dolphin> into the Map
        When I Get entry 13 from the Map
        Then the next index is <42>