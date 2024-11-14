# language: en
Feature: Equipment
    So that the player can equip weapons, armor, and other equipment
    As a game engine
    I want to test many things about the character equipment

    Scenario: I can equip light sources
        Given I have a Player
        Given I spawn a Torch:20
        Given the Torch:20 is not cursed
        Given the player has a Torch:20 in inventory
        Given the player equips the item 20
        Given the player has a Torch:20 in equipment at 11
        # And A torch is in the lighting equipment slot

    Scenario: Light sources light the area around me
        Given I have a Player
        Given I spawn a Torch:20
        Given the Torch:20 is not cursed
        Given the player has a Torch:20 in inventory
        Given the player equips the item 20
        Given the player has a Torch:20 in equipment at 11
        Then the area near the player is lit
        # And A torch is in the lighting equipment slot

    Scenario: Light sources have a limited duration
        Given I have a Player
        Given I spawn a Torch:20
        Given the Torch:20 is not cursed
        Given the player has a Torch:20 in inventory
        Given the player equips the item 20
        Given the player has a Torch:20 in equipment at 11
        Given the player has a nonzero lightsource
        # And A torch is in the lighting equipment slot

    Scenario: Depleted light sources do not light the area around me
        Given I have a Player
        Given the player has a zero lightsource
        Then the area near the player is not lit
        # And A torch is in the lighting equipment slot

    # Scenario: Unlit areas are a different color than lit areas
    # Scenario: Unknown areas are a different color than unlit areas
    # Scenario: a lit room lights when entered
    # Scenario: a dark room lights when walked over
    # Scenario: Scroll of Light will light a dark room
    # Scenario: Scroll of Light will light a 3x3 section of dark hallway
    # Scenario: Lanterns last longer than torches
    # Scenario: Lanterns can be refueled with Flasks of Oil
