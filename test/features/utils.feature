# language: en
Feature: Utils
    So that I can test my RNG
    As a game engine
    I want to make sure my dice rolls are working

    Scenario: Flip a coin
        Given I ask for a 0,1 result
        When I call GetRandom
        Then I get either 0 or 1

    Scenario: RNG
        Given I ask for a 1,100 result
        When I call GetRandom
        Then The result is between 1 and 100

    Scenario: RNG with floats
        Given I ask for a 1.0,10.0 float result
        When I call GetRandom as float
        Then The float result is between 1.0 and 10.0

    Scenario: Dice
        Given I ask for a 3d6 dice roll
        When I call Roll
        Then The result is between 3 and 18

    Scenario: Dice XdY
        Given I ask for a 3d6 dice roll
        When I call Roll 3d6
        Then The result is between 3 and 18