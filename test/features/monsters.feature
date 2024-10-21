# language: en
Feature: Render All Monsters
    So that all possible monster configurations
    As a map generator
    I want to be able to parse monsters safely

    Scenario: I can render the first monster
        Given A map with a single level
        Given The monster configuration file
        When I read a monster from the config file
        When I create a monster
        Then I can see the monster name
