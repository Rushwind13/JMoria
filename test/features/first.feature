# language: en
Feature: First
    So that positions will work properly
    As a game engine
    I want to test many things about JVector math

    Scenario: On Wednesdays we wear pink
        Given It is Wednesday
        Then Wearing Pink

    Scenario: I can create positions
        Given I have a JVector 1,2 in the data
        Then It is a JVector
        Then It is not a position

    Scenario: add two JVectors
        Given I have a JVector 0,1 in the data
        And I have a second JVector 1,0 in the data
        When I press add_JVector
        Then the result should be 1,1 a JVector
        And It is a JVector
    
    Scenario: subtract two JVectors
        Given I have a JVector 3,2 in the data
        And I have a second JVector 5,6 in the data
        When I press subtract_JVector
        Then the result should be -2,-4 a JVector
        And It is a JVector

    Scenario: subtract JVector from the zero JVector
        Given I have a JVector 0,0 in the data
        And I have a second JVector 1,-2 in the data
        When I press subtract_JVector
        Then the result should be -1,2 a JVector
        And It is a JVector

    Scenario: negate a JVector
        Given I have a JVector 1,-2 in the data
        When I press negate_JVector
        Then the result should be -1,2 a JVector
        And It is a JVector

    Scenario: multiply JVector by a scalar
        Given I have a JVector 1,-2 in the data
        When I press scale_JVector 3.5
        Then the result should be 3.5,-7 a JVector
        And It is a JVector