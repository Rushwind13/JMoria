# language: en
Feature: Multicolor Beam Effects
    So that wands emit color-coded beams based on their effect type
    As the visual effects system
    I want to verify beam colors match effect types (Fire, Cold, Acid, Electricity)

    Background:
        Given I initialize my Constants
        And the dungeon is seeded
        And I have a Player
        And the game has a player

    # AC1.1: Fire wands emit red/orange gradient beam in OpenGL mode
    Scenario: Fire wand emits red/orange beam
        Given the player has a Wand of Firebolts in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the beam effect has EFFECT_FLAG_FIRE
        And the beam renders with red/orange gradient colors
        And the beam renders with character 'w'

    # AC1.2: Cold wands emit blue/white gradient beam in OpenGL mode
    Scenario: Cold wand emits blue/white beam
        Given the player has a Wand of Frost in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the beam effect has EFFECT_FLAG_COLD
        And the beam renders with cyan/blue/white gradient colors
        And the beam renders with character 'x'

    # AC1.3: Acid wands emit green beam in OpenGL mode
    Scenario: Acid wand emits green beam
        Given the player has a Wand of Acid in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the beam effect has EFFECT_FLAG_ACID
        And the beam renders with green gradient colors
        And the beam renders with character '*'

    # AC1.4: Electric wands emit yellow/white beam in OpenGL mode
    Scenario: Electric wand emits yellow/white beam
        Given the player has a Wand of Lightning in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the beam effect has EFFECT_FLAG_ELECTRICITY
        And the beam renders with yellow/white gradient colors
        And the beam renders with character '-'

    # AC1.5: ASCII mode renders beams with distinct characters per type (data-driven from Effects.txt Beam field)
    Scenario: Fire beam uses character 'w' in ASCII mode
        Given the player has a Wand of Firebolts in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the beam effect has EFFECT_FLAG_FIRE
        And the beam renders with character 'w'

    Scenario: Cold beam uses character 'x' in ASCII mode
        Given the player has a Wand of Frost in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the beam effect has EFFECT_FLAG_COLD
        And the beam renders with character 'x'

    Scenario: Acid beam uses character '*' in ASCII mode
        Given the player has a Wand of Acid in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the beam effect has EFFECT_FLAG_ACID
        And the beam renders with character '*'

    Scenario: Electric beam uses character '-' in ASCII mode
        Given the player has a Wand of Lightning in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        Then the beam effect has EFFECT_FLAG_ELECTRICITY
        And the beam renders with character '-'

    # AC1.7: Existing beam trajectory/range mechanics unchanged
    Scenario: Beam trajectory mechanics remain unchanged
        Given the player has a Wand of Firebolts in slot a
        And I spawn a Kobold, a monster at distance 3
        When I enter targeting mode
        And I confirm the target
        And I zap slot a
        And the projectile completes its trajectory
        Then the ranged hit position matches the target
