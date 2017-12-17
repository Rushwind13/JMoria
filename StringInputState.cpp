//
//  StringInputState.cpp
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#include "StringInputState.h"

#include "DungeonTile.h"
#include "DisplayText.h"
#include "Game.h"

#include "Dungeon.h"
#include "Player.h"

extern CGame *g_pGame;

CStringInputState::CStringInputState()
: m_cCommand(0)
{
    memset(m_szInput, 0, MAX_STRING_LENGTH);
    m_pKeyHandlers[SI_INIT]    = &CStringInputState::OnHandleInit;
    m_pKeyHandlers[SI_NAME]    = &CStringInputState::OnHandleName;
    m_pKeyHandlers[SI_HAGGLE]    = &CStringInputState::OnHandleHaggle;
    
    m_eCurModifier = SI_INIT;
    m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

int CStringInputState::OnHandleKey(SDL_Keysym *keysym)
{
    int retval;
    retval = ((*this).*(m_pCurKeyHandler))(keysym);
    return retval;
}

int CStringInputState::OnHandleName( SDL_Keysym *keysym )
{
    int retval;
    printf( "Handling NAME modifier\n" );
    retval = OnBaseHandleKey( keysym );
    
    if( retval == JRESETSTATE )
    {
        return 0;
    }
    
    if( retval == JCOMPLETESTATE )
    {
        printf( "NAME modifier resetting game state to COMMAND, NAME state to INIT\n");
        // One way or another, we're done with this state now.
        g_pGame->GetPlayer()->SetName(m_szInput);
        memset(m_szInput, 0, MAX_STRING_LENGTH);
        g_pGame->GetMsgs()->Clear();
        ResetToState( STATE_COMMAND );
    }
    
    if( retval != JSUCCESS )
    {
        printf( "Name cmd still waiting for a Alphanumeric key.\n" );
//        g_pGame->GetMsgs()->Printf("Direction(1 2 3 4 6 7 8 9):\n");
        return 0;
    }
    
    // We got a alpha key; append it to the name
    printf( "NAME modifier got a alpha\n" );
    g_pGame->GetMsgs()->Clear();
    g_pGame->GetMsgs()->Printf("Character Name: %s", m_szInput);
    
    return 0;
}

int CStringInputState::OnHandleHaggle( SDL_Keysym *keysym )
{
    int retval;
    printf( "Handling HAGGLE modifier\n" );
    retval = OnBaseHandleKey( keysym );
    
    if( retval == JRESETSTATE )
    {
        return 0;
    }
    
    if( retval != JSUCCESS )
    {
        printf( "HAGGLE cmd still waiting for a Numeric key.\n" );
        g_pGame->GetMsgs()->Printf("Enter a number.\n");
        return 0;
    }
    
    // We got a numeric key; add to haggle number
    printf( "HAGGLE modifier got a numeric\n" );
    if( TestHaggle() )
    {
        if( DoHaggle() )
        {
            g_pGame->GetMsgs()->Printf("Your offer: %s", m_szInput);
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
        printf( "HAGGLE modifier resetting game state to COMMAND, HAGGLE state to INIT\n");
        // One way or another, we're done with this state now.
        ResetToState( STATE_COMMAND );
    }
    return 0;
}

int CStringInputState::OnHandleInit( SDL_Keysym *keysym )
{
    printf( "Initializing modify state...\n" );
    if( !m_cCommand )
    {
        m_cCommand = keysym->sym;
        
        eStringInputModifier mod = SI_INIT;
        switch(m_cCommand)
        {
            case SDLK_n:
                mod = SI_NAME;
                g_pGame->GetMsgs()->Clear();
                g_pGame->GetMsgs()->Printf("Character Name: %s", m_szInput);
                break;
            case SDLK_p:
                mod = SI_HAGGLE;
                break;
           default:
                printf( "There seems to be some kind of mistake; I don't handle mod: %d\n", m_cCommand );
                ResetToState( STATE_COMMAND );
                return 0;
                break;
        }
        
        m_eCurModifier = mod;
        m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
        return 0;
    }
    
    printf( "Error: tried to init stringinput state when it was already initted...\n" );
    ResetToState( STATE_COMMAND );
    // shouldn't get here
    return JRESETSTATE;
}

int CStringInputState::OnBaseHandleKey( SDL_Keysym *keysym )
{
    char bInput = GetAlphaNumeric(keysym);
    if( bInput != nul )
    {
        if( strlen(m_szInput) < MAX_STRING_LENGTH-1 )
        {
            m_szInput[strlen(m_szInput)] = bInput;
        }
        return JSUCCESS;
    }
    else if( keysym->sym == SDLK_RETURN )
    {
        // actually set the string on the place
        printf("you entered: <%s>\n", m_szInput);
        return JCOMPLETESTATE;
    }
    else if( keysym->sym == SDLK_ESCAPE )
    {
        // ESC key gets us out of modify mode
        ResetToState(STATE_COMMAND);
        return JRESETSTATE;
    }
    
    return -1;
}

void CStringInputState::ResetToState( int newstate )
{
    g_pGame->SetState(newstate);
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
