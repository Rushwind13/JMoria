# language: en
Feature: Game
    So that I can run my game
    As a game engine
    I want to make sure my game is working

    Scenario: lookup strings
        Given I have a game
        When I initialize the game
        Then the game initalized successfully