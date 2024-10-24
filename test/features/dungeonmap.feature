# language: en
Feature: Dungeon Creation
    So that the player and monsters have a map to play in
    As a game engine
    I want to test many things about Dungeon Creation

    Scenario: I can create a world filled with walls
        Given I have a DungeonMap
        Given I have a JRect 5,5,10,10 to fill
        Then The JRect 5,5,10,10 is now filled with 1
        And The rect is in world

    Scenario: I can create rooms filled with floor
        Given I have a DungeonMap
        Given I have a JRect 5,5,10,10 to fill
        When I call FillArea for a room
        Then The JRect 5,5,10,10 is now filled with 0
        Then The JRect 4,4,11,11 is now lit
        And The rect is in world

    Scenario: I can create room that stays in-world
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,5 to fill
        When I call FillArea for a room
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world

    Scenario: Rooms that are not in-world remain filled with walls
        Given I have a DungeonMap
        Given I have a JRect 0,0,5,5 to fill
        When I call FillArea for a room
        Then The JRect 0,0,5,5 is now filled with 1
        And The rect is in world

    Scenario: Rooms can already exist
        Given I have a DungeonMap
        Given There is already a room at 1,1,5,5 in the dungeon
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world

    Scenario: Can build hallway east from existing room
        Given I have a DungeonMap
        Given There is already a room at 1,1,5,5 in the dungeon
        Given I have a JRect 6,3,10,3 to fill
        When I call FillArea for a hallway east
        Then The JRect 6,3,10,3 is now filled with 0
        Then The JRect 5,2,11,4 is now lit
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world

    Scenario: FillArea does not care about intersections
        Given I have a DungeonMap
        Given There is already a room at 1,1,5,5 in the dungeon
        Given I have a JRect 3,3,10,3 to fill
        When I call FillArea for a hallway east
        Then The JRect 6,3,10,3 is now filled with 0
        Then The JRect 5,2,11,4 is now lit
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world

    Scenario: I can create Hallway that stays in-world
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,1 to fill
        When I call FillArea for a hallway east
        Then The JRect 1,1,5,1 is now filled with 0
        Then The JRect 0,0,6,2 is now lit
        And The rect is in world

    Scenario: Hallways that are not in-world remain filled with walls
        Given I have a DungeonMap
        Given I have a JRect 0,0,5,0 to fill
        When I call FillArea for a hallway east
        Then The JRect 0,0,5,0 is now filled with 1
        And The rect is in world

    Scenario: I can create N rooms
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,5 to fill
        When I call FillArea for a room N
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world
    Scenario: I can create S rooms
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,5 to fill
        When I call FillArea for a room S
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world
    Scenario: I can create E rooms
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,5 to fill
        When I call FillArea for a room E
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world
    Scenario: I can create W rooms
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,5 to fill
        When I call FillArea for a room W
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world

    Scenario: I can create N hallway
        Given I have a DungeonMap
        Given I have a JRect 1,1,1,5 to fill
        When I call FillArea for a hallway north
        Then The JRect 1,1,1,5 is now filled with 0
        Then The JRect 0,0,2,6 is now lit
        And The rect is in world

    Scenario: I can create S hallway
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,1 to fill
        When I call FillArea for a hallway south
        Then The JRect 1,1,5,1 is now filled with 0
        Then The JRect 0,0,6,2 is now lit
        And The rect is in world

    Scenario: I can create W hallway
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,1 to fill
        When I call FillArea for a hallway west
        Then The JRect 1,1,5,1 is now filled with 0
        Then The JRect 0,0,6,2 is now lit
        And The rect is in world

    Scenario: I can create E hallway
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,1 to fill
        When I call FillArea for a hallway east
        Then The JRect 1,1,5,1 is now filled with 0
        Then The JRect 0,0,6,2 is now lit
        And The rect is in world

    Scenario: I can create a dungeon map step
        Given I have a DungeonMap
        Given I have an origin 1,1
        Given I have a DungeonCreationStep
        Then I'll have a CDungeonCreationStep

    Scenario: I can CheckArea for a Dungeon Creation step
        Given I have a DungeonMap
        Given I have an origin 1,1
        Given I have a DungeonCreationStep
        When I call CheckArea
        Then I'll have a valid area

    Scenario: I can get a north room rect
        Given I have a DungeonMap
        Given I have a direction north
        When I call get room rect
        Then I'll have a valid N room rect
