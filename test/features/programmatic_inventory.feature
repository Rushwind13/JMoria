Feature: Programmatic inventory API
  Ensure the CPlayer programmatic API can operate on items by instance id (wield/quaff)

  Scenario: Programmatic wielding of a spawned dagger
    Given I have a Player
    Given I spawn a Dagger:37
    When I programmatically wield the spawned item
    Then the spawned item is equipped at 0

  Scenario: Programmatic quaffing of a spawned potion
    Given I have a Player
    Given I spawn a Potion of Minor Healing:16
    When I programmatically quaff the spawned item
    Then the spawned item is removed from inventory

  Scenario: Programmatic remove (unequip) of a spawned dagger
    Given I have a Player
    Given I spawn a Dagger:37
    When I programmatically wield the spawned item
    When I programmatically remove the spawned item
    Then the spawned item is back in inventory

  Scenario: Programmatic drop of a spawned dagger
    Given I have a Player
    Given I spawn a Dagger:37
    When I programmatically drop the spawned item
    Then the spawned item is on the ground at spawn location
    Then the spawned item is removed from inventory

  Scenario: Programmatic read of a spawned scroll
    Given I have a Player
    Given I spawn a Scroll of Light:20
    When I programmatically read the spawned item
    Then the spawned item is removed from inventory

  Scenario: Programmatic remove (unequip) of a cursed dagger should fail
    Given I have a Player
    Given I spawn a Dagger:37
    Given the Dagger:37 is cursed
    When I programmatically wield the spawned item
    When I programmatically attempt to remove the spawned item
    Then the programmatic remove failed
    Then The Dagger:37 is in equipment at 0

  Scenario: Programmatic wielding two-handed weapon should unequip shield
    Given I have a Player
    Given I spawn a Small Wooden Shield:29
    Given the Small Wooden Shield:29 is not cursed
    When I programmatically wield the spawned item
    Given I spawn a Battle Axe:42
    When I programmatically wield the spawned item
    Then The Small Wooden Shield:29 is not in equipment at 1
    And The Battle Axe:42 is in equipment at 0
