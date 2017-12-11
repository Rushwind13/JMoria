#include "UseState.h"

#include "JMDefs.h"
#include "DungeonTile.h"
#include "DisplayText.h"
#include "Game.h"

#include "Dungeon.h"
#include "Player.h"

extern CGame *g_pGame;

CUseState::CUseState()
: m_cCommand(0)
{
    m_pKeyHandlers[MENU_INIT]    = &CUseState::OnHandleInit;
	m_pKeyHandlers[MENU_WIELD]	= &CUseState::OnHandleWield;
	m_pKeyHandlers[MENU_REMOVE]= &CUseState::OnHandleRemove;
	m_pKeyHandlers[MENU_DROP]= &CUseState::OnHandleDrop;

	m_eCurModifier = MENU_INIT;
	m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

int CUseState::OnHandleKey(SDL_Keysym *keysym)
{
	int retval;
	retval = ((*this).*(m_pCurKeyHandler))(keysym);
	return retval;
}

int CUseState::OnHandleWield( SDL_Keysym *keysym )
{
	int retval;
	printf( "Handling WIELD \n" );
	retval = OnBaseHandleKey( keysym, MENU_WIELD );

	if( retval == JRESETSTATE )
	{
		return 0;
	}

	if( retval != JSUCCESS )
	{
		printf( "Use cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
		g_pGame->GetMsgs()->Printf("Choose an item from inventory(a to z):\n");
		return 0;
	}

	// We got a alpha key; do a "wield" of that item
	printf( "WIELD  got a selection\n" );
	if( TestWield() )
	{
		if( DoWield() )
		{
			g_pGame->GetMsgs()->Printf("You are now wielding the %s.\n", m_pSelected->m_lpData->GetName());
		}
		else
		{
			g_pGame->GetMsgs()->Printf("The %s slips from your fingers and returns to your pack!\n", m_pSelected->m_lpData->GetName());
		}
	}
	else
	{
		g_pGame->GetMsgs()->Printf("You can't wield a %s!\n", m_pSelected->m_lpData->GetName());
	}
    m_pSelected = NULL;

	printf( "WIELD  resetting game state to COMMAND, WIELD state to INIT\n");
	// One way or another, we're done with this state now.
	ResetToState( STATE_COMMAND );
	return 0;
}

int CUseState::OnHandleRemove( SDL_Keysym *keysym )
{
	int retval;
	printf( "Handling REMOVE \n" );
	retval = OnBaseHandleKey( keysym, MENU_REMOVE );

	if( retval == JRESETSTATE )
	{
		return 0;
	}

    if( retval != JSUCCESS )
    {
        printf( "Use cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
        g_pGame->GetMsgs()->Printf("Choose an item from equipment(a to z):\n");
        return 0;
    }

    // We got a alpha key; do a "remove" of that item
    printf( "REMOVE  got a selection\n" );
    if( TestRemove() )
    {
        if( DoRemove() )
        {
            g_pGame->GetMsgs()->Printf("You take off the %s.\n", m_pSelected->m_lpData->GetName());
        }
        else
        {
            g_pGame->GetMsgs()->Printf("You can't remove that!\n");
        }
        m_pSelected = NULL;
    }
    else
    {
        g_pGame->GetMsgs()->Printf("The %s is welded to your body!\n", m_pSelected->m_lpData->GetName());
    }

    printf( "REMOVE  resetting game state to COMMAND, REMOVE state to INIT\n");
    // One way or another, we're done with this state now.
    ResetToState( STATE_COMMAND );
    return 0;
}

int CUseState::OnHandleDrop( SDL_Keysym *keysym )
{
	int retval;
	printf( "Handling DROP \n" );
	retval = OnBaseHandleKey( keysym, MENU_DROP );

	if( retval == JRESETSTATE )
	{
		return 0;
	}

	if( retval != JSUCCESS )
	{
		printf( "Use cmd still waiting for a alphabetic key: Alpha key not pressed.\n" );
		g_pGame->GetMsgs()->Printf("Choose an item from inventory(a to z):\n");
		return 0;
	}

	// We got a alpha key; do a "drop" of that item
	printf( "DROP  got a selection\n" );
	if( TestDrop() )
	{
		if( DoDrop() )
		{
			g_pGame->GetMsgs()->Printf("You dropped the %s.\n", m_pSelected->m_lpData->GetName());
		}
		else
		{
			g_pGame->GetMsgs()->Printf("The %s slips from your fingers and returns to your pack!\n", m_pSelected->m_lpData->GetName());
		}
	}
	else
	{
		g_pGame->GetMsgs()->Printf("You can't drop a %s here!\n", m_pSelected->m_lpData->GetName());
	}
    m_pSelected = NULL;

	printf( "WIELD resetting game state to COMMAND, USE state to INIT\n");
	// One way or another, we're done with this state now.
	ResetToState( STATE_COMMAND );
	return 0;
}

int CUseState::OnHandleInit( SDL_Keysym *keysym )
{
	printf( "Initializing USE state...\n" );
	if( !m_cCommand )
	{
		m_cCommand = keysym->sym;

		eUseModifier mod = MENU_INIT;
		switch(m_cCommand)
		{
		case SDLK_w:
			mod = MENU_WIELD;
            g_pGame->GetMsgs()->Printf("Wield which item? [a-z]\n");
			break;
		case SDLK_t:
			mod = MENU_REMOVE;
            g_pGame->GetMsgs()->Printf("Remove which item? [a-j]\n");
			break;
		case SDLK_d:
			mod = MENU_DROP;
            g_pGame->GetMsgs()->Printf("Drop which item? [a-j]\n");
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

	printf( "Error: tried to init USE state when it was already initted...\n" );
	ResetToState( STATE_COMMAND );
	// shouldn't get here
	return JRESETSTATE;
}

int CUseState::OnBaseHandleKey( SDL_Keysym *keysym, eUseModifier whichUse )
{
	if( IsAlpha( keysym ) )
	{
		m_pSelected = GetResponse(whichUse);
        if( m_pSelected == NULL )
        {
            g_pGame->GetMsgs()->Printf("Please select a valid item.\n");

            return -1;
        }

		return JSUCCESS;
	}
	else if( keysym->sym == SDLK_ESCAPE )
	{
		// ESC key gets us out of  mode
		ResetToState(STATE_COMMAND);
		return JRESETSTATE;
	}

	return -1;
}

void CUseState::ResetToState( int newstate )
{
	g_pGame->SetState(newstate);
	m_cCommand = NULL;
	m_eCurModifier = MENU_INIT;
	m_pCurKeyHandler = m_pKeyHandlers[m_eCurModifier];
}

// TODO: Why is this exact code copied from CCmdState?! -- 12.3.2017
bool CUseState::IsAlpha(SDL_Keysym *keysym)
{
    // All the alphabet keys
    // have sequential SDLK_ symbols,
    // so we can handle them with this check
    if( keysym->sym >= SDLK_a  && keysym->sym <= SDLK_z )
    {
        m_dwSelected = keysym->sym - SDLK_a;
        return true;
    }

    return false;
}

CLink<CItem> *CUseState::GetResponse(eUseModifier whichUse)
{
    JLinkList<CItem> *pList = NULL;
    switch( whichUse )
    {
        case MENU_WIELD:
            pList = g_pGame->GetPlayer()->m_llInventory;
            break;
        case MENU_REMOVE:
            pList = g_pGame->GetPlayer()->m_llEquipment;
            break;
        case MENU_DROP:
            pList = g_pGame->GetPlayer()->m_llInventory;
            break;
        default:
            printf("Can't get response for : %d\n", whichUse);
            return NULL;
            break;
    }
    CLink<CItem> *pLink = pList->GetLink(m_dwSelected, false);

    return pLink;
}

//////////////////////////////////////
/// command-specific fcns go below

//// Open commands
bool CUseState::TestWield()
{
	return g_pGame->GetPlayer()->IsWieldable(m_pSelected);
}

bool CUseState::DoWield()
{
    return g_pGame->GetPlayer()->Wield(m_pSelected);
}

//// Close commands
bool CUseState::TestRemove()
{
    return g_pGame->GetPlayer()->IsRemovable(m_pSelected);
}

bool CUseState::DoRemove()
{
    return g_pGame->GetPlayer()->Remove(m_pSelected);
}

//// Drop commands
bool CUseState::TestDrop()
{
    return g_pGame->GetPlayer()->CanDropHere();
}

bool CUseState::DoDrop()
{
    return g_pGame->GetPlayer()->Drop(m_pSelected->m_lpData);
}
