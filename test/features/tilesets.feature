# language: en
Feature: Tilesets
    So that I can display my game
    As a game engine
    I want to make sure my tilesets are working

    Scenario: Load a tileset
        Given I load the player tileset
        When I call Load
        Then I get a successful result
        And The player tileset is loaded

    Scenario: Load a different tileset
        Given I load the messages tileset
        When I call Load
        Then I get a successful result
        And The messages tileset is loaded

    Scenario: GetTile
        Given I load the player tileset
        When I call Load
        And I ask for tile 50
        Then I get tile 2,3 back
        And The player tileset is loaded

    Scenario: GetTile 2
        Given I load the player tileset
        When I call Load
        And I ask for tile 150
        Then I get tile 6,9 back
        And The player tileset is loaded
