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
