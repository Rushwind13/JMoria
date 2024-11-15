//
//  RunState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 11/14/24.
//  Copyright © 2024 Jimbo S. Harris. All rights reserved.
//

#include "RunState.h"

#include "Game.h"
#include "Player.h"

extern CGame *g_pGame;

CRunState::CRunState() : m_dwClock( 0 )
{
    m_pKeyHandlers[RUN_INIT] = &CRunState::OnHandleInit;
    m_pKeyHandlers[RUN_TICK] = &CRunState::OnHandleTick;

    m_eCurModifier = RUN_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

CRunState::~CRunState() {}

int CRunState::OnHandleKey( SDL_Keysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CRunState::OnHandleTick( SDL_Keysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling TICK modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        ResetToState( STATE_COMMAND );
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true, "TICK modifier complete, RUN state to next TICK\n" );
        DoTick();
        m_eCurModifier = RUN_TICK;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_NOISE, true, "RUN state still waiting for a valid key.\n" );
        return 0;
    }

    // We got a valid key
    JLog( LOG_LEVEL_DEBUG, true, "TICK modifier got a valid key\n" );
    DoTick();

    return 0;
}

int CRunState::OnHandleInit( SDL_Keysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing RUN state...\n" );

    DoTick();
    m_eCurModifier = RUN_TICK;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    return 0;
}

int CRunState::OnBaseHandleKey( SDL_Keysym *keysym ) { return JSUCCESS; }

void CRunState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = RUN_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

//////////////////////////////////////
/// command-specific fcns go below

//// Tick commands
bool CRunState::DoTick()
{
    m_dwClock++;

    if( g_pGame->GetPlayer()->Move( g_pGame->GetPlayer()->m_vVel ) == DUNG_COLL_NO_COLLISION )
    {
        JLog( LOG_LEVEL_NOISE, true, "RUN state continuing.\n" );
    }
    else
    {
        JLog( LOG_LEVEL_DEBUG, true, "RUN state collided, reset to CMD state.\n" );
        g_pGame->GetPlayer()->m_bIsDisturbed = true;
        g_pGame->GetPlayer()->m_vVel.Init( 0, 0 );
        ResetToState( STATE_COMMAND );
    }

    if( g_pGame->GetPlayer()->m_bIsDisturbed )
    {
        JLog( LOG_LEVEL_DEBUG, true, "RUN state complete, reset to CMD state.\n" );
        ResetToState( STATE_COMMAND );
    }

    g_pGame->SetReadyForUpdate( true );
    return true;
}
