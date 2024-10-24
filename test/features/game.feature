# language: en
Feature: Game
    So that I can run my game
    As a game engine
    I want to make sure my game is working

    Scenario: lookup strings
        Given I have a game
        And I initialize the game
        And the game has a player
        When I spawn a monster with SEEK
        Then the game initalized successfully
        And the monster spawned successfully