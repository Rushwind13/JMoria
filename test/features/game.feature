# language: en
Feature: Game
    So that I can run my game
    As a game engine
    I want to make sure my game is working

    Scenario: Game creation is possible
        Given I have a game
        And I initialize the game
        Then the game initalized successfully

    Scenario: Game creation is idempotent
        Given I have a game
        And I have a game
        And I have a game
        And I initialize the game
        Then the game initalized successfully

    Scenario: Game termination works
        Given I have a game
        And I initialize the game
        When I terminate the game
        Then the game terminates successfully
    @skip
    Scenario: Player Seek works
        Given I have a game
        And I initialize the game
        And the game has a player
        And I spawn a Red Dragon:20, a monster with SEEK
        And I update the monster's brain
        Then the game initalized successfully
        And the Red Dragon spawned successfully
        And the Red Dragon wants to move toward the player
    @skip
    Scenario: AI state changes work
        Given I have a game
        And I initialize the game
        And the game has a player
        And I spawn a Red Dragon:20, a monster with SEEK
        And I update the monster's brain
        When I update the monster's brain again
        And the Red Dragon moves toward the player
