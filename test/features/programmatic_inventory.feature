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
