//
//  StringInputState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#include "StringInputState.h"

#include "DisplayText.h"
#include "DungeonTile.h"
#include "Game.h"

#include "Dungeon.h"
#include "Player.h"

extern CGame *g_pGame;

CStringInputState::CStringInputState() : m_cCommand( 0 )
{
    memset( m_szInput, 0, MAX_STRING_LENGTH );
    m_pKeyHandlers[SI_INIT] = &CStringInputState::OnHandleInit;
    m_pKeyHandlers[SI_FLAG] = &CStringInputState::OnHandleFlag;
    m_pKeyHandlers[SI_ITEM] = &CStringInputState::OnHandleItem;
    m_pKeyHandlers[SI_MONSTER] = &CStringInputState::OnHandleMonster;
    m_pKeyHandlers[SI_NAME] = &CStringInputState::OnHandleName;
    m_pKeyHandlers[SI_HAGGLE] = &CStringInputState::OnHandleHaggle;

    m_eCurModifier = SI_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

int CStringInputState::OnHandleKey( JKeysym *keysym )
{
    int retval;
    retval = ( ( *this ).*( m_pCurKeyHandler ) )( keysym );
    return retval;
}

int CStringInputState::OnHandleName( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling NAME modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "NAME modifier resetting game state to COMMAND, NAME state to INIT\n" );
        // One way or another, we're done with this state now.
        g_pGame->GetPlayer()->SetName( m_szInput );
        memset( m_szInput, 0, MAX_STRING_LENGTH );
        g_pGame->GetMsgs()->Clear();
        ResetToState( STATE_COMMAND );
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true, "Name cmd still waiting for a Alphanumeric key.\n" );
        //        g_pGame->GetMsgs()->Printf("Direction(1 2 3 4 6 7 8 9):\n");
        return 0;
    }

    // We got a alpha key; append it to the name
    JLog( LOG_LEVEL_NOISE, true, "NAME modifier got a alpha\n" );
    g_pGame->GetMsgs()->Clear();
    g_pGame->GetMsgs()->Printf( "Character Name: %s", m_szInput );

    return 0;
}

int CStringInputState::OnHandleItem( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling ITEM modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "ITEM modifier resetting game state to COMMAND, ITEM state to INIT\n" );
        // One way or another, we're done with this state now.

        CItemDef *pid = g_pGame->GetDungeon()->GetItemDef( m_szInput );
        if( pid != NULL )
        {
            JVector vPos = g_pGame->GetPlayer()->m_vPos;
            CItem::CreateItem( pid, vPos );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "Unknown item: %s\n", m_szInput );
        }

        memset( m_szInput, 0, MAX_STRING_LENGTH );
        g_pGame->GetMsgs()->Clear();
        ResetToState( STATE_COMMAND );
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true, "ITEM cmd still waiting for a Alphanumeric key.\n" );
        return 0;
    }

    // We got a alpha key; append it to the name
    JLog( LOG_LEVEL_NOISE, true, "ITEM modifier got a alpha\n" );
    g_pGame->GetMsgs()->Clear();
    g_pGame->GetMsgs()->Printf( "Item Name: %s", m_szInput );

    return 0;
}

int CStringInputState::OnHandleFlag( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling FLAG modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "FLAG modifier resetting game state to COMMAND, FLAG state to INIT\n" );
        // One way or another, we're done with this state now.

        int dwFlag = g_Constants.LookupString( m_szInput );
        if( dwFlag != -1 )
        {
            JLog( LOG_LEVEL_INFO, true, "Setting flag %s = %d\n", m_szInput, dwFlag );
            g_pGame->GetPlayer()->SetIntrinsic( dwFlag );
        }

        memset( m_szInput, 0, MAX_STRING_LENGTH );
        g_pGame->GetMsgs()->Clear();
        ResetToState( STATE_COMMAND );
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true, "FLAG cmd still waiting for a Alphanumeric key.\n" );
        return 0;
    }

    // We got a alpha key; append it to the name
    JLog( LOG_LEVEL_NOISE, true, "FLAG modifier got a alpha\n" );
    g_pGame->GetMsgs()->Clear();
    g_pGame->GetMsgs()->Printf( "Flag Name: %s", m_szInput );

    return 0;
}

int CStringInputState::OnHandleMonster( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling MONSTER modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval == JCOMPLETESTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "MONSTER modifier resetting game state to COMMAND, MONSTER state to INIT\n" );
        // One way or another, we're done with this state now.

        CMonsterDef *pmd = g_pGame->GetDungeon()->GetMonsterDef( m_szInput );
        if( pmd != NULL )
        {
            JIVector vPos( VEC_EXPAND( g_pGame->GetPlayer()->m_vPos ) );
            CMonster::CreateMonster( pmd, vPos, true );
        }
        else
        {
            g_pGame->GetMsgs()->Printf( "Unknown monster: %s\n", m_szInput );
        }

        memset( m_szInput, 0, MAX_STRING_LENGTH );
        g_pGame->GetMsgs()->Clear();
        ResetToState( STATE_COMMAND );
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true, "MONSTER cmd still waiting for a Alphanumeric key.\n" );
        return 0;
    }

    // We got a alpha key; append it to the name
    JLog( LOG_LEVEL_NOISE, true, "MONSTER modifier got a alpha\n" );
    g_pGame->GetMsgs()->Clear();
    g_pGame->GetMsgs()->Printf( "Monster Name: %s", m_szInput );

    return 0;
}

