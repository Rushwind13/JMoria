# language: en
Feature: Equipment
    So that the player can equip weapons, armor, and other equipment
    As a game engine
    I want to test many things about the character equipment

    Scenario: I can equip things
        Given I have a Player
        Given the player has a dagger in inventory
        When the player equips the item
        Then The dagger is not in inventory
        And The dagger is in equipment
        And A dagger is in the primary weapon equipment slot

    Scenario: Equipment goes to the proper slot
        Given I have a Player
        Given the player has a shield in inventory
        When the player equips the item
        Then The shield is not in inventory
        And The shield is in equipment
        And A shield is in the shield equipment slot


    Scenario: Some Equipment has two proper slots
        Given I have a Player
        Given the player has a ring in inventory
        Given the player is wearing a ring
        When the player equips the item
        Then The ring is not in inventory
        And The ring is in equipment
        And A ring is in the second ring equipment slot

    Scenario: Primary and secondary weapon can be swapped
        Given I have a Player
        Given A dagger is in the primary weapon equipment slot
        Given A pickaxe is in the secondary weapon equipment slot
        When the player swaps weapons
        Given A pickaxe is in the primary weapon equipment slot
        Given A dagger is in the secondary weapon equipment slot

    Scenario: Equipment can be taken off
        Given I have a Player
        Given A dagger is in the primary weapon equipment slot
        When the player takes off the dagger
        Then The dagger is in inventory
        And The dagger is not in equipment
    Scenario: Cursed Equipment can be wielded
        Given I have a Player
        Given the player has a dagger in inventory
        Given the dagger is cursed
        When the player equips the item
        Then The dagger is not in inventory
        And The dagger is in equipment
        And A dagger is in the primary weapon equipment slot
    Scenario: Cursed Equipment cannot be taken off
        Given I have a Player
        Given A dagger is in the primary weapon equipment slot
        Given the dagger is cursed
        When the player takes off the dagger
        Then The dagger is not in inventory
        And The dagger is in equipment
    Scenario: New Equipment replaces old equipment
        Given I have a Player
        Given A dagger is in the primary weapon equipment slot
        Given the player has a pickaxe in inventory
        When the player wields the pickaxe
        Then The dagger is in inventory
        And The pickaxe is in equipment
    Scenario: New Equipment does not replace cursed equipment
        Given I have a Player
        Given A dagger is in the primary weapon equipment slot
        Given the dagger is cursed
        Given the player has a pickaxe in inventory
        When the player wields the pickaxe
        Then The dagger is in equipment
        And The pickaxe is in inventory


    