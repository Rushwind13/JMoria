Feature: Programmatic identification
  Verify item known-intrinsics truth-table and display gating

  Scenario: Item is cursed but unknown
    Given I create an item that is cursed and unknown
    Then the item should not be known to be cursed

  Scenario: Item is cursed and known
    Given I create an item that is cursed and known
    Then the item should be known to be cursed

  Scenario: Item is uncursed but known
    Given I create an item that is uncursed but known
    Then the item should be known to be uncursed

  Scenario: Item is uncursed and unknown
    Given I create an item that is uncursed and unknown
    Then the item should not be known to be cursed
