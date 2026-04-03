# language: en
Feature: TextEntry
    So that I can parse string config in my game
    As a game engine
    I want to make sure my textentry is working

    Scenario: create a TextEntry
        Given I create a TextEntry of <pizza>,13
        Then I get a TextEntry with <pizza>,13

    Scenario: create a different TextEntry
        Given I create a TextEntry of <dolphin>,42
        Then I get a TextEntry with <dolphin>,42
