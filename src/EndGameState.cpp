//
//  EndGameState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#include "EndGameState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"

#include "FileParse.h"

#define TOMBSTONE_FIELD_WIDTH 17

extern CGame *g_pGame;

CEndGameState::CEndGameState() : m_cCommand( 0 ), m_szTombstone( NULL )
{
    m_pKeyHandlers[ENDGAME_INIT] = &CEndGameState::OnHandleInit;
    m_pKeyHandlers[ENDGAME_TOMB] = &CEndGameState::OnHandleTomb;
    m_pKeyHandlers[ENDGAME_MAP] = &CEndGameState::OnHandleMap;
    m_pKeyHandlers[ENDGAME_INVENTORY] = &CEndGameState::OnHandleInventory;
    m_pKeyHandlers[ENDGAME_SCORES] = &CEndGameState::OnHandleScores;

    m_eCurModifier = ENDGAME_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    char tombstone[] =
        "\n\n\n\n\n             /----------\\    \n            /            \\   \n           /    "
        "          \\  \n          /  Rest In Peace \\ \n          |                 | \n          "
        "|%*s%*s| \n          |                 | \n          |Level %2d %8s| \n          |        "
        "         | \n          |                 | \n          |died on level %3d| \n          |  "
        "               | \n          |   killed by a   | \n          |                 | \n       "
        "   |%*s%*s| \n          |%*s%*s| \n          |_________________| \n";
    m_szTombstone = new char[Util::jstrlen( tombstone ) + 1];
    memset( m_szTombstone, 0, Util::jstrlen( tombstone ) + 1 );
    Util::jstrcpy( m_szTombstone, tombstone );

    m_pScore = new CScore;
}

CEndGameState::~CEndGameState()
{
    if( m_szTombstone )
    {
        delete[] m_szTombstone;
        m_szTombstone = NULL;
    }
    if( m_pScore )
    {
        delete m_pScore;
        m_pScore = NULL;
    }
}

int CEndGameState::OnHandleKey( JKeysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CEndGameState::OnHandleTomb( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling TOMB modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true, "TOMB modifier complete, ENDGAME state to MAP\n" );
        g_pGame->GetEnd()->Clear();
        DoMap();
        m_eCurModifier = ENDGAME_MAP;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true, "Name cmd still waiting for a valid key.\n" );
        return 0;
    }

    // We got a valid key
    JLog( LOG_LEVEL_NOISE, true, "TOMB modifier got a valid key\n" );
    g_pGame->GetEnd()->Clear();
    DoTomb();

    return 0;
}

int CEndGameState::OnHandleMap( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling MAP modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true, "MAP modifier complete, ENDGAME state to INVENTORY\n" );
        g_pGame->GetEnd()->Clear();
        DoInventory();
        m_eCurModifier = ENDGAME_INVENTORY;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true, "Map cmd still waiting for a valid key.\n" );
        return 0;
    }

    return 0;
}

int CEndGameState::OnHandleScores( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling SCORES modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true, "SCORES modifier complete, ENDGAME state to INIT\n" );
        g_pGame->Quit( 0 );
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true, "Name cmd still waiting for a valid key.\n" );
        return 0;
    }

    // We got a valid key
    JLog( LOG_LEVEL_NOISE, true, "SCORES modifier got a valid key\n" );
    g_pGame->GetEnd()->Clear();
    DoScores();

    return 0;
}

int CEndGameState::OnHandleInit( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing endgame state...\n" );

    g_pGame->GetPlayer()->IdentifyAllInventory();
    m_pScore->InitScore();

    g_pGame->GetEnd()->Clear();
    DoTomb();
    m_eCurModifier = ENDGAME_TOMB;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    return 0;
}

int CEndGameState::OnBaseHandleKey( JKeysym *keysym )
{
    if( keysym->sym == JKEY_RETURN || keysym->sym == JKEY_SPACE )
    {
        return JCOMPLETESTATE;
    }

    return -1;
}

void CEndGameState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = ENDGAME_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

//////////////////////////////////////
/// command-specific fcns go below

