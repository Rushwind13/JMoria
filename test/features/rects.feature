# language: en
Feature: Rects
    So that positions will work properly
    As a game engine
    I want to test many things about JRect math

    Scenario: I can create positions
        Given I have a JRect 1,2,3,4 in the data
        Then The rect is in world

    Scenario: Rect is in world
        Given I have a JRect 0,0,0,0 in the data
        Then The rect is in world

    Scenario: Rect is in world
        Given I have a JRect 99,99,99,99 in the data
        Then The rect is in world

    Scenario: Rect is not in world horz
        Given I have a JRect 50,50,100,50 in the data
        Then The rect is not in world

    Scenario: Rect is not in world vert
        Given I have a JRect 50,50,50,100 in the data
        Then The rect is not in world

    Scenario: Rect is not in world small horz
        Given I have a JRect -1,50,50,50 in the data
        Then The rect is not in world

    Scenario: Rect is not in world small vert
        Given I have a JRect 50,-1,50,50 in the data
        Then The rect is not in world

    Scenario: Rect is invalid horz
        Given I have a JRect 50,50,-1,50 in the data
        Then The rect is not in world

    Scenario: Rect is invalid vert
        Given I have a JRect 50,50,50,-1 in the data
        Then The rect is not in world

    Scenario: Rect SetWidth
        Given I have a JRect 10,10,10,10 in the data
        When I call SetWidth 5,true
        Then The rect corners are now 10,10,15,10
        And The rect width is 5
        And The rect is in world

    Scenario: Rect SetWidth
        Given I have a JRect 10,10,10,10 in the data
        When I call SetWidth 5,false
        Then The rect corners are now 5,10,10,10
        And The rect width is 5
        And The rect is in world