//
//  ClockStepState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#include "ClockStepState.h"

#include "DisplayText.h"
#include "Dungeon.h"
#include "Game.h"

extern CGame *g_pGame;

CClockStepState::CClockStepState()
    : m_dwClock( 0 ),
      m_dwStep( 1 ),
      m_bShowDiagnostics( true ),
      m_bLevelPopulated( false )
{
    // m_bLevelPopulated: Tracks whether scenery/items/monsters have been placed.
    // Prevents re-spawning on every tick after generation completes.
    m_pKeyHandlers[CLOCKSTEP_INIT] = &CClockStepState::OnHandleInit;
    m_pKeyHandlers[CLOCKSTEP_TICK] = &CClockStepState::OnHandleTick;

    m_eCurModifier = CLOCKSTEP_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

CClockStepState::~CClockStepState() {}

int CClockStepState::OnHandleKey( JKeysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CClockStepState::OnHandleTick( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling TICK modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true, "TICK modifier complete, CLOCKSTEP state to next TICK\n" );
        DoTick();
        m_eCurModifier = CLOCKSTEP_TICK;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true, "CLOCK still waiting for a valid key.\n" );
        return 0;
    }

    // We got a valid key
    JLog( LOG_LEVEL_NOISE, true, "TICK modifier got a valid key\n" );
    g_pGame->GetEnd()->Clear();
    DoTick();

    return 0;
}

int CClockStepState::OnHandleInit( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing CLOCKSTEP state...\n" );

    // Create the initial dungeon level.
    // In CLOCKSTEP mode, CreateNewLevel skips PopulateLevel,
    // so the player won't be spawned yet.
    if( g_pGame && g_pGame->GetDungeon() && g_pGame->GetDungeon()->depth == 0 )
    {
        g_pGame->GetDungeon()->OnChangeLevel( DUNG_CFG_START_LEVEL );
        m_bLevelPopulated = false;
    }

    if( g_pGame && g_pGame->GetStats() )
    {
        g_pGame->GetStats()->Clear();
        g_pGame->GetStats()->Printf( g_Strings[STR_CLOCKSTEP_MODE] );
        g_pGame->GetStats()->Printf( g_Strings[STR_DUNGEON_GENERATION_COMPLETE] );
        g_pGame->GetStats()->Printf( g_Strings[STR_SPAWN_PLAYER] );
    }

    m_eCurModifier = CLOCKSTEP_TICK;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    return 0;
}

int CClockStepState::OnBaseHandleKey( JKeysym *keysym )
{
    if( keysym->sym == JKEY_RETURN || keysym->sym == JKEY_SPACE )
    {
        // Only continue ticking if generation not complete.
        // After PopulateLevel() called (m_bLevelPopulated=true), SPACE is ignored
        // to prevent re-calling PopulateLevel() and duplicating scenery/items/monsters.
        if( !m_bLevelPopulated )
            return JCOMPLETESTATE;
        else
            return -1; // Ignore SPACE after generation complete
    }

    if( keysym->sym == JKEY_ESCAPE )
    {
        // Force-complete generation if still in progress, redrawing each step
        while( g_pGame->GetDungeon()->Tick( m_dwClock++ ) )
        {
            g_pGame->GetDungeon()->SetDrawFlag( true );
        }

        // Complete generation with stats + populate if not done yet
        if( !m_bLevelPopulated )
        {
            CompleteGeneration();
        }

        // Clear stale CLOCKSTEP diagnostics from stats panel
        g_pGame->GetStats()->Clear();

        ResetToState( STATE_COMMAND );
        return JRESETSTATE;
    }

    return -1;
}

void CClockStepState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = CLOCKSTEP_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

//////////////////////////////////////
/// command-specific fcns go below

//// Tick commands
bool CClockStepState::DoTick()
{
    m_dwClock += m_dwStep;

    // Safety check for dungeon and level
    if( !g_pGame || !g_pGame->GetDungeon() || !g_pGame->GetDungeon()->GetCurLevel() )
    {
        JLog( LOG_LEVEL_ERROR, true, "DoTick called but dungeon not ready!\n" );
        g_pGame->GetStats()->Printf( g_Strings[STR_ERROR_DUNGEON_NOT_INITIALIZED] );
        return false;
    }

    if( m_bShowDiagnostics )
    {
        CDungeonMap *pMap = g_pGame->GetDungeon()->GetCurLevel();
        const DungeonGenDiagnostics &diag = pMap->GetDiagnostics();
        double elapsed_ms = Util::GetTimeInMillis() - diag.start_time_ms;

        g_pGame->GetStats()->Printf( g_Strings[STR_TICK], m_dwClock );
        g_pGame->GetStats()->Printf( g_Strings[STR_SEED_U], pMap->GetSeed() );
        g_pGame->GetStats()->Printf( g_Strings[STR_STACK], pMap->GetStackSize() );
        g_pGame->GetStats()->Printf( g_Strings[STR_ROOMS], pMap->GetRoomCount() );
        g_pGame->GetStats()->Printf( g_Strings[STR_HALLS], pMap->GetHallwayCount() );
        g_pGame->GetStats()->Printf( g_Strings[STR_TIME_MS], elapsed_ms );
    }
    else
    {
        g_pGame->GetStats()->Printf( g_Strings[STR_TICK], m_dwClock );
    }

    g_pGame->SetReadyForUpdate( true );
    bool bStillGenerating = g_pGame->GetDungeon()->Tick( m_dwClock );

    // Force dungeon to redraw after each generation step
    g_pGame->GetDungeon()->SetDrawFlag( true );

    if( !bStillGenerating && !m_bLevelPopulated )
    {
        CompleteGeneration();
        g_pGame->GetStats()->Printf( g_Strings[STR_START_PLAYING] );
    }

    // Optional: Add small delay to prevent CPU spike during stepped generation
    // Yields to system and keeps UI responsive. Can be disabled for faster generation.
    // SDL_Delay( 1 ); // Uncomment to add 1ms delay per step

    return true;
}

void CClockStepState::CompleteGeneration()
{
    CDungeonMap *pMap = g_pGame->GetDungeon()->GetCurLevel();
    const DungeonGenDiagnostics &diag = pMap->GetDiagnostics();
    double total_ms = Util::GetTimeInMillis() - diag.start_time_ms;

    g_pGame->GetStats()->Printf( g_Strings[STR_GENERATION_COMPLETE] );
    g_pGame->GetStats()->Printf( g_Strings[STR_TIME_MS_SEC], total_ms, total_ms / 1000.0 );
    g_pGame->GetStats()->Printf( g_Strings[STR_ROOMS_HALLS], pMap->GetRoomCount(),
                                 pMap->GetHallwayCount() );
    if( total_ms > 0.0 )
    {
        double steps_per_sec = ( diag.steps_created * 1000.0 ) / total_ms;
        g_pGame->GetStats()->Printf( g_Strings[STR_STEPS_PER_SEC], steps_per_sec );
    }
    g_pGame->GetStats()->Printf( g_Strings[STR_POPULATE] );

    // PopulateLevel() called exactly once after generation completes.
    // m_bLevelPopulated flag prevents duplicate spawns on subsequent ticks.
    g_pGame->GetDungeon()->PopulateLevel( g_pGame->GetDungeon()->depth );
    m_bLevelPopulated = true;
}
