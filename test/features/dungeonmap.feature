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
        When I call LightArea for the room
        Then The JRect 5,5,10,10 is now filled with 0
        Then The JRect 4,4,11,11 is now lit
        And The rect is in world

    Scenario: I can create room that stays in-world
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,5 to fill
        When I call FillArea for a room
        When I call LightArea for the room
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world

    Scenario: Rooms that are not in-world remain filled with walls
        Given I have a DungeonMap
        Given I have a JRect 0,0,5,5 to fill
        When I call FillArea for a room
        When I call LightArea for the room
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
        Then The JRect 7,2,11,4 is not lit
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world

    Scenario: FillArea does not care about intersections
        Given I have a DungeonMap
        Given There is already a room at 1,1,5,5 in the dungeon
        Given I have a JRect 3,3,10,3 to fill
        When I call FillArea for a hallway east
        Then The JRect 6,3,10,3 is now filled with 0
        Then The JRect 7,2,11,4 is not lit
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world

    Scenario: I can create Hallway that stays in-world
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,1 to fill
        When I call FillArea for a hallway east
        Then The JRect 1,1,5,1 is now filled with 0
        Then The JRect 0,0,6,2 is not lit
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
        When I call LightArea for the room
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world
    Scenario: I can create S rooms
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,5 to fill
        When I call FillArea for a room S
        When I call LightArea for the room
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world
    Scenario: I can create E rooms
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,5 to fill
        When I call FillArea for a room E
        When I call LightArea for the room
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world
    Scenario: I can create W rooms
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,5 to fill
        When I call FillArea for a room W
        When I call LightArea for the room
        Then The JRect 1,1,5,5 is now filled with 0
        Then The JRect 0,0,6,6 is now lit
        And The rect is in world

    Scenario: I can create N hallway
        Given I have a DungeonMap
        Given I have a JRect 1,1,1,5 to fill
        When I call FillArea for a hallway north
        Then The JRect 1,1,1,5 is now filled with 0
        Then The JRect 0,0,2,6 is not lit
        And The rect is in world

    Scenario: I can create S hallway
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,1 to fill
        When I call FillArea for a hallway south
        Then The JRect 1,1,5,1 is now filled with 0
        Then The JRect 0,0,6,2 is not lit
        And The rect is in world

    Scenario: I can create W hallway
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,1 to fill
        When I call FillArea for a hallway west
        Then The JRect 1,1,5,1 is now filled with 0
        Then The JRect 0,0,6,2 is not lit
        And The rect is in world

    Scenario: I can create E hallway
        Given I have a DungeonMap
        Given I have a JRect 1,1,5,1 to fill
        When I call FillArea for a hallway east
        Then The JRect 1,1,5,1 is now filled with 0
        Then The JRect 0,0,6,2 is not lit
        And The rect is in world

    Scenario: I can create rooms and hallways using steps
        Given I have a DungeonMap
        Given I have a room create step
        Given I have a E hallway create step
        When I create a S hallway create step
        Then the S hallway meets the E hallway

    Scenario: Deterministic dungeon generation with seed
        Given I create a dungeon at depth 1 with seed 12345
        Then The dungeon has seed 12345

    Scenario: Same seed produces same dungeon layout
        Given I create a dungeon at depth 1 with seed 99999
        Then The dungeon matches another dungeon with the same seed

    Scenario: Stress test - 100 consecutive generations with same seed are identical
        Given I create 100 consecutive dungeons at depth 1 with seed 11111
        Then All 100 dungeons are identical
        And All have the same room count
        And All have the same hallway count

    Scenario: Out-of-world tile attempts are rejected
        Given I have a DungeonMap
        Given I have a JRect 98,98,102,102 to fill
        When I call FillArea for a room
        Then The out-of-world portion remains as walls

    Scenario: Dungeon can be exported to fixture file
        Given I create a dungeon at depth 1 with seed 42
        When I export the dungeon to a fixture file
        Then The fixture file exists

    Scenario: Dungeon fixture can be imported
        Given I create a dungeon at depth 2 with seed 55
        When I export the dungeon to a fixture file
        And I import the fixture file into a new dungeon
        Then The imported dungeon has the same structure

    Scenario: GetRoomRect rejects degenerate rectangles
        Given I create a dungeon at depth 1 with seed 777
        When I call GetRoomRect with position at 50,50 direction north
        Then The returned rect has positive width and height

    Scenario: GetHallRect rejects out-of-bounds geometry
        Given I create a dungeon at depth 1 with seed 888
        When I call GetHallRect with position at 98,50 direction east
        Then GetHallRect returns success or properly handles boundary