//// Tomb commands
bool CEndGameState::DoTomb()
{
    int dwNameLen = Util::jstrlen( m_pScore->m_szName );
    int dwNamePadding = MAX( 0, ( TOMBSTONE_FIELD_WIDTH - dwNameLen ) / 2 );
    int dwNameExtraPad = ( dwNameLen % 2 == 0 ) ? 1 : 0;

    // Split killer name across two lines if needed
    char szKiller1[TOMBSTONE_FIELD_WIDTH + 1];
    char szKiller2[TOMBSTONE_FIELD_WIDTH + 1];
    memset( szKiller1, 0, sizeof( szKiller1 ) );
    memset( szKiller2, 0, sizeof( szKiller2 ) );

    int dwKillerLen = Util::jstrlen( m_pScore->m_szKilledBy );
    if( dwKillerLen > TOMBSTONE_FIELD_WIDTH )
    {
        strncpy( szKiller1, m_pScore->m_szKilledBy, TOMBSTONE_FIELD_WIDTH );
        szKiller1[TOMBSTONE_FIELD_WIDTH] = '\0';
        int remainder = dwKillerLen - TOMBSTONE_FIELD_WIDTH;
        if( remainder > TOMBSTONE_FIELD_WIDTH )
            remainder = TOMBSTONE_FIELD_WIDTH;
        strncpy( szKiller2, m_pScore->m_szKilledBy + TOMBSTONE_FIELD_WIDTH, remainder );
        szKiller2[remainder] = '\0';
    }
    else
    {
        Util::jstrcpy( szKiller1, m_pScore->m_szKilledBy );
    }

    int dwKiller1Len = Util::jstrlen( szKiller1 );
    int dwKiller1Padding = MAX( 0, ( TOMBSTONE_FIELD_WIDTH - dwKiller1Len ) / 2 );
    int dwKiller1ExtraPad = ( dwKiller1Len % 2 == 0 ) ? 1 : 0;

    int dwKiller2Len = Util::jstrlen( szKiller2 );
    int dwKiller2Padding = MAX( 0, ( TOMBSTONE_FIELD_WIDTH - dwKiller2Len ) / 2 );
    int dwKiller2ExtraPad = ( dwKiller2Len % 2 == 0 ) ? 1 : 0;

    g_pGame->GetEnd()->Printf(
        m_szTombstone, dwNamePadding + dwNameLen, m_pScore->m_szName,
        dwNamePadding + dwNameExtraPad, "", m_pScore->m_dwLevel, m_pScore->m_szClass,
        m_pScore->m_dwDepth, dwKiller1Padding + dwKiller1Len, szKiller1,
        dwKiller1Padding + dwKiller1ExtraPad, "", dwKiller2Padding + dwKiller2Len, szKiller2,
        dwKiller2Padding + dwKiller2ExtraPad, "" );
    return true;
}

int CEndGameState::OnHandleInventory( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling INVENTORY modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true, "INVENTORY modifier complete, ENDGAME state to SCORES\n" );
        g_pGame->GetEnd()->Clear();
        if( g_pGame->GetPlayer()->IsWizard() )
        {
            JLog( LOG_LEVEL_WARN, true, "*** Wizard Mode: On *** Score not recorded.\n" );
            g_pGame->Quit( 0 );
        }
        InitScores();
        DoScores();
        m_eCurModifier = ENDGAME_SCORES;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
    }

    return 0;
}

//// Map commands
bool CEndGameState::DoMap()
{
    char *map = g_pGame->GetDungeon()->DumpMap();
    g_pGame->GetEnd()->Printf( "Dungeon Level %d (%d ft)\n\n", g_pGame->GetDungeon()->depth,
                               g_pGame->GetDungeon()->depth * 50 );
    g_pGame->GetEnd()->Printf( "%s", map );
    delete[] map;
    return true;
}

bool CEndGameState::DoInventory()
{
    CPlayer *pPlayer = g_pGame->GetPlayer();
    g_pGame->GetEnd()->Printf( "Inventory at time of death:\n\n" );

    CLink<CItem> *pLink = pPlayer->m_llInventory->GetHead();
    char cId = 'a';
    if( pLink == NULL )
        g_pGame->GetEnd()->Printf( "  (none)\n" );
    while( pLink != NULL )
    {
        CItem *pItem = pLink->m_lpData;
        if( pItem->IsStackable() && pItem->m_dwCount > 1 )
            g_pGame->GetEnd()->Printf( "  %c - %d %s\n", cId, pItem->m_dwCount,
                                       pItem->GetPlural() );
        else
            g_pGame->GetEnd()->Printf( "  %c - %s\n", cId, pItem->GetName() );
        cId++;
        pLink = pPlayer->m_llInventory->GetNext( pLink );
    }

    g_pGame->GetEnd()->Printf( "\nEquipment:\n\n" );

    pLink = pPlayer->m_llEquipment->GetHead();
    if( pLink == NULL )
        g_pGame->GetEnd()->Printf( "  (none)\n" );
    while( pLink != NULL )
    {
        CItem *pItem = pLink->m_lpData;
        if( pItem )
            g_pGame->GetEnd()->Printf( "  %s\n", pItem->GetName() );
        pLink = pPlayer->m_llEquipment->GetNext( pLink );
    }

    return true;
}

bool CEndGameState::InitScores()
{
    // call FileParse::Append to add new score (to end of file)
    CDataFile dfScores;
    if( m_pScore->m_dwScore > 0 )
    {
        dfScores.Append( "Resources/Scores.txt" );
        dfScores.WriteScore( m_pScore );
        dfScores.Close();
    }

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
    dfScores.Open( "Resources/Scores.txt" );

    ps = new CScore;
    while( dfScores.ReadScore( *ps ) )
    {
        ps->InitToString();
        m_llScores->Add( ps, ps->m_dwScore, -1, false );
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
    sprintf( meta.header, "High Score List\n" );
    meta.limit = 25;
    sprintf( meta.footer, "Scores past first page not shown.\n" );
    g_pGame->GetEnd()->DisplayList( m_llScores, &meta );
    // copy PLayer::DisplayInventory (or similar) to list the first N scores. Number them 1-N
    // make sure to display the current player's score, with its rank number, even if it's not on
    // the first page.

    return true;
}
