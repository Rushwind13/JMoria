# language: en
Feature: Effect System
    So that items can have magical effects
    As a game engine
    I want the effect flag system to work correctly

    Scenario: lookup EFFECT_FLAG from word 1
        Given I initialize my Constants
        Then I can look up <EFFECT_FLAG_FIRE> and get value 1

    Scenario: lookup EFFECT_FLAG2 from word 2
        Given I initialize my Constants
        Then I can look up <EFFECT_FLAG2_DOOR> and get value 1
        And I can look up <EFFECT_FLAG2_TRAP> and get value 2
        And I can look up <EFFECT_FLAG2_MONSTERS> and get value 4

    Scenario: LookupEffectFlag routes word 1 correctly
        Given I initialize my Constants
        When I call LookupEffectFlag with <EFFECT_FLAG_FIRE>
        Then flags word 1 is 1
        And flags word 2 is 0

    Scenario: LookupEffectFlag routes word 2 correctly
        Given I initialize my Constants
        When I call LookupEffectFlag with <EFFECT_FLAG2_DOOR>
        Then flags word 1 is 0
        And flags word 2 is 1

    Scenario: EffectFlagToString returns word 1 name
        Given I initialize my Constants
        When I call EffectFlagToString with word 1 set to 1 and word 2 set to 0
        Then the effect flag name is <EFFECT_FLAG_FIRE>

    Scenario: EffectFlagToString returns word 2 name
        Given I initialize my Constants
        When I call EffectFlagToString with word 1 set to 0 and word 2 set to 1
        Then the effect flag name is <EFFECT_FLAG2_DOOR>

    Scenario: EffectFlagToString returns none when both zero
        Given I initialize my Constants
        When I call EffectFlagToString with word 1 set to 0 and word 2 set to 0
        Then the effect flag name is <(none)>
