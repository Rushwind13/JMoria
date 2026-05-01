# language: en
Feature: Elemental Damage
    So that dungeon exploration carries meaningful risk
    As the combat system
    I want elemental attacks to potentially destroy vulnerable inventory items

    Background:
        Given the dungeon is seeded
        And I have a Player
        And the game has a player

    Scenario: Cold damage can shatter potions over repeated hits
        Given I have 10 Potion of Minor Healing in my inventory
        When the player takes 100 cold damage hits
        Then the player's Potion of Minor Healing count is less than 10

    Scenario: Fire damage can burn scrolls over repeated hits
        Given I have 10 Scroll of Light in my inventory
        When the player takes 100 fire damage hits
        Then the player's Scroll of Light count is less than 10

    Scenario: Acid damage can dissolve wands over repeated hits
        Given I have 10 Wand of Light in my inventory
        When the player takes 100 acid damage hits
        Then the player's Wand of Light count is less than 10

    Scenario: Lightning damage can blast wands over repeated hits
        Given I have 10 Wand of Light in my inventory
        When the player takes 100 lightning damage hits
        Then the player's Wand of Light count is less than 10

    Scenario: Swords are not destroyed by fire
        Given I have 3 Dagger in my inventory
        When the player takes 100 fire damage hits
        Then the player's Dagger count is 3

    Scenario: Physical damage does not destroy inventory items
        Given I have 10 Scroll of Light in my inventory
        When the player takes 100 physical damage hits
        Then the player's Scroll of Light count is 10
