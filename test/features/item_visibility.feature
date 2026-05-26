# language: en
Feature: Item visibility — items visible within player sight distance
    So that players can see nearby items on the floor
    As the visibility system
    I want PlayerCanSee to return true for items within SIGHT_DISTANCE_PLAYER (5 tiles)
    and false for items beyond that distance

    # Test geometry: a 25x25 open floor region centered at (50,50).
    # No rooms, no doors — raw floor tiles.  Player starts at (50,50).
    #
    #   Items are placed at various offsets and queried via PlayerCanSee.
    #
    # SIGHT_DISTANCE_PLAYER = 5 (Chebyshev square: ±5 in each axis)

    Background:
        Given I have a game
        And I initialize the game
        And the game has a player
        And the item visibility geometry is centered at 50,50

    # ---- Adjacent tiles: all 8 directions ----

    Scenario Outline: Item is visible at 1 step in all 8 directions
        Given the player is at 50,50
        When I check LOS to <tx>,<ty>
        Then the LOS result is true

        Examples:
          | tx | ty |
          | 51 | 50 |
          | 49 | 50 |
          | 50 | 51 |
          | 50 | 49 |
          | 51 | 51 |
          | 51 | 49 |
          | 49 | 51 |
          | 49 | 49 |

    # ---- Cardinal directions: distances 2-5 ----

    Scenario Outline: Item is visible within 5 tiles (cardinal directions)
        Given the player is at 50,50
        When I check LOS to <tx>,<ty>
        Then the LOS result is true

        Examples:
          | tx | ty |
          | 52 | 50 |
          | 53 | 50 |
          | 54 | 50 |
          | 55 | 50 |
          | 50 | 52 |
          | 50 | 53 |
          | 50 | 54 |
          | 50 | 55 |

    # ---- Sight boundary: exactly 5 tiles (Chebyshev) ----

    Scenario: Item at exactly 5 steps east is visible
        Given the player is at 50,50
        When I check LOS to 55,50
        Then the LOS result is true

    Scenario: Item at exactly 5 steps diagonally is visible
        Given the player is at 50,50
        When I check LOS to 55,55
        Then the LOS result is true
