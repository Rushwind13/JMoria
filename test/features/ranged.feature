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
    @skip
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

    Scenario: Wand of Light beam lights tiles along its path
        Given the player has a Wand of Light in slot a
        And I spawn a Kobold, a monster at distance 5
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        And the projectile completes its trajectory
        Then the tiles between the player and the target are lit

    Scenario: Wand of Light beam lights room boundary wall tiles
        Given the player has a Wand of Light in slot a
        And I spawn a Kobold, a monster at distance 3
        And a wall exists one tile above the beam path
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        And the projectile completes its trajectory
        Then the wall tile adjacent to the beam is lit

    Scenario: Wand of Light beam sets the room LIT flag
        Given the player has a Wand of Light in slot a
        And I spawn a Kobold, a monster at distance 5
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        And the projectile completes its trajectory
        Then the player's current room is lit

    # Arrow ground behavior tests (P2 — Arrow ground behavior)
    @skip
    Scenario: Arrow drops to ground when fire trajectory ends (miss)
        Given the player has a Short Bow in slot a
        And the player has Flight Arrow in slot a with 1 count
        And the player equips the Short Bow
        When I enter targeting mode for ranged attack with no monster
        And I fire slot a
        And the projectile completes its trajectory
        Then an arrow exists on the ground at the trajectory end
        And the player has no Flight Arrow in inventory
        
    @skip
    Scenario: Arrows of same type stack on ground
        Given the player has a Short Bow in slot a
        And the player has Flight Arrow in slot a with 1 count
        And the player equips the Short Bow
        And an arrow exists on the ground at distance 10 with count 2
        When I enter targeting mode for ranged attack with no monster
        And I fire slot a  
        And the projectile completes its trajectory
        Then the ground arrow count is 3
        And the player has no Flight Arrow in inventory

    Scenario: Arrow landing location has space to scatter
        Given the player has a Short Bow in slot a
        And the player has Flight Arrow in slot a with 1 count
        And the player equips the Short Bow
        And an item exists on the ground at distance 5
        When I enter targeting mode for ranged attack with no monster
        And I fire slot a
        And the projectile completes its trajectory
        Then an arrow exists on the ground adjacent to the trajectory end

    # Bow+arrow combat math tests (P1 — Bow+arrow combined combat math)

    @skip
    Scenario: Fire arrow at visible monster with successful hit
        Given the player has a Short Bow in slot a
        And the player has Flight Arrow in slot b with 10 count
        And the player equips the Short Bow
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        Then the player's target is set
        When I fire slot b
        And the projectile completes its trajectory
        Then the ranged hit position matches the target

    @skip
    Scenario: Fire arrow uses ammo from inventory
        Given the player has a Short Bow in slot a
        And the player has Flight Arrow in slot b with 10 count
        And the player equips the Short Bow
        And I spawn a Kobold, a monster at distance 3
        And I store the current Flight Arrow inventory count
        When I enter targeting mode
        And I confirm the target
        And I fire slot b
        And the projectile completes its trajectory
        Then the ranged hit position matches the target

    @skip
    Scenario: Bow with to-hit bonus increases hit chance
        Given the player has a Short Bow with +3 to-hit bonus in slot a
        And the player has Flight Arrow in slot b with 10 count
        And the player equips the Short Bow
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I fire slot b
        And the projectile completes its trajectory
        Then the ranged hit position matches the target

    @skip
    Scenario: Arrow with damage bonus adds to hit damage
        Given the player has a Short Bow in slot a
        And the player has Flight Arrow with +2 damage bonus in slot b
        And the player equips the Short Bow
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I fire slot b
        And the projectile completes its trajectory
        Then the ranged hit position matches the target
