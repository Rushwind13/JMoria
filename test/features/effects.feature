# language: en
Feature: Effect System
    So that items can have magical effects
    As a game engine
    I want the effect flag system to work correctly

    Scenario: lookup EFFECT_FLAG from word 1
        Given I initialize my Constants
        Then I can look up <EFFECT_FLAG_FIRE> and get value 1

    Scenario: lookup EFFECT_FLAG from word 2
        Given I initialize my Constants
        Then I can look up <EFFECT_FLAG_DOOR> and get value 1
        And I can look up <EFFECT_FLAG_TRAP> and get value 2
        And I can look up <EFFECT_FLAG_MONSTERS> and get value 4

    Scenario: LookupEffectFlag routes word 1 correctly
        Given I initialize my Constants
        When I call LookupEffectFlag with <EFFECT_FLAG_FIRE>
        Then flags word 1 is 1
        And flags word 2 is 0

    Scenario: LookupEffectFlag routes word 2 correctly
        Given I initialize my Constants
        When I call LookupEffectFlag with <EFFECT_FLAG_DOOR>
        Then flags word 1 is 0
        And flags word 2 is 1

    Scenario: EffectFlagToString returns word 1 name
        Given I initialize my Constants
        When I call EffectFlagToString with word 1 set to 1 and word 2 set to 0
        Then the effect flag name is <EFFECT_FLAG_FIRE>

    Scenario: EffectFlagToString returns word 2 name
        Given I initialize my Constants
        When I call EffectFlagToString with word 1 set to 0 and word 2 set to 1
        Then the effect flag name is <EFFECT_FLAG_DOOR>

    Scenario: EffectFlagToString returns none when both zero
        Given I initialize my Constants
        When I call EffectFlagToString with word 1 set to 0 and word 2 set to 0
        Then the effect flag name is <(none)>

    Scenario: RECALL flag is defined in word 1
        Given I initialize my Constants
        Then I can look up <EFFECT_FLAG_RECALL> and get value 131072

    Scenario: SUMMON flag is defined in word 1
        Given I initialize my Constants
        Then I can look up <EFFECT_FLAG_SUMMON> and get value 524288

    @skip
    Scenario: Reading Word of Recall from town takes player to the dungeon
        Given I have a Player
        And the dungeon depth is 0
        And I spawn a Scroll of Word of Recall
        When I programmatically read the spawned item
        And the recall completes
        Then the dungeon depth is greater than 0
        And the spawned item is removed from inventory

    @skip
    Scenario: Reading Word of Recall from dungeon returns player to town
        Given I have a Player
        And the player descends to depth 3
        And I spawn a Scroll of Word of Recall
        When I programmatically read the spawned item
        And the recall completes
        Then the dungeon depth is 0
        And the spawned item is removed from inventory

    # Deferred: requires monsters at all depth ranges to be reliable
    # Scenario: Reading a Scroll of Summon Monsters spawns monsters near the player
    #     Given I have a Player
    #     And the player descends to depth 30
    #     And I record the monster count
    #     And I spawn a Scroll of Summon Monsters
    #     When I programmatically read the spawned item
    #     Then the monster count increased
    #     And the spawned item is removed from inventory
