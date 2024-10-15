# language: en
Feature: Tilesets
    So that I can display my game
    As a game engine
    I want to make sure my tilesets are working

    Scenario: Load a tileset
        Given I load the player tileset
        When I call Load
        Then I get a successful result
        And The tileset is loaded