int CStringInputState::OnHandleHaggle( JKeysym *keysym )
{
    int retval;
    JLog( LOG_LEVEL_DEBUG, true, "Handling HAGGLE modifier\n" );
    retval = OnBaseHandleKey( keysym );

    if( retval == JRESETSTATE )
    {
        return 0;
    }

    if( retval != JSUCCESS )
    {
        JLog( LOG_LEVEL_DEBUG, true, "HAGGLE cmd still waiting for a Numeric key.\n" );
        g_pGame->GetMsgs()->Printf( "Enter a number.\n" );
        return 0;
    }

    // We got a numeric key; add to haggle number
    JLog( LOG_LEVEL_NOISE, true, "HAGGLE modifier got a numeric\n" );
    if( TestHaggle() )
    {
        if( DoHaggle() )
        {
            g_pGame->GetMsgs()->Printf( "Your offer: %s", m_szInput );
        }
        else
        {
            // can't get here?
            //            g_pGame->GetMsgs()->Printf("You failed to close the door.\n");
        }
    }
    else
    {
        // can't get here? how to handle bad input?
        //        g_pGame->GetMsgs()->Printf("I do not see anything to close there.\n");
    }

    if( retval == JRESETSTATE )
    {
        JLog( LOG_LEVEL_DEBUG, true,
              "HAGGLE modifier resetting game state to COMMAND, HAGGLE state to INIT\n" );
        // One way or another, we're done with this state now.
        ResetToState( STATE_COMMAND );
    }
    return 0;
}

int CStringInputState::OnHandleInit( JKeysym *keysym )
{
    JLog( LOG_LEVEL_DEBUG, true, "Initializing modify state...\n" );
    if( !m_cCommand )
    {
        m_cCommand = keysym->sym;

        eStringInputModifier mod = SI_INIT;
        switch( m_cCommand )
        {
        case JKEY_n:
            mod = SI_NAME;
            g_pGame->GetMsgs()->Clear();
            g_pGame->GetMsgs()->Printf( "Character Name: %s", m_szInput );
            break;
        case JKEY_p:
            mod = SI_HAGGLE;
            break;
        case JKEY_f:
            mod = SI_FLAG;
            g_pGame->GetMsgs()->Clear();
            g_pGame->GetMsgs()->Printf( "Flag Name: %s", m_szInput );
            break;
        case JKEY_i:
            mod = SI_ITEM;
            g_pGame->GetMsgs()->Clear();
            g_pGame->GetMsgs()->Printf( "Item Name: %s", m_szInput );
            break;
        case JKEY_s:
            mod = SI_MONSTER;
            g_pGame->GetMsgs()->Clear();
            g_pGame->GetMsgs()->Printf( "Monster Name: %s", m_szInput );
            break;
        default:
            JLog( LOG_LEVEL_ERROR, true,
                  "There seems to be some kind of mistake; I don't handle mod: %d\n", m_cCommand );
            ResetToState( STATE_COMMAND );
            return 0;
            break;
        }

        m_eCurModifier = mod;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
        return 0;
    }

    JLog( LOG_LEVEL_ERROR, true,
          "Error: tried to init stringinput state when it was already initted...\n" );
    ResetToState( STATE_COMMAND );
    // shouldn't get here
    return JRESETSTATE;
}

int CStringInputState::OnBaseHandleKey( JKeysym *keysym )
{
    char bInput = GetAlphaNumeric( keysym );
    if( bInput != nul )
    {
        if( Util::jstrlen( m_szInput ) < MAX_STRING_LENGTH - 1 )
        {
            m_szInput[Util::jstrlen( m_szInput )] = bInput;
        }
        return JSUCCESS;
    }
    else if( keysym->sym == JKEY_DELETE || keysym->sym == JKEY_BACKSPACE )
    {
        m_szInput[Util::jstrlen( m_szInput ) - 1] = nul;
        return JSUCCESS;
    }
    else if( keysym->sym == JKEY_RETURN )
    {
        // actually set the string on the place
        JLog( LOG_LEVEL_DEBUG, true, "you entered: <%s>\n", m_szInput );
        return JCOMPLETESTATE;
    }
    else if( keysym->sym == JKEY_ESCAPE )
    {
        // ESC key gets us out of modify mode
        ResetToState( STATE_COMMAND );
        return JRESETSTATE;
    }

    return -1;
}

void CStringInputState::ResetToState( int newstate )
{
    g_pGame->SetState( newstate );
    m_cCommand = NULL;
    m_eCurModifier = SI_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

//////////////////////////////////////
/// command-specific fcns go below

//// Name commands
bool CStringInputState::TestName()
{
    // Make sure that it's a alphanumeric input
    return false;
}

bool CStringInputState::DoName()
{
    // append this letter to the running string (only set on user when complete?)
    return false;
}

//// Haggle commands
bool CStringInputState::TestHaggle()
{
    // Make sure that it's a numeric input
    return false;
}

bool CStringInputState::DoHaggle()
{
    // append this numeral to the running string (only send it back when complete)
    return false;
}

//// Haggle commands
bool CStringInputState::TestFlag()
{
    // Make sure that it's a numeric input
    return false;
}

bool CStringInputState::DoFlag()
{
    // append this numeral to the running string (only send it back when complete)
    return false;
}

//// Haggle commands
bool CStringInputState::TestItem()
{
    // Make sure that it's a numeric input
    return false;
}

bool CStringInputState::DoItem()
{
    // append this numeral to the running string (only send it back when complete)
    return false;
}

//// Haggle commands
bool CStringInputState::TestMonster()
{
    // Make sure that it's a numeric input
    return false;
}

bool CStringInputState::DoMonster()
{
    // append this numeral to the running string (only send it back when complete)
    return false;
}
