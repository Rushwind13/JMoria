//
//  IntroState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 11/18/24.
//  Copyright © 2024 Jimbo S. Harris. All rights reserved.
//

#include "IntroState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"

#include "FileParse.h"

extern CGame *g_pGame;

CIntroState::CIntroState() : m_szSplash( NULL ), m_cCommand( 0 )
{
    m_pKeyHandlers[INTRO_INIT] = &CIntroState::OnHandleInit;
    m_pKeyHandlers[INTRO_SPLASH] = &CIntroState::OnHandleSplash;
    m_pKeyHandlers[INTRO_CREATE] = &CIntroState::OnHandleCharacterCreate;

    m_eCurModifier = INTRO_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    char splash[] = "\n\n\n\n\n"
                    "             (**********)    \n"
                    "            (            )   \n"
                    "           (              )  \n"
                    "          (                ) \n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [                  ]\n"
                    "         [__________________]\n"
                    "         [__________________]\n";
    m_szSplash = new char[Util::jstrlen( splash ) + 1];
    memset( m_szSplash, 0, Util::jstrlen( splash ) + 1 );
    Util::jstrcpy( m_szSplash, splash );
}

CIntroState::~CIntroState()
{
    if( m_szSplash )
    {
        delete[] m_szSplash;
        m_szSplash = NULL;
    }
}

int CIntroState::OnHandleKey( SDL_Keysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CIntroState::OnHandleSplash( SDL_Keysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling SPLASH modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true, "INTRO modifier complete, INTRO state to CREATE\n" );

        g_pGame->GetEnd()->Clear();
        DoCharacterCreation();
        m_eCurModifier = INTRO_CREATE;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_INFO, true, "INTRO still waiting for a valid key.\n" );
        return 0;
    }

    // We got a valid key
    JLog( LOG_LEVEL_NOISE, true, "INTRO modifier got a valid key\n" );
    g_pGame->GetEnd()->Clear();
    DoSplash();

    return 0;
}

int CIntroState::OnHandleCharacterCreate( SDL_Keysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling CREATE modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true, "CREATE modifier complete, INTRO state to CMD\n" );
        ResetToState( STATE_COMMAND );
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_INFO, true, "CREATE still waiting for a valid key.\n" );
        return 0;
    }

    // We got a valid key
    JLog( LOG_LEVEL_NOISE, true, "CREATE modifier got a valid key\n" );
    g_pGame->GetEnd()->Clear();

    return 0;
}

int CIntroState::OnHandleInit( SDL_Keysym *keysym )
{
    JLog( LOG_LEVEL_INFO, true, "Initializing intro state...\n" );

    g_pGame->GetEnd()->Clear();
    DoSplash();
    m_eCurModifier = INTRO_SPLASH;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    return 0;
}

int CIntroState::OnBaseHandleKey( SDL_Keysym *keysym )
{
    if( keysym->sym == SDLK_RETURN || keysym->sym == SDLK_SPACE )
    {
        return JCOMPLETESTATE;
    }

    return -1;
}

void CIntroState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = INTRO_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

//////////////////////////////////////
/// command-specific fcns go below

//// Splash commands
bool CIntroState::DoSplash()
{
    g_pGame->GetEnd()->Printf( m_szSplash );
    return true;
}
//// Create commands
bool CIntroState::DoCharacterCreation()
{
    g_pGame->GetEnd()->Printf( "Character Creation Screen goes here...\n" );
    return true;
}
