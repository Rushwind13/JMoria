//
//  ClockStepState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#include "ClockStepState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"

#include "FileParse.h"

extern CGame *g_pGame;

CClockStepState::CClockStepState() : m_dwClock( 0 ), m_dwStep( 1 )
{
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
    printf( "Handling TICK modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        printf( "TICK modifier complete, CLOCKSTEP state to next TICK\n" );
        DoTick();
        m_eCurModifier = CLOCKSTEP_TICK;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    }

    if( retval != JSUCCESS )
    {
        printf( "CLOCK still waiting for a valid key.\n" );
        return 0;
    }

    // We got a valid key
    printf( "TICK modifier got a valid key\n" );
    g_pGame->GetEnd()->Clear();
    DoTick();

    return 0;
}

int CClockStepState::OnHandleInit( SDL_Keysym *keysym )
{
    printf( "Initializing CLOCKSTEP state...\n" );

    g_pGame->GetStats()->Clear();
    DoTick();
    m_eCurModifier = CLOCKSTEP_TICK;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    return 0;
}

int CClockStepState::OnBaseHandleKey( SDL_Keysym *keysym )
{
    if( keysym->sym == SDLK_RETURN || keysym->sym == SDLK_SPACE )
    {
        return JCOMPLETESTATE;
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
    g_pGame->GetStats()->Printf( "Tick! %d\n", m_dwClock );
    g_pGame->SetReadyForUpdate( true );
    g_pGame->GetDungeon()->Tick( m_dwClock );
    return true;
}
