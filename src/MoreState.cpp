//
//  MoreState.cpp
//  JMoria
//
//  Created May 25, 2026.
//

#include "MoreState.h"

#include "Constants.h"
#include "DisplayText.h"
#include "Game.h"

extern CGame *g_pGame;

int CMoreState::OnHandleKey( JKeysym * /*keysym*/ )
{
    g_pGame->GetMsgs()->AdvancePage();
    return 0;
}

void CMoreState::ResetToState( int newstate ) { g_pGame->SetState( newstate ); }
