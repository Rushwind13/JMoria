# language: en
Feature: Game
    So that I can run my game
    As a game engine
    I want to make sure my game is working

    Scenario: Player Seek works
        Given I have a game
        And I initialize the game
        And the game has a player
        And I spawn a monster with SEEK
        When I update the monster's brain
        Then the game initalized successfully
        And the monster spawned successfully
        And the monster wants to move toward the player