# language: en
Feature: Targeting
    So that the player can select enemies for ranged attacks
    As the targeting system
    I want to verify target selection, cycling, and validation

    Scenario: Target list includes visible monster
        Given I have a Player
        And I spawn a Kobold:14, a monster with SEEK, near the player
        When I enter targeting mode
        Then the player has a target
        And the target is a Kobold

    Scenario: Target list excludes non-visible monster
        Given I have a Player
        And I spawn a Kobold:14, a monster behind a wall
        When I enter targeting mode
        Then targeting mode exits with no targets

    Scenario: Target list is sorted by distance
        Given I have a Player
        And I spawn a Kobold:14, a monster at distance 2
        And I spawn a Giant Ant:31, a monster at distance 4
        When I enter targeting mode
        Then the target is a Kobold
        When I cycle to the next target
        Then the target is a Giant Ant

    Scenario: Target cycling wraps around
        Given I have a Player
        And I spawn a Kobold:14, a monster at distance 2
        When I enter targeting mode
        Then the target is a Kobold
        When I cycle to the next target
        Then the target is a Kobold

    Scenario: Confirm selects target and returns to command
        Given I have a Player
        And I spawn a Kobold:14, a monster with SEEK, near the player
        When I enter targeting mode
        And I confirm the target
        Then the player has a target
        And the game is in command state

    Scenario: Cancel clears targeting and returns to command
        Given I have a Player
        And I spawn a Kobold:14, a monster with SEEK, near the player
        When I enter targeting mode
        And I cancel targeting
        Then the game is in command state

    Scenario: LOS line is drawn while targeting
        Given I have a Player
        And I spawn a Kobold:14, a monster with SEEK, near the player
        When I enter targeting mode
        Then a LOS line is drawn

    Scenario: LOS line is cleared after targeting exits
        Given I have a Player
        And I spawn a Kobold:14, a monster with SEEK, near the player
        When I enter targeting mode
        And I cancel targeting
        Then no LOS line is drawn

    Scenario: Target dies before confirm
        Given I have a Player
        And I spawn a Kobold:14, a monster with SEEK, near the player
        When I enter targeting mode
        And the targeted monster is removed
        And I confirm the target
        Then the player has no target
