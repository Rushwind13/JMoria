# language: en
Feature: Item LOS — items do not leak through closed doors
    So that players cannot see items through walls or closed doors
    As the line-of-sight system
    I want PlayerCanSee to return false when the direct path is blocked

    # Test geometry (set up by "the LOS geometry" step):
    #
    #   col:  44 45 46 47 48 49 50 51 52 53 54 55 56
    #   y=19:  .  .  .  .  .  .  .  .  .  .  .  .  .
    #   y=20:  .  .  .  .  .  .  !  .  .  .  .  .  .   <- item at (50,20)
    #   y=21:  .  .  .  .  .  .  +  .  .  .  .  .  .   <- closed door at (50,21)
    #   y=22:  .  .  .  .  .  .  .  .  .  .  .  .  .
    #   y=23:  .  .  .  .  .  .  .  .  .  .  .  .  .
    #   y=24:  .  .  @  @  @  .  .  .  @  @  @  .  .   <- 6 bug positions (rows 24-25)
    #   y=25:  .  .  @  @  @  .  .  .  @  @  @  .  .
    #
    # The 12 positions marked @ are the near-diagonal angles where the Bresenham
    # line stops before reaching the door tile (50,21), making the item appear visible.

    Background:
        Given I have a game
        And I initialize the game
        And the game has a player
        And the LOS geometry has item at 50,20 and door at 50,21

    Scenario Outline: Item behind closed door is invisible from near-diagonal angles
        Given the player is at <px>,<py>
        When I check LOS to 50,20
        Then the LOS result is false

        Examples:
          | px | py |
          | 48 | 24 |
          | 47 | 24 |
          | 46 | 24 |
          | 48 | 25 |
          | 47 | 25 |
          | 46 | 25 |
          | 52 | 24 |
          | 53 | 24 |
          | 54 | 24 |
          | 52 | 25 |
          | 53 | 25 |
          | 54 | 25 |

    Scenario: Item behind closed door is invisible from directly south
        Given the player is at 50,24
        When I check LOS to 50,20
        Then the LOS result is false

    Scenario: Item is visible when door is removed
        Given the player is at 50,24
        And the door at 50,21 is removed
        When I check LOS to 50,20
        Then the LOS result is true
