# language: en
Feature: Equipment
    So that the player can equip weapons, armor, and other equipment
    As a game engine
    I want to test many things about the character equipment
    
    Scenario: I can equip things
        Given I have a Player
        Given I spawn a Dagger
        Given the Dagger is not cursed
        Given the player has a Dagger in inventory
        Given the player equips the Dagger
        Then The Dagger is in equipment at 0
        Then The Dagger is not in inventory at -1
        # And A dagger is in the primary weapon equipment slot
    
    Scenario: Equipment goes to the proper slot
        Given I have a Player
        Given I spawn a Dagger
        Given the Dagger is not cursed
        Given the player has a Dagger in inventory
        Given the player equips the Dagger
        Given I spawn a Small Wooden Shield
        Given the Small Wooden Shield is not cursed
        # Given I spawn a Battle Axe
        Given the player has a Small Wooden Shield in inventory
        Given the player equips the Small Wooden Shield
        Then The Small Wooden Shield is not in inventory at -1
        And The Dagger is in equipment at 0
        And The Small Wooden Shield is in equipment at 1
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

    Scenario: Primary and secondary weapon can be swapped
        Given I have a Player
        Given A Dagger is in the primary weapon equipment slot
        Given A Pickaxe is in the secondary weapon equipment slot
        When the player swaps weapons
        Given A Pickaxe is in the primary weapon equipment slot
        Given A Dagger is in the secondary weapon equipment slot

    Scenario: Main and offhand weapons can be swapped together
        Given I have a Player
        Given A Battle Axe is in the primary weapon equipment slot
        Given A Steel Shield is in the offhand weapon equipment slot
        Given A Long Sword is in the secondary weapon equipment slot
        Given A Small Steel Shield is in the secondary offhand weapon equipment slot
        When the player swaps weapons
        Then A Long Sword should be in the primary weapon equipment slot
        Then A Small Steel Shield should be in the offhand weapon equipment slot
        Then A Battle Axe should be in the secondary weapon equipment slot
        Then A Steel Shield should be in the secondary offhand weapon equipment slot

    Scenario: Sword and shield swap with two-handed bow
        Given I have a Player
        Given A Long Sword is in the primary weapon equipment slot
        Given A Steel Shield is in the offhand weapon equipment slot
        Given A Long Bow is in the secondary weapon equipment slot
        When the player swaps weapons
        Then A Long Bow should be in the primary weapon equipment slot
        Then A Long Sword should be in the secondary weapon equipment slot
        Then A Steel Shield should be in the secondary offhand weapon equipment slot

    Scenario: Sword and shield swap with one-handed dagger
        Given I have a Player
        Given A Long Sword is in the primary weapon equipment slot
        Given A Steel Shield is in the offhand weapon equipment slot
        Given A Dagger is in the secondary weapon equipment slot
        When the player swaps weapons
        Then A Dagger should be in the primary weapon equipment slot
        Then A Long Sword should be in the secondary weapon equipment slot
        Then A Steel Shield should be in the secondary offhand weapon equipment slot

    Scenario: Two-handed quarterstaff swaps with two-handed bow
        Given I have a Player
        Given A Quarter Staff is in the primary weapon equipment slot
        Given A Long Bow is in the secondary weapon equipment slot
        When the player swaps weapons
        Given A Long Bow is in the primary weapon equipment slot
        Given A Quarter Staff is in the secondary weapon equipment slot

    Scenario: Shield goes to inventory when wielding two-handed weapon
        Given I have a Player
        Given A Long Sword is in the primary weapon equipment slot
        Given A Steel Shield is in the offhand weapon equipment slot
        Given I spawn a Long Bow
        Given the Long Bow is not cursed
        Given the player has a Long Bow in inventory
        When the player equips the Long Bow
        Then The Long Bow is in equipment at 0
        # Then The Steel Shield is in inventory at -1

    Scenario: Equipment can be taken off
        Given I have a Player
        Given I spawn a Dagger
        Given the Dagger is not cursed
        Given the player has a Dagger in inventory
        Given the player equips the Dagger
        # Given A dagger is in the primary weapon equipment slot
        Given the player has a Dagger in equipment at 0
        When the player takes off the Dagger at 0
        Then The Dagger is not in equipment at 0
        Then The Dagger is in inventory at -1

    Scenario: Cursed Equipment can be wielded
        Given I have a Player
        Given I spawn a Dagger
        Given the Dagger is cursed
        Given the player has a Dagger in inventory
        Given the player equips the Dagger
        Then The Dagger is in equipment at 0
        Then The Dagger is not in inventory at -1
        # And A dagger is in the primary weapon equipment slot

    Scenario: Cursed Equipment cannot be taken off
        Given I have a Player
        Given I spawn a Dagger
        Given the Dagger is cursed
        Given the player has a Dagger in inventory
        Given the player equips the Dagger
        # Given A dagger is in the primary weapon equipment slot
        Given the player has a Dagger in equipment at 0
        When the player takes off the Dagger at 0
        Then The Dagger is in equipment at 0
        Then The Dagger is not in inventory at -1

    # @skip
    Scenario: Cursed Equipment can be uncursed with scroll of remove cruse
        Given I have a Player
        Given I spawn a Dagger
        Given the Dagger is cursed
        Given the player has a Dagger in inventory
        Given the player equips the Dagger
        Given I spawn a Scroll of Remove Curse
        Given the player has a Scroll of Remove Curse in inventory
        # Given A dagger is in the primary weapon equipment slot
        Given the player has a Dagger in equipment at 0
        When the player reads the scroll in inventory at 0
        When the player applies the pending effect to equipment at 0
        Then the equipped Dagger at 0 is not cursed

    Scenario: Cursed scroll of remove curse curses an equipped item
        Given I have a Player
        Given I spawn a Dagger
        Given the player has a Dagger in inventory
        Given the player equips the Dagger
        Given I spawn a Scroll of Remove Curse
        Given the Scroll of Remove Curse is cursed
        Given the player has a Scroll of Remove Curse in inventory
        Given the player has a Dagger in equipment at 0
        When the player reads the scroll in inventory at 0
        When the player applies the pending effect to equipment at 0
        Then the equipped Dagger at 0 is cursed
        When the player takes off the Dagger at 0
        Then the player has a Dagger in equipment at 0
        Then the equipped Dagger at 0 is cursed

    Scenario: New Equipment replaces old equipment
        Given I have a Player
        Given I spawn a Dagger
        Given the Dagger is not cursed
        Given the player has a Dagger in inventory
        Given the player equips the Dagger
        Given I spawn a Long Sword
        Given the player has a Long Sword in inventory
        Given the player equips the Long Sword
        Then The Long Sword is not in inventory at -1
        Then The Dagger is in inventory at -1
        And The Long Sword is in equipment at 0

    Scenario: New Equipment does not replace cursed equipment
        Given I have a Player
        Given I spawn a Dagger
        Given the Dagger is cursed
        Given the player has a Dagger in inventory
        Given the player equips the Dagger
        Given I spawn a Long Sword
        Given the player has a Long Sword in inventory
        Given the player equips the Long Sword
        Then The Long Sword is in inventory at -1
        And The Dagger is in equipment at 0

    Scenario: Unidentified cursed equipment does not display label cursed
        Given I have a Player
        Given I spawn a Dagger
        Given the Dagger is not identified
        When I display equipment
        Then the Dagger is not labeled as cursed 

