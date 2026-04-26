# language: en
Feature: Visible Monsters Pane
    So that the player can see what monsters are nearby at a glance
    As the visible monsters pane
    I want to show visible monsters grouped by type, sorted by distance

    Background:
        Given the dungeon is seeded
        And I have a Player
        And the game has a player

    Scenario: Monsters pane is hidden by default
        Then the monsters pane is not showing

    Scenario: Toggle v shows and hides the monsters pane
        When I toggle the monsters pane
        Then the monsters pane is showing
        When I toggle the monsters pane
        Then the monsters pane is not showing

    Scenario: A visible monster appears in the pane
        Given I spawn a Kobold, a monster at distance 1
        When I toggle the monsters pane
        And I update visible monsters
        Then the monsters pane contains "Kobold"

    Scenario: Two of the same monster type show a count
        Given I spawn a Kobold, a monster at distance 1
        And I spawn a Kobold, a monster at distance 2
        When I toggle the monsters pane
        And I update visible monsters
        Then the monsters pane contains "Kobold (2)"

    Scenario: Closest monster type appears first
        Given I spawn a Kobold, a monster at distance 3
        And I spawn a Giant Ant, a monster at distance 1
        When I toggle the monsters pane
        And I update visible monsters
        Then the monsters pane lists "Giant Ant" before "Kobold"
