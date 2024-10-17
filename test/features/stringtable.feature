# language: en
Feature: StringTable
    So that I can parse string config in my game
    As a game engine
    I want to make sure my stringtable is working

    Scenario: lookup strings
        Given I initialize my Constants
        Then I get a StringTable with <MON_IDX_BALROG> in it with index 26