Feature: Programmatic inventory API
  Ensure the CPlayer programmatic API can operate on items by instance id (wield/quaff)

  Scenario: Programmatic wielding of a spawned dagger
    Given I have a Player
    Given I spawn a Dagger
    When I programmatically wield the spawned item
    Then the spawned item is equipped at 0

  Scenario: Programmatic quaffing of a spawned potion
    Given I have a Player
    Given I spawn a Potion of Minor Healing
    When I programmatically quaff the spawned item
    Then the spawned item is removed from inventory

  Scenario: Programmatic remove (unequip) of a spawned dagger
    Given I have a Player
    Given I spawn a Dagger
    When I programmatically wield the spawned item
    When I programmatically remove the spawned item
    Then the spawned item is back in inventory

  Scenario: Programmatic drop of a spawned dagger
    Given I have a Player
    Given I spawn a Dagger
    When I programmatically drop the spawned item
    Then the spawned item is on the ground at spawn location
    Then the spawned item is removed from inventory

  Scenario: Programmatic read of a spawned scroll
    Given I have a Player
    Given I spawn a Scroll of Light
    When I programmatically read the spawned item
    Then the spawned item is removed from inventory

  Scenario: Programmatic remove (unequip) of a cursed dagger should fail
    Given I have a Player
    Given I spawn a Dagger
    Given the Dagger is cursed
    When I programmatically wield the spawned item
    When I programmatically attempt to remove the spawned item
    Then the programmatic remove failed
    Then The Dagger is in equipment at 0

  Scenario: Programmatic wielding two-handed weapon should unequip shield
    Given I have a Player
    Given I spawn a Small Wooden Shield
    Given the Small Wooden Shield is not cursed
    When I programmatically wield the spawned item
    Given I spawn a Battle Axe
    When I programmatically wield the spawned item
    Then The Small Wooden Shield is not in equipment at 1
    And The Battle Axe is in equipment at 0
