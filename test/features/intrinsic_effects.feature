# language: en
# Regression tests for issue #309: resist-type intrinsic effects (e.g. Resist Fear,
# Resist Poison) were incorrectly treated as debuffs instead of resistances.
Feature: Intrinsic Resist Effects
    So that resist potions grant resistance instead of the associated debuff
    As a game engine
    I want Resist Fear and Resist Poison effects to be tracked as resistances

    Scenario: Potion of Courage grants fear resistance, not the afraid debuff
        Given I have a Player
        And I spawn a Potion of Courage
        When I programmatically quaff the spawned item
        Then the player has an active fear resistance
        And the player does not have the afraid debuff

    Scenario: Potion of Slow Poison grants poison resistance, not the poisoned debuff
        Given I have a Player
        And I spawn a Potion of Slow Poison
        When I programmatically quaff the spawned item
        Then the player has an active poison resistance
        And the player does not have the poisoned debuff
