# language: en
Feature: Equipment
    So that the player can equip weapons, armor, and other equipment
    As a game engine
    I want to test many things about the character equipment
    
    # @skip
    Scenario: I can equip things
        Given I have a Player
        Given I spawn a Dagger:27
        Given the Dagger:27 is not cursed
        Given the player has a Dagger:27 in inventory
        Given the player equips the item 27
        Then The Dagger:27 is in equipment at 0
        Then The Dagger:27 is not in inventory at -1
        # And A dagger is in the primary weapon equipment slot
    
    # @skip
    Scenario: Equipment goes to the proper slot
        Given I have a Player
        Given I spawn a Dagger:27
        Given the Dagger:27 is not cursed
        Given the player has a Dagger:27 in inventory
        Given the player equips the item 27
        Given I spawn a Small Wooden Shield:26
        Given the Small Wooden Shield:26 is not cursed
        # Given I spawn a Battle Axe:42
        Given the player has a Small Wooden Shield:26 in inventory
        Given the player equips the item 26
        Then The Small Wooden Shield:26 is not in inventory at -1
        And The Dagger:27 is in equipment at 0
        And The Small Wooden Shield:26 is in equipment at 1
    #     And A shield is in the shield equipment slot

    # @skip
    # Scenario: Some Equipment has two proper slots
    #     Given I have a Player
    #     Given the player has a ring in inventory
    #     Given the player is wearing a ring
    #     When the player equips the item
    #     Then The ring is not in inventory
    #     And The ring is in equipment
    #     And A ring is in the second ring equipment slot

    # Scenario: Primary and secondary weapon can be swapped
    #     Given I have a Player
    #     Given A dagger is in the primary weapon equipment slot
    #     Given A pickaxe is in the secondary weapon equipment slot
    #     When the player swaps weapons
    #     Given A pickaxe is in the primary weapon equipment slot
    #     Given A dagger is in the secondary weapon equipment slot

    # @skip
    Scenario: Equipment can be taken off
        Given I have a Player
        Given I spawn a Dagger:27
        Given the Dagger:27 is not cursed
        Given the player has a Dagger:27 in inventory
        Given the player equips the item 27
        # Given A dagger is in the primary weapon equipment slot
        Given the player has a Dagger:27 in equipment at 0
        When the player takes off the item 27 at 0
        Then The Dagger:27 is not in equipment at 0
        Then The Dagger:27 is in inventory at -1

    # @skip
    Scenario: Cursed Equipment can be wielded
        Given I have a Player
        Given I spawn a Dagger:27
        Given the Dagger:27 is cursed
        Given the player has a Dagger:27 in inventory
        Given the player equips the item 27
        Then The Dagger:27 is in equipment at 0
        Then The Dagger:27 is not in inventory at -1
        # And A dagger is in the primary weapon equipment slot

    # @skip
    Scenario: Cursed Equipment cannot be taken off
        Given I have a Player
        Given I spawn a Dagger:27
        Given the Dagger:27 is cursed
        Given the player has a Dagger:27 in inventory
        Given the player equips the item 27
        # Given A dagger is in the primary weapon equipment slot
        Given the player has a Dagger:27 in equipment at 0
        When the player takes off the item 27 at 0
        Then The Dagger:27 is in equipment at 0
        Then The Dagger:27 is not in inventory at -1

    # @skip
    Scenario: Cursed Equipment can be uncursed with scroll of remove cruse
        Given I have a Player
        Given I spawn a Dagger:27
        Given the Dagger:27 is cursed
        Given the player has a Dagger:27 in inventory
        Given the player equips the item 27
        Given I spawn a Scroll of Remove Curse:30
        Given the player has a Scroll of Remove Curse:30 in inventory
        # Given A dagger is in the primary weapon equipment slot
        Given the player has a Dagger:27 in equipment at 0
        When the player reads the scroll in inventory at 0
        Then the equipped Dagger:27 at 0 is not cursed

    # @skip
    Scenario: Cursed scroll of remove curse curses an equipped item
        Given I have a Player
        Given I spawn a Dagger:27
        Given the player has a Dagger:27 in inventory
        Given the player equips the item 27
        Given I spawn a Scroll of Remove Curse:30
        Given the Scroll of Remove Curse:30 is cursed
        Given the player has a Scroll of Remove Curse:30 in inventory
        Given the player has a Dagger:27 in equipment at 0
        When the player reads the scroll in inventory at 0
        Then the equipped Dagger:27 at 0 is cursed
        When the player takes off the item 27 at 0
        Then the player has a Dagger:27 in equipment at 0
        Then the equipped Dagger:27 at 0 is cursed

    # @skip
    Scenario: New Equipment replaces old equipment
        Given I have a Player
        Given I spawn a Dagger:27
        Given the Dagger:27 is not cursed
        Given the player has a Dagger:27 in inventory
        Given the player equips the item 27
        Given I spawn a Long Sword:28
        Given the player has a Long Sword:28 in inventory
        Given the player equips the item 28
        Then The Long Sword:28 is not in inventory at -1
        Then The Dagger:27 is in inventory at -1
        And The Long Sword:28 is in equipment at 0

    # @skip
    Scenario: New Equipment does not replace cursed equipment
        Given I have a Player
        Given I spawn a Dagger:27
        Given the Dagger:27 is cursed
        Given the player has a Dagger:27 in inventory
        Given the player equips the item 27
        Given I spawn a Long Sword:28
        Given the player has a Long Sword:28 in inventory
        Given the player equips the item 28
        Then The Long Sword:28 is in inventory at -1
        And The Dagger:27 is in equipment at 0
