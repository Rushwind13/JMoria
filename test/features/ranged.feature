# language: en
Feature: Ranged Combat
    So that the player can attack monsters from a distance
    As the ranged combat system
    I want to verify zapping wands and firing projectiles at targets

    Background:
        Given the dungeon is seeded
        And I have a Player
        And the game has a player

    Scenario: Zap wand at visible target hits monster
        Given the player has a Wand of Light in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        And the projectile completes its trajectory
        Then the ranged hit position matches the target

    Scenario: Zap with no charges shows nothing happens
        Given the player has a Wand of Light in slot a with 0 charges
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the game is in command state

    Scenario: Auto-target reuses previous target selection
        Given the player has a Wand of Light in slot a
        And I spawn a Kobold, a monster at distance 2
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the game is in ranged trajectory state

    Scenario: Projectile trajectory completes and returns to command
        Given the player has a Wand of Light in slot a
        And I spawn a Kobold, a monster at distance 2
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        And the projectile completes its trajectory
        Then the game is in command state

    Scenario: Zap non-zappable item is rejected
        Given the player has a Torch in slot a
        And I spawn a Kobold, a monster at distance 2
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the game is in command state

    Scenario: Fire non-fireable item is rejected
        Given the player has a Wand of Light in slot a
        And I spawn a Kobold, a monster at distance 2
        When I enter targeting mode
        And I confirm the target
        And I fire slot a
        Then the game is in command state
