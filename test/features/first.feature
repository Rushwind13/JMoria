# language: en
Feature: First
    So that positions will work properly
    As a game engine
    I want to test many things about vector math

    Scenario: On Wednesdays we wear pink
        Given It is Wednesday
        Then Wearing Pink

    Scenario: I can create positions
        Given I have a JVector 1,2 in the data
        Then It is a vector
        Then It is not a position

    Scenario: add two vectors
        Given I have a JVector 0,1 in the data
        And I have a second JVector 1,0 in the data
        When I press add_vector
        Then the result should be 1,1 a JVector
        And It is a vector