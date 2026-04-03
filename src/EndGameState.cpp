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

#include "FileParse.h"

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
    "\n\n\n\n\n             /----------\\    \n            /            \\   \n           /              \\  \n          /  Rest In Peace \\ \n          |                 | \n          |%*s%*s| \n          |                 | \n          |Level %2d %8s| \n          |                 | \n          |                 | \n          |died on level %3d| \n          |                 | \n          |   killed by a   | \n          |                 | \n          |%*s%*s| \n          |                 | \n          |_________________| \n";
    m_szTombstone = new char[strlen(tombstone)+1];
    memset(m_szTombstone, 0, strlen(tombstone)+1);
    strcpy(m_szTombstone, tombstone);
    
    m_pScore = new CScore;
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
        InitScores();
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
    
    m_pScore->InitScore();

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
    int dwNamePadding = (17-strlen(m_pScore->m_szName))/2;
    int dwNameExtraPad = 0;
    if( strlen(m_pScore->m_szName) %2 == 0 ) dwNameExtraPad = 1;
    
    int dwKillerPadding = (17-strlen(m_pScore->m_szKilledBy))/2;
    int dwKillerExtraPad = 0;
    if( strlen(m_pScore->m_szKilledBy) %2 == 0 ) dwKillerExtraPad = 1;

    g_pGame->GetEnd()->Printf(m_szTombstone,
                              dwNamePadding+strlen(m_pScore->m_szName),
                              m_pScore->m_szName,
                              dwNamePadding+dwNameExtraPad,
                              "",
                              m_pScore->m_dwLevel,
                              m_pScore->m_szClass,
                              m_pScore->m_dwDepth,
                              dwKillerPadding+strlen(m_pScore->m_szKilledBy),
                              m_pScore->m_szKilledBy,
                              dwKillerPadding+dwKillerExtraPad,
                              "");
    return true;
}

bool CEndGameState::InitScores()
{
    // call FileParse::Append to add new score (to end of file)
    CDataFile dfScores;
    dfScores.Append("Resources/Scores.txt");
    dfScores.WriteScore(m_pScore);
    dfScores.Close();
    
    // Score entry should have:
    // Player name
    // User Name (login?)
    // Player level
    // Class
    // Race (tbd, obviously)
    // Dungeon level
    // KilledBy
    // Score (just the character XP at this point)
    // Date

    // call FileParse::ReadScores to read score list (read sorted into a LL)

    m_llScores = new JLinkList<CScore>;
    
    CScore *ps;
    dfScores.Open("Resources/Scores.txt");
    
    ps = new CScore;
    while( dfScores.ReadScore(*ps) )
    {
        ps->InitToString();
        m_llScores->Add(ps, ps->m_dwScore, false);
        ps = new CScore;
    }
    
    delete ps;
    return true;
}
//// Scores commands
bool CEndGameState::DoScores()
{
    // append this numeral to the running string (only send it back when complete)
    CDisplayMeta meta;
    sprintf( meta.header, "High Score List\n");
    meta.limit = 25;
    sprintf( meta.footer, "Scores past first page not shown.\n");
    g_pGame->GetEnd()->DisplayList( m_llScores, &meta );
    // copy PLayer::DisplayInventory (or similar) to list the first N scores. Number them 1-N
    // make sure to display the current player's score, with its rank number, even if it's not on the first page.

    return true;
}
