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
#include "DungeonConstants.h"
#include "DungeonTile.h"
#include "Game.h"
#include "Player.h"

#include "FileParse.h"

extern CGame *g_pGame;

CClockStepState::CClockStepState() : m_dwClock( 0 ), m_dwStep( 1 ), m_bShowDiagnostics( true ), m_bLevelPopulated( false )
{
    // m_bLevelPopulated: Tracks whether scenery/items/monsters have been placed.
    // Prevents re-spawning on every tick after generation completes.
    m_pKeyHandlers[CLOCKSTEP_INIT] = &CClockStepState::OnHandleInit;
    m_pKeyHandlers[CLOCKSTEP_TICK] = &CClockStepState::OnHandleTick;

    m_eCurModifier = CLOCKSTEP_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

CClockStepState::~CClockStepState() {}

int CClockStepState::OnHandleKey( SDL_Keysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CClockStepState::OnHandleTick( SDL_Keysym *keysym )
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

int CClockStepState::OnHandleInit( SDL_Keysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing CLOCKSTEP state...\n" );

    // Create the initial dungeon level without spawning player yet.
    // Player spawn is deferred to avoid NULL pointer crashes during generation
    // (many functions check g_pGame->GetPlayer()->m_bHasSpawned)
    if( g_pGame && g_pGame->GetDungeon() && g_pGame->GetDungeon()->depth == 0 )
    {
        g_pGame->GetDungeon()->OnChangeLevel( DUNG_CFG_START_LEVEL );
        m_bLevelPopulated = false;
    }

    if( g_pGame && g_pGame->GetStats() )
    {
        g_pGame->GetStats()->Clear();
        g_pGame->GetStats()->Printf( "CLOCKSTEP Mode\n" );
        g_pGame->GetStats()->Printf( "Press SPACE to step through generation\n" );
        g_pGame->GetStats()->Printf( "Press ESC when done to spawn player\n" );
    }
    
    m_eCurModifier = CLOCKSTEP_TICK;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    return 0;
}

int CClockStepState::OnBaseHandleKey( SDL_Keysym *keysym )
{
    if( keysym->sym == SDLK_RETURN || keysym->sym == SDLK_SPACE )
    {
        // Only continue ticking if generation not complete.
        // After PopulateLevel() called (m_bLevelPopulated=true), SPACE is ignored
        // to prevent re-calling PopulateLevel() and duplicating scenery/items/monsters.
        if( !m_bLevelPopulated )
            return JCOMPLETESTATE;
        else
            return -1; // Ignore SPACE after generation complete
    }
    
    if( keysym->sym == SDLK_ESCAPE )
    {
        // Exit CLOCKSTEP mode and spawn player
        g_pGame->GetPlayer()->m_bHasSpawned = false;
        g_pGame->GetPlayer()->SpawnPlayer();
        g_pGame->GetMsgs()->Printf( "You pass through a one-way door, to arrive on level %d.\n",
                                    g_pGame->GetDungeon()->depth );
        ResetToState( STATE_COMMAND );
        return JSUCCESS;
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
        g_pGame->GetStats()->Printf( "ERROR: Dungeon not initialized\n" );
        return false;
    }
    
    if( m_bShowDiagnostics )
    {
        CDungeonMap *pMap = g_pGame->GetDungeon()->GetCurLevel();
        g_pGame->GetStats()->Printf( "Tick! %d\n", m_dwClock );
        g_pGame->GetStats()->Printf( "Seed: %u\n", pMap->GetSeed() );
        g_pGame->GetStats()->Printf( "Stack: %d\n", pMap->GetStackSize() );
        g_pGame->GetStats()->Printf( "Rooms: %d\n", pMap->GetRoomCount() );
        g_pGame->GetStats()->Printf( "Halls: %d\n", pMap->GetHallwayCount() );
    }
    else
    {
        g_pGame->GetStats()->Printf( "Tick! %d\n", m_dwClock );
    }
    
    g_pGame->SetReadyForUpdate( true );
    bool bStillGenerating = g_pGame->GetDungeon()->Tick( m_dwClock );
    
    // Force dungeon to redraw after each generation step
    g_pGame->GetDungeon()->SetDrawFlag( true );
    
    if( !bStillGenerating && !m_bLevelPopulated )
    {
        g_pGame->GetStats()->Printf( "\nGeneration complete!\n" );
        g_pGame->GetStats()->Printf( "Placing scenery, items, and monsters...\n" );
        
        // PopulateLevel() called exactly once after generation completes.
        // m_bLevelPopulated flag prevents duplicate spawns on subsequent ticks.
        g_pGame->GetDungeon()->PopulateLevel( g_pGame->GetDungeon()->depth );
        m_bLevelPopulated = true;
        
        g_pGame->GetStats()->Printf( "Press ESC to spawn player.\n" );
    }
    
    return true;
}
