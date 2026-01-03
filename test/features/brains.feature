# language: en
Feature: Brains
    So that my AI can kill the player
    As a game engine
    I want to make sure my Brains is working

    Scenario: Can set brain state
        Given I have a brain
        And I set the move type to MON_AI_SEEKPLAYER
        When I set the brain state to 2
        Then GetState returns 2
    Scenario: Can update the brain
        Given I have a brain
        And I set the move type to MON_AI_SEEKPLAYER
        When I call WalkSeek
        Then WalkSeek was successful

    Scenario: GetStateString returns correct string for each state
        Given I have a brain
        When I set the brain state to 0
        Then GetStateString returns "rest"
        When I set the brain state to 1
        Then GetStateString returns "gotodest"
        When I set the brain state to 2
        Then GetStateString returns "seek"
        When I set the brain state to 3
        Then GetStateString returns "idle"