//
//  EndGameState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#include "EndGameState.h"

#include "DungeonTile.h"
#include "DisplayText.h"
#include "Game.h"

#include "Dungeon.h"
#include "Player.h"

extern CGame *g_pGame;

CEndGameState::CEndGameState()
: m_cCommand(0),
m_szTombstone(NULL)
{
    m_pKeyHandlers[ENDGAME_INIT]    = &CEndGameState::OnHandleInit;
    m_pKeyHandlers[ENDGAME_TOMB]    = &CEndGameState::OnHandleTomb;
    m_pKeyHandlers[ENDGAME_SCORES]    = &CEndGameState::OnHandleScores;
    
    m_eCurModifier = ENDGAME_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    char tombstone[] = \
    "\n\n\n\n\n             /----------\\    \n            /            \\   \n           /              \\  \n          /  Rest In Peace \\ \n          |                 | \n          |%17s| \n          |                 | \n          |Level %2d %8s| \n          |                 | \n          |                 | \n          |died on level %3d| \n          |                 | \n          |   killed by a   | \n          |                 | \n          |%17s| \n          |                 | \n          |_________________| \n";
    m_szTombstone = new char[strlen(tombstone)+1];
    memset(m_szTombstone, 0, strlen(tombstone)+1);
    strcpy(m_szTombstone, tombstone);
}

CEndGameState::~CEndGameState()
{
    if( m_szTombstone )
    {
        delete [] m_szTombstone;
        m_szTombstone = NULL;
    }
}

int CEndGameState::OnHandleKey(SDL_Keysym *keysym)
{
    int retval;
    retval = ((*this).*(m_pCurKeyHandler))(keysym);
    return retval;
}

int CEndGameState::OnHandleTomb( SDL_Keysym *keysym )
{
    int retval;
    printf( "Handling TOMB modifier\n" );
    retval = OnBaseHandleKey( keysym );
    
    if( retval == JRESETSTATE )
    {
        return 0;
    }
    
    if( retval == JCOMPLETESTATE )
    {
        printf( "TOMB modifier complete, ENDGAME state to SCORES\n");
        g_pGame->GetEnd()->Clear();
        DoScores();
        m_eCurModifier = ENDGAME_SCORES;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    }
    
    if( retval != JSUCCESS )
    {
        printf( "Name cmd still waiting for a valid key.\n" );
        return 0;
    }
    
    // We got a valid key
    printf( "TOMB modifier got a valid key\n" );
    g_pGame->GetEnd()->Clear();
    DoTomb();
    
    return 0;
}

int CEndGameState::OnHandleScores( SDL_Keysym *keysym )
{
    int retval;
    printf( "Handling SCORES modifier\n" );
    retval = OnBaseHandleKey( keysym );
    
    if( retval == JRESETSTATE )
    {
        return 0;
    }
    
    if( retval == JCOMPLETESTATE )
    {
        printf( "SCORES modifier complete, ENDGAME state to INIT\n");
        g_pGame->Quit(0);
    }
    
    if( retval != JSUCCESS )
    {
        printf( "Name cmd still waiting for a valid key.\n" );
        return 0;
    }
    
    // We got a valid key
    printf( "SCORES modifier got a valid key\n" );
    g_pGame->GetEnd()->Clear();
    DoScores();
    
    return 0;
}

int CEndGameState::OnHandleInit( SDL_Keysym *keysym )
{
    printf( "Initializing endgame state...\n" );
    
    g_pGame->GetEnd()->Clear();
    DoTomb();
    m_eCurModifier = ENDGAME_TOMB;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    return 0;
}

int CEndGameState::OnBaseHandleKey( SDL_Keysym *keysym )
{
    if( keysym->sym == SDLK_RETURN || keysym->sym == SDLK_SPACE )
    {
        return JCOMPLETESTATE;
    }
    
    return -1;
}

void CEndGameState::ResetToState( int newstate )
{
    g_pGame->SetState(newstate);
    m_cCommand = NULL;
    m_eCurModifier = ENDGAME_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}


//////////////////////////////////////
/// command-specific fcns go below

//// Tomb commands
bool CEndGameState::DoTomb()
{
    g_pGame->GetEnd()->Printf(m_szTombstone,
                              g_pGame->GetPlayer()->GetName(),
                              (int)g_pGame->GetPlayer()->GetLevel(),
                              g_pGame->GetPlayer()->GetClass()->m_szName,
                              g_pGame->GetDungeon()->depth,
                              g_pGame->GetPlayer()->m_szKilledBy );
    return true;
}

//// Scores commands
bool CEndGameState::DoScores()
{
    // append this numeral to the running string (only send it back when complete)
    g_pGame->GetEnd()->Printf("High Score List\n");
    return true;
}
