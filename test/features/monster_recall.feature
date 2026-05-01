Feature: Monster Recall — Knowledge of the Ancestors

  Background:
    Given I have a game
    And I initialize the game
    And the game has a player

  # ─── First encounter ───────────────────────────────────────────────────────

  Scenario: No knowledge before first encounter
    Given a fresh monster recall system
    Then recall for "Giant Ant" shows "No knowledge"

  Scenario: First sighting does not reveal depth
    Given a fresh monster recall system
    When I record a sighting of "Giant Ant" at depth 250
    Then recall for "Giant Ant" does not show "Depth:"
    And  recall for "Giant Ant" does not show "XP:"

  Scenario: Kill reveals depth
    Given a fresh monster recall system
    When I record a sighting of "Giant Ant" at depth 250
    And  I record a kill of "Giant Ant" with estimated HP 12
    Then recall for "Giant Ant" shows "Depth:"

  # ─── Kill tracking ──────────────────────────────────────────────────────────

  Scenario: Kill unlocks XP
    Given a fresh monster recall system
    When I record a sighting of "Giant Ant" at depth 250
    And  I record a kill of "Giant Ant" with estimated HP 12
    Then recall for "Giant Ant" shows "XP:"
    And  recall for "Giant Ant" shows "HP:"
    And  recall for "Giant Ant" does not show "HD"

  Scenario: Kill counter increments
    Given a fresh monster recall system
    When I record a sighting of "Giant Ant" at depth 250
    And  I record a kill of "Giant Ant" with estimated HP 10
    And  I record a kill of "Giant Ant" with estimated HP 14
    Then the kill count for "Giant Ant" is 2

  # ─── HP range ───────────────────────────────────────────────────────────────

  Scenario: HP range tracks observed min and max
    Given a fresh monster recall system
    When I record a kill of "Giant Ant" with estimated HP 8
    And  I record a kill of "Giant Ant" with estimated HP 14
    Then the HP min for "Giant Ant" is 8
    And  the HP max for "Giant Ant" is 14

  # ─── Attack observation ──────────────────────────────────────────────────────

  Scenario: Observed attack appears in recall
    Given a fresh monster recall system
    When I record an attack by "Giant Ant" of type 4 damage 5 effect ""
    Then recall for "Giant Ant" shows "bites"
    And  recall for "Giant Ant" shows "5-5dmg"

  Scenario: Attack seen-count increments on repeated observation
    Given a fresh monster recall system
    When I record an attack by "Giant Ant" of type 4 damage 5 effect ""
    And  I record an attack by "Giant Ant" of type 4 damage 7 effect ""
    Then the attack seen count for "Giant Ant" type 4 is 2

  Scenario: Second distinct attack type is tracked separately
    Given a fresh monster recall system
    When I record an attack by "Cave Dragon" of type 64 damage 18 effect "fire"
    And  I record an attack by "Cave Dragon" of type 16 damage 8 effect ""
    Then the attack count for "Cave Dragon" is 2

  # ─── Flag observation ────────────────────────────────────────────────────────

  Scenario: Breed flag revealed by RecordObservation
    Given a fresh monster recall system
    When I record a sighting of "White Worm Mass" at depth 100
    And  I record observation of "White Worm Mass" flag 32768
    Then recall for "White Worm Mass" shows "Breeds"

  Scenario: Flag not shown before observation
    Given a fresh monster recall system
    When I record a sighting of "White Worm Mass" at depth 100
    Then recall for "White Worm Mass" does not show "Breeds"

  # ─── Per-session dedup ───────────────────────────────────────────────────────

  Scenario: Sighting only counted once per session
    Given a fresh monster recall system
    When I record a sighting of "Giant Ant" at depth 250
    And  I record a sighting of "Giant Ant" at depth 250
    Then the encounter count for "Giant Ant" is 1

  # ─── File round-trip ─────────────────────────────────────────────────────────

  Scenario: Save and reload preserves all recall data
    Given a fresh monster recall system
    When I record a sighting of "Giant Ant" at depth 250
    And  I record a kill of "Giant Ant" with estimated HP 12
    And  I record an attack by "Giant Ant" of type 4 damage 5 effect ""
    And  I record observation of "Giant Ant" flag 256
    And  I save and reload the recall data
    Then the encounter count for "Giant Ant" is 1
    And  the kill count for "Giant Ant" is 1
    And  the HP min for "Giant Ant" is 12
    And  recall for "Giant Ant" shows "bites"
    And  recall for "Giant Ant" shows "Warm."

  # ─── Fixture file parsing ────────────────────────────────────────────────────

  Scenario: Load sample fixture file
    Given a fresh monster recall system
    When I load the recall fixture "../../JMoria/test/fixtures/monster_recall_sample.txt"
    Then the encounter count for "Giant Ant" is 7
    And  the kill count for "Giant Ant" is 3
    And  the HP min for "Giant Ant" is 8
    And  the HP max for "Giant Ant" is 14
    And  the attack count for "Giant Ant" is 1
    And  the encounter count for "White Worm Mass" is 2
    And  the attack count for "Cave Dragon" is 2
