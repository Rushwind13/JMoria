# language: en
Feature: Targeting
    So that the player can select enemies for ranged attacks
    As the targeting system
    I want to verify target selection, cycling, and validation

    Background:
        Given the dungeon is seeded
        And I have a Player
        And the game has a player

    Scenario: Target list includes visible monster
        Given I spawn a Kobold, a monster at distance 1
        When I enter targeting mode
        Then the player has a target
        And the target is a Kobold

    Scenario: Target list excludes non-visible monster
        Given I spawn a Giant Ant, a monster behind a wall
        And I spawn a Kobold, a monster at distance 2
        When I enter targeting mode
        Then the target is a Kobold

    Scenario: Target list is sorted by distance
        Given I spawn a Kobold, a monster at distance 1
        And I spawn a Giant Ant, a monster at distance 3
        When I enter targeting mode
        Then the target is a Kobold
        When I cycle to the next target
        Then the target is a Giant Ant

    Scenario: Target cycling wraps around
        Given I spawn a Kobold, a monster at distance 1
        When I enter targeting mode
        Then the target is a Kobold
        When I cycle to the next target
        Then the player has a target

    Scenario: Confirm selects target and returns to command
        Given I spawn a Kobold, a monster at distance 1
        When I enter targeting mode
        And I confirm the target
        Then the player has a target
        And the game is in command state

    Scenario: Cancel clears targeting and returns to command
        Given I spawn a Kobold, a monster at distance 1
        When I enter targeting mode
        And I cancel targeting
        Then the game is in command state

    Scenario: LOS line is drawn while targeting
        Given I spawn a Kobold, a monster at distance 2
        When I enter targeting mode
        Then a LOS line is drawn

    Scenario: LOS line is cleared after targeting exits
        Given I spawn a Kobold, a monster at distance 2
        When I enter targeting mode
        And I cancel targeting
        Then no LOS line is drawn

    Scenario: Target dies before confirm
        Given I spawn a Kobold, a monster at distance 1
        When I enter targeting mode
        And the targeted monster is removed
        And I confirm the target
        Then the player has no target

    Scenario: Monsters have unique instance IDs
        Given I spawn a Kobold, a monster at distance 1
        And I spawn a Giant Ant, a monster at distance 3
        Then the spawned monsters have different instance IDs

    Scenario: Target persists after another monster is removed
        Given I spawn a Kobold, a monster at distance 1
        And I spawn a Giant Ant, a monster at distance 3
        When I enter targeting mode
        And I cycle to the next target
        Then the target is a Giant Ant
        When I confirm the target
        And the Kobold is removed from the dungeon
        Then the player has a target
        And the target is a Giant Ant

    Scenario: Target survives retargeting after monster list mutation
        Given I spawn a Kobold, a monster at distance 1
        And I spawn a Giant Ant, a monster at distance 2
        And I spawn a Red Jelly, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And the Kobold is removed from the dungeon
        When I enter targeting mode
        Then the target is a Giant Ant
