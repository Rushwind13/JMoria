# language: en
Feature: Dungeon Connectivity
    So that dungeons are fully explorable
    As a game engine
    I want to ensure all rooms and hallways are reachable

  Scenario: Disconnected rooms are detected
    Given I have a DungeonMap
    And There is already a room at 10,10,20,20 in the dungeon
    And There is already a room at 30,30,40,40 in the dungeon
    When I validate dungeon connectivity
    Then Not all tiles are reachable

  Scenario: Single room dungeon is fully connected
    Given I have a DungeonMap
    And There is already a room at 10,10,20,20 in the dungeon
    When I validate dungeon connectivity
    Then All tiles are reachable
    And The reachable tile count equals the total walkable tile count
