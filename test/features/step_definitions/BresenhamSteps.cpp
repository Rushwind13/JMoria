#include "TestContext.hpp"
using cucumber::ScenarioScope;

#include "Util.h"

// Shared state for Bresenham step definitions
static JLinkList<JIVector> *s_bresLine = NULL;
static bool s_bresResult = true;
static JIVector s_obstacle( -1, -1 );
static bool s_collisionResult = true;

// Callback: everything is walkable
static bool AlwaysWalkable( JVector &v ) { return true; }

// Callback: everything except the obstacle position
static bool WalkableExceptObstacle( JVector &v )
{
    if( (int)v.x == s_obstacle.x && (int)v.y == s_obstacle.y )
        return false;
    return true;
}

static void CleanupBresLine()
{
    if( s_bresLine )
    {
        s_bresLine->Terminate();
        delete s_bresLine;
        s_bresLine = NULL;
    }
    s_bresResult = true;
    s_collisionResult = true;
    s_obstacle.Init( -1, -1 );
}

/*#######
##
## WHEN
##
#######*/
WHEN( "^I compute a Bresenham line from ([0-9]+),([0-9]+) to ([0-9]+),([0-9]+) with distance "
      "([0-9]+)$" )
{
    REGEX_PARAM( int, sx );
    REGEX_PARAM( int, sy );
    REGEX_PARAM( int, tx );
    REGEX_PARAM( int, ty );
    REGEX_PARAM( int, dist );
    ScenarioScope<TestCtx> context;

    CleanupBresLine();
    s_bresLine = new JLinkList<JIVector>;
    JIVector src( sx, sy );
    JIVector tgt( tx, ty );
    s_bresResult = Util::Bresenham( src, tgt, (uint8)dist, AlwaysWalkable, s_bresLine );
}

WHEN( "^I compute a Bresenham line from ([0-9]+),([0-9]+) to ([0-9]+),([0-9]+) with distance "
      "([0-9]+) and obstacle at ([0-9]+),([0-9]+)$" )
{
    REGEX_PARAM( int, sx );
    REGEX_PARAM( int, sy );
    REGEX_PARAM( int, tx );
    REGEX_PARAM( int, ty );
    REGEX_PARAM( int, dist );
    REGEX_PARAM( int, ox );
    REGEX_PARAM( int, oy );
    ScenarioScope<TestCtx> context;

    CleanupBresLine();
    s_bresLine = new JLinkList<JIVector>;
    s_obstacle.Init( ox, oy );
    JIVector src( sx, sy );
    JIVector tgt( tx, ty );
    s_bresResult = Util::Bresenham( src, tgt, (uint8)dist, WalkableExceptObstacle, s_bresLine );
}

WHEN( "^I generate a line from ([0-9]+),([0-9]+) to ([0-9]+),([0-9]+) with distance ([0-9]+)$" )
{
    REGEX_PARAM( int, sx );
    REGEX_PARAM( int, sy );
    REGEX_PARAM( int, tx );
    REGEX_PARAM( int, ty );
    REGEX_PARAM( int, dist );
    ScenarioScope<TestCtx> context;

    CleanupBresLine();
    JIVector src( sx, sy );
    JIVector tgt( tx, ty );
    s_bresLine = Util::GenerateLine( src, tgt, (uint8)dist );
}

WHEN( "^I generate a line from ([0-9]+),([0-9]+) to ([0-9]+),([0-9]+) with distance ([0-9]+) and "
      "obstacle at ([0-9]+),([0-9]+)$" )
{
    REGEX_PARAM( int, sx );
    REGEX_PARAM( int, sy );
    REGEX_PARAM( int, tx );
    REGEX_PARAM( int, ty );
    REGEX_PARAM( int, dist );
    REGEX_PARAM( int, ox );
    REGEX_PARAM( int, oy );
    ScenarioScope<TestCtx> context;

    CleanupBresLine();
    s_obstacle.Init( ox, oy );
    JIVector src( sx, sy );
    JIVector tgt( tx, ty );
    s_bresLine = Util::GenerateLine( src, tgt, (uint8)dist );
}

WHEN( "^I check collision on the line from ([0-9]+),([0-9]+)$" )
{
    REGEX_PARAM( int, sx );
    REGEX_PARAM( int, sy );
    ScenarioScope<TestCtx> context;

    JIVector src( sx, sy );
    if( s_obstacle.x >= 0 )
        s_collisionResult = Util::CheckLineCollision( s_bresLine, src, WalkableExceptObstacle );
    else
        s_collisionResult = Util::CheckLineCollision( s_bresLine, src, AlwaysWalkable );
}

/*#######
##
## THEN
##
#######*/
THEN( "^the line has ([0-9]+) points$" )
{
    REGEX_PARAM( int, expected );
    ScenarioScope<TestCtx> context;

    int actual = s_bresLine ? (int)s_bresLine->length() : 0;
    EXPECT_EQ( actual, expected );
}

THEN( "^the line has at least ([0-9]+) points$" )
{
    REGEX_PARAM( int, minimum );
    ScenarioScope<TestCtx> context;

    int actual = s_bresLine ? (int)s_bresLine->length() : 0;
    EXPECT_GE( actual, minimum );
}

THEN( "^the line contains (.+)$" )
{
    REGEX_PARAM( std::string, pointList );
    ScenarioScope<TestCtx> context;

    ASSERT_NE( s_bresLine, nullptr );

    // Parse "x1,y1 and x2,y2 and ..." into expected points
    std::vector<std::pair<int, int>> expected;
    std::string remaining = pointList;
    while( !remaining.empty() )
    {
        // Find next coordinate pair
        int x, y;
        int consumed = 0;
        if( sscanf( remaining.c_str(), "%d,%d%n", &x, &y, &consumed ) == 2 )
        {
            expected.push_back( { x, y } );
            remaining = remaining.substr( consumed );
            // Skip " and " if present
            if( remaining.find( " and " ) == 0 )
                remaining = remaining.substr( 5 );
            // Skip whitespace
            while( !remaining.empty() && remaining[0] == ' ' )
                remaining = remaining.substr( 1 );
        }
        else
        {
            break;
        }
    }

    ASSERT_EQ( (int)s_bresLine->length(), (int)expected.size() );

    CLink<JIVector> *pLink = s_bresLine->GetHead();
    for( size_t i = 0; i < expected.size(); i++ )
    {
        ASSERT_NE( pLink, nullptr );
        EXPECT_EQ( pLink->m_lpData->x, expected[i].first );
        EXPECT_EQ( pLink->m_lpData->y, expected[i].second );
        pLink = pLink->next;
    }
}

THEN( "^the Bresenham result is false$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_FALSE( s_bresResult );
}

THEN( "^the Bresenham result is true$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_TRUE( s_bresResult );
}

THEN( "^the collision result is true$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_TRUE( s_collisionResult );
}

THEN( "^the collision result is false$" )
{
    ScenarioScope<TestCtx> context;
    EXPECT_FALSE( s_collisionResult );
}
