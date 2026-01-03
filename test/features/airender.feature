# language: en
Feature: AI Render
    So that my AI logging produces correct output
    As a game engine
    I want to make sure my RLE encoding and character lookups work

    Scenario: RLE encodes repeated characters
        Given I have a row "######"
        When I call RLEEncodeRow
        Then The encoded result is "#6"

    Scenario: RLE encodes mixed characters
        Given I have a row "###..."
        When I call RLEEncodeRow
        Then The encoded result is "#3.3"

    Scenario: RLE encodes single characters without count
        Given I have a row "#.@"
        When I call RLEEncodeRow
        Then The encoded result is "#.@"

    Scenario: RLE encodes complex pattern
        Given I have a row ".#########@...#######"
        When I call RLEEncodeRow
        Then The encoded result is ".#9@.3#7"

    # Note: Quote and backslash escaping is tested implicitly in the implementation
    # Cucumber has trouble parsing escaped characters in feature files

    Scenario: IsAllWalls returns true for all walls
        Given I have a row "######"
        When I call IsAllWalls
        Then IsAllWalls returns true

    Scenario: IsAllWalls returns false for mixed
        Given I have a row "###.##"
        When I call IsAllWalls
        Then IsAllWalls returns false

    Scenario: IsAllWalls returns false for floor
        Given I have a row "......"
        When I call IsAllWalls
        Then IsAllWalls returns false

    # Note: Monster/Item GetChar tests require full game context
    # and are covered by integration tests with AI logging enabled
