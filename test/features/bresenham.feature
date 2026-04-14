# language: en
Feature: Bresenham Line
    So that trajectory and line-of-sight calculations are correct
    As the Bresenham algorithm
    I want to verify line generation for all directions and edge cases

    # distance parameter = max step budget (not target distance)
    # the algorithm walks up to 'distance' steps from source along the line

    Scenario: Horizontal line east
        When I compute a Bresenham line from 0,0 to 5,0 with distance 6
        Then the line has 6 points
        And the line contains 0,0 and 1,0 and 2,0 and 3,0 and 4,0 and 5,0

    Scenario: Horizontal line west
        When I compute a Bresenham line from 5,0 to 0,0 with distance 6
        Then the line has 6 points
        And the line contains 5,0 and 4,0 and 3,0 and 2,0 and 1,0 and 0,0

    Scenario: Vertical line south
        When I compute a Bresenham line from 0,0 to 0,5 with distance 6
        Then the line has 6 points
        And the line contains 0,0 and 0,1 and 0,2 and 0,3 and 0,4 and 0,5

    Scenario: Vertical line north
        When I compute a Bresenham line from 0,5 to 0,0 with distance 6
        Then the line has 6 points

    Scenario: Diagonal line
        When I compute a Bresenham line from 0,0 to 4,4 with distance 5
        Then the line has 5 points

    Scenario: Near-diagonal line
        When I compute a Bresenham line from 0,0 to 3,4 with distance 10
        Then the line has at least 4 points

    Scenario: Distance cap limits line length
        When I compute a Bresenham line from 0,0 to 10,0 with distance 5
        Then the line has 5 points

    Scenario: Obstacle blocks line
        When I compute a Bresenham line from 0,0 to 5,0 with distance 10 and obstacle at 3,0
        Then the Bresenham result is false

    Scenario: Zero distance produces empty line
        When I compute a Bresenham line from 0,0 to 5,0 with distance 0
        Then the line has 0 points

    # GenerateLine - pure line generation without collision
    Scenario: GenerateLine produces same points as Bresenham without obstacles
        When I generate a line from 0,0 to 5,0 with distance 6
        Then the line has 6 points
        And the line contains 0,0 and 1,0 and 2,0 and 3,0 and 4,0 and 5,0

    Scenario: GenerateLine diagonal
        When I generate a line from 0,0 to 4,4 with distance 5
        Then the line has 5 points

    # CheckLineCollision - collision checking on a pre-generated line
    Scenario: CheckLineCollision passes with no obstacles
        When I generate a line from 0,0 to 5,0 with distance 6
        And I check collision on the line from 0,0
        Then the collision result is true

    Scenario: CheckLineCollision fails at obstacle
        When I generate a line from 0,0 to 5,0 with distance 6 and obstacle at 3,0
        And I check collision on the line from 0,0
        Then the collision result is false
