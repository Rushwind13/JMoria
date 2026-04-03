//
//  RestState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#include "RestState.h"

#include "Game.h"
#include "Player.h"

extern CGame *g_pGame;

CRestState::CRestState() : m_dwClock( 0 )
{
    m_pKeyHandlers[REST_INIT] = &CRestState::OnHandleInit;
    m_pKeyHandlers[REST_TICK] = &CRestState::OnHandleTick;

    m_eCurModifier = REST_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

CRestState::~CRestState() {}

int CRestState::OnHandleKey( SDL_Keysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CRestState::OnHandleTick( SDL_Keysym *keysym )
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
        JLog( LOG_LEVEL_DEBUG, true, "TICK modifier complete, REST state to next TICK\n" );
        DoTick();
        m_eCurModifier = REST_TICK;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_WARN, true, "REST state still waiting for a valid key.\n" );
        return 0;
    }

    // We got a valid key
    JLog( LOG_LEVEL_NOISE, true, "TICK modifier got a valid key\n" );
    DoTick();

    return 0;
}

int CRestState::OnHandleInit( SDL_Keysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing REST state...\n" );

    g_pGame->GetPlayer()->m_bIsDisturbed = false;

    DoTick();
    m_eCurModifier = REST_TICK;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    return 0;
}

int CRestState::OnBaseHandleKey( SDL_Keysym *keysym )
{
    if( keysym->sym == SDLK_RETURN || keysym->sym == SDLK_SPACE )
    {
        return JCOMPLETESTATE;
    }

    return -1;
}

void CRestState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = REST_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

//////////////////////////////////////
/// command-specific fcns go below

//// Tick commands
bool CRestState::DoTick()
{
    m_dwClock++;
    if( g_pGame->GetPlayer()->m_bIsRested || g_pGame->GetPlayer()->m_bIsDisturbed )
    {
        JLog( LOG_LEVEL_DEBUG, true, "REST state complete, reset to CMD state.\n" );
        g_pGame->GetDungeon()->DisturbPlayer();
        ResetToState( STATE_COMMAND );
    }
    g_pGame->SetReadyForUpdate( true );
    return true;
}
