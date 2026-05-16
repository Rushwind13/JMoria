# language: en
Feature: Monster Elemental Immunity and Weakness
    So that elemental creatures feel distinct and dangerous in a meaningful way
    As the combat system
    I want monsters to be immune to their own element and weak to its opposite

    Background:
        Given the dungeon is seeded
        And I have a Player
        And the game has a player

    Scenario: Fire monster is immune to fire — HP unchanged
        Given I spawn a Fire Elemental, a monster at distance 3
        And I record the spawned monster's HP
        When I fire a fire elemental bolt at the spawned monster
        Then the spawned monster's HP equals the recorded HP

    Scenario: Fire monster is healed by fire when injured
        Given I spawn a Fire Elemental, a monster at distance 3
        And the spawned monster is damaged to half HP
        And I record the spawned monster's HP
        When I fire a fire elemental bolt at the spawned monster
        Then the spawned monster's HP is greater than the recorded HP

    Scenario: Fire monster is weak to cold — HP reduced
        Given I spawn a Fire Elemental, a monster at distance 3
        And I record the spawned monster's HP
        When I fire a cold elemental bolt at the spawned monster
        Then the spawned monster's HP is less than the recorded HP

    Scenario: Normal monster takes fire damage
        Given I spawn a Norska, a monster at distance 3
        And I record the spawned monster's HP
        When I fire a fire elemental bolt at the spawned monster
        Then the spawned monster's HP is less than the recorded HP

    Scenario: Ice monster is immune to cold — HP unchanged
        Given I spawn a Ice Elemental, a monster at distance 3
        And I record the spawned monster's HP
        When I fire a cold elemental bolt at the spawned monster
        Then the spawned monster's HP equals the recorded HP

    Scenario: Ice monster is weak to fire — HP reduced
        Given I spawn a Ice Elemental, a monster at distance 3
        And I record the spawned monster's HP
        When I fire a fire elemental bolt at the spawned monster
        Then the spawned monster's HP is less than the recorded HP
