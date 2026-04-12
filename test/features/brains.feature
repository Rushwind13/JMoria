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