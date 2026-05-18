# language: en
Feature: Monster and Player Door Interaction
    So that dungeon doors feel meaningful
    As a game engine
    I want monsters and the player to interact with doors correctly

    Background:
        Given I have a game
        And I initialize the game
        And the game has a player

    Scenario: Monster with hands opens a closed door
        Given I place tiles for a door corridor east of the player
        And I place a Hobgoblin east of the door
        When I drive the spawned monster toward the player
        Then the door east of the player is open

    Scenario: Large monster bashes a closed door
        Given I place tiles for a door corridor east of the player
        And I place an Adult White Dragon east of the door
        When I drive the spawned monster toward the player
        Then the door east of the player is broken

    Scenario: Large monster bashes a locked door
        Given I place tiles for a door corridor east of the player
        And I lock the door east of the player
        And I place an Adult White Dragon east of the door
        When I drive the spawned monster toward the player
        Then the door east of the player is broken

    Scenario: Monster with hands cannot open a locked door
        Given I place tiles for a door corridor east of the player
        And I lock the door east of the player
        And I place a Hobgoblin east of the door
        When I drive the spawned monster toward the player
        Then the door east of the player is closed

    Scenario: Monster without hands or LARGE cannot open any door
        Given I place tiles for a door corridor east of the player
        And I place a Giant Centipede east of the door
        When I drive the spawned monster toward the player
        Then the door east of the player is closed

    Scenario: Player bashes an unlocked door open
        Given I place tiles for a door corridor east of the player
        And the player has a strength bonus of 20
        When the player bashes the door east
        Then the door east of the player is broken

    Scenario: Player bash fails when strength bonus is too low
        Given I place tiles for a door corridor east of the player
        And the player has a strength bonus of -100
        When the player bashes the door east
        Then the door east of the player is closed

    Scenario: Player spikes a door shut
        Given I place tiles for a door corridor east of the player
        And the player has an Iron Spike in inventory
        When the player spikes the door east
        Then the door east of the player is locked

    Scenario: Spiked door blocks a monster with hands
        Given I place tiles for a door corridor east of the player
        And the player has an Iron Spike in inventory
        And the player spikes the door east
        And I place a Hobgoblin east of the door
        When I drive the spawned monster toward the player
        Then the door east of the player is closed

    Scenario: Player unspikes a door
        Given I place tiles for a door corridor east of the player
        And I lock the door east of the player
        When the player unspikes the door east
        Then the door east of the player is not locked

    @skip
    Scenario: Wizard Lock scroll locks a door
        Given I have a game
        And I initialize the game
        And the game has a player
        And I place tiles for a door corridor east of the player
        And I spawn a Scroll of Wizard Lock in the player inventory
        When the player reads the scroll
        Then the door east of the player is locked
