# language: en
Feature: Speed System
    So that fast monsters are threatening and speed items matter
    As a game engine
    I want to test the action economy speed mechanic

    Scenario: Player starts at base speed
        Given I have a Player
        Then the player speed is 1.0

    Scenario: Equipping Boots of Speed increases player speed
        Given I have a Player
        Given I spawn a Boots of Speed
        Given the Boots of Speed is not cursed
        Given the player has a Boots of Speed in inventory
        Given the player equips the Boots of Speed
        Then the player speed is greater than 1.0

    Scenario: Removing Boots of Speed restores player speed
        Given I have a Player
        Given I spawn a Boots of Speed
        Given the Boots of Speed is not cursed
        Given the player has a Boots of Speed in inventory
        Given the player equips the Boots of Speed
        Given the player has a Boots of Speed in equipment at 8
        When the player takes off the Boots of Speed at 8
        Then the player speed is 1.0

    Scenario: Drinking a Potion of Speed increases player speed
        Given I have a Player
        Given I spawn a Potion of Speed
        Given the player has a Potion of Speed in inventory
        When the player uses the Potion of Speed
        Then the player speed is greater than 1.0

    Scenario: Equipping a Ring of Speed increases player speed
        Given I have a Player
        Given I spawn a Ring of Speed
        Given the Ring of Speed is not cursed
        Given the player has a Ring of Speed in inventory
        Given the player equips the Ring of Speed
        Then the player speed is greater than 1.0
