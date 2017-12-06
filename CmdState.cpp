#include "CmdState.h"

#include "JMDefs.h"
#include "DungeonTile.h"
#include "DisplayText.h"
#include "Game.h"
#include "Player.h"
#include "Dungeon.h"

extern CGame *g_pGame;

int CCmdState::OnHandleKey( SDL_Keysym *keysym )
{
	if( IsDirectional( keysym ) )
	{
		JVector vTestDir(0,0);
		int dwCollideType;

		GetDir( keysym, vTestDir );
		m_vNewPos = g_pGame->GetPlayer()->m_vPos + vTestDir;

		dwCollideType = TestCollision(m_vNewPos);
		if( dwCollideType == DUNG_COLL_NO_COLLISION )
		{
			UpdatePlayerPos( m_vNewPos );
		}
		else
		{
			HandleCollision( dwCollideType );
		}
		return 0;
	}
	// Not a directional key; check for other commands
	// (many will induce state changes)

	// Some commands need a directional modifier:
	// Open, Tunnel, Close, Run, Bash, Look, Search, Fire, Hurl, Disarm
	if( IsModifierNeeded( keysym ) )
	{
		// Add "modify" to the top of the state stack
		g_pGame->SetState(STATE_MODIFY);
		g_pGame->GetGameState()->HandleKey(keysym);
		return 0;
	}
    
    switch(keysym->sym)
    {
    case SDLK_COMMA:
    case SDLK_PERIOD:
        if( keysym->mod & KMOD_SHIFT )
        {
            m_vNewPos = g_pGame->GetPlayer()->m_vPos;
            OnHandleStairs( keysym );
            return JSUCCESS;
        }
        break;
    default:
        break;
    }

	/*
	// These commands will bring up a "menu"
	// Inventory, Equipment
	if( IsMenuCommand( keysym ) )
	{
		newState = STATE_MENU;
		newState.cmd(keysym);
		return;
	}
	
	// The following commands work with spellbooks:
	// Magic/Music, Pray/Play, Browse (special case: doesn't cast anything)
	// Note, each of these is class-specific.
	if( IsMagicCommand( keysym ) )
	{
		newState = STATE_SPELL;
		newState.cmd(keysym);
		return;
	}

	// These commands take an item (from inventory) to use:
	// Wear/Wield, Put on, Take off, Quaff, Read, Aim, Use, Apply, Eat, Remove, Drop
	if( IsUseCommand( keysym ) )
	{
		newState = STATE_USE;
		newState.cmd(keysym);
		return;
	}

	// These commands put the user into some part of the help system:
	// Help, Level Info, Weapon Info, Armor Info, etc.
	if( IsHelpCommand( keysym ) )
	{
		newState = STATE_HELP;
		newState.cmd(keysym);
		return;
	}

	if( IsRestCommand( keysym ) )
	{
		newState = STATE_REST;
		newState.cmd( keysym );
		return;
	}

	// The following commands don't induce state changes;
	// They're one-shot deals, baby.
	switch( keysym->sym )
	{
	case CMD_SWAPWEAPONS:
		DoSwapWeapons();
	break;
	case CMD_SEARCHMODE:
		g_Searching = true;
	break;
	default:
		printf( "Press ? for help.\n" );
	break;
	}/**/

	// If you haven't handled the key by now, 
	// it's an invalid key, so return an error.
	return -1;
}

void CCmdState::HandleCollision( int dwCollideType )
{
	// There are only 2 things to collide with; 
	// Monsters and Walls.
	if( IsMonster( dwCollideType ) )
	{
		char szStatus[16];
		char *szMonster;
		float fRoll = 0.0f;
		float fDamageMult = 1.0f;
		bool bHit;
		// When Players Attack
		CMonster *pMon = g_pGame->GetDungeon()->GetTile(m_vNewPos)->m_pCurMonster;
		szMonster = pMon->GetName();
		
		bHit = pMon->Hit(fRoll);
		if( bHit )
		{
			sprintf( szStatus, "hit" );
		}
		else
		{
			sprintf( szStatus, "miss" );
		}
		
		g_pGame->GetMsgs()->Printf( "You %s the %s.\n", szStatus, szMonster );

		if(bHit)
		{
			if( fRoll > 80.0f )
			{		
				g_pGame->GetMsgs()->Printf( "(It was an excellent hit! (x2 damage)\n" );
				fDamageMult = 2.0f;
			}
			
			if( pMon->Damage(fDamageMult) == STATUS_DEAD )
			{
				sprintf( szStatus, "have slain" );			
				g_pGame->GetMsgs()->Printf( "You %s the %s.\n", szStatus, szMonster );
				g_pGame->GetDungeon()->RemoveMonster(pMon);
			}
		}
	}
	else
	{
		// Ouch, you bumped into a %s.
		char what[16];
		switch( dwCollideType )
		{
		case DUNG_IDX_WALL:
			sprintf( what, "a wall" );
			break;
		case DUNG_IDX_DOOR:
			sprintf( what, "a door" );
			break;
		case DUNG_IDX_RUBBLE:
			sprintf( what, "some rubble" );
			break;
		default:
			sprintf( what, "um, something?" );
			break;
		}
		g_pGame->GetMsgs()->Printf( "Ouch! You bumped into %s!\n", what );
	}
}

bool CCmdState::IsDirectional(SDL_Keysym *keysym)
{
	// All the movement keys 
	// (arrows and numberpad keys)
	// have sequential SDLK_ symbols,
	// so we can handle them with this check
	if( ( keysym->sym >= SDLK_KP_1  && keysym->sym <= SDLK_KP_0 ) ||
        ( keysym->sym >= SDLK_RIGHT && keysym->sym <= SDLK_UP ) )
	{
		return true;
	}

	return false;
}

void CCmdState::GetDir(SDL_Keysym *keysym, JVector &vDir)
{
	switch( keysym->sym )
	{
	case SDLK_UP:
	case SDLK_KP_8:
		//up
		vDir.y = -1;
		break;
	case SDLK_DOWN:
	case SDLK_KP_2:
		//down
		vDir.y = 1;
		break;
	case SDLK_LEFT:
	case SDLK_KP_4:
		//left
		vDir.x = -1;
		break;
	case SDLK_RIGHT:
	case SDLK_KP_6:
		vDir.x = 1;
		//right
		break;
	case SDLK_KP_7:
		//up + left
		vDir.x = -1;
		vDir.y = -1;
		break;
	case SDLK_KP_9:
		//up + right
		vDir.x =  1;
		vDir.y = -1;
		break;
	case SDLK_KP_1:
		//down + left
		vDir.x = -1;
		vDir.y =  1;
		break;
	case SDLK_KP_3:
		//down + right
		vDir.x = 1;
		vDir.y = 1;
		break;
	case SDLK_KP_5:
		// rest
		break;
	case SDLK_KP_0:
	default:
		// nothing
		break;
		
	}
}

int CCmdState::TestCollision( JVector &vTest )
{
	return (g_pGame->GetDungeon()->IsWalkableFor(vTest, true));
}

void CCmdState::UpdatePlayerPos( JVector &vNewPos )
{
	g_pGame->GetPlayer()->m_vPos = vNewPos;
}

bool CCmdState::IsModifierNeeded(SDL_Keysym *keysym)
{
	switch( keysym->sym )
	{
	case SDLK_o:
	case SDLK_t:
	case SDLK_c:
		return true;
		break;
	default:
		return false;
		break;
	}

	return false;
}


#define DIR_UP 4
#define DIR_DOWN 5

int CCmdState::OnHandleStairs( SDL_Keysym *keysym )
{
    int stair_dir = TestStairs();
    if( stair_dir == DUNG_IDX_INVALID )
    {
        g_pGame->GetMsgs()->Printf("I do not see any stairs here.\n");
        return JSUCCESS;
    }
    
    // if on <, go up stairs
    if( keysym->sym == SDLK_COMMA && stair_dir == DUNG_IDX_UPSTAIRS )
    {
        g_pGame->GetMsgs()->Printf("You enter a maze of up staircases.\n");
        // dungeon_level--, make sure not to go less than 0
        // respawn new dungeon level
        return JSUCCESS;
    }
    else if( keysym->sym == SDLK_COMMA && stair_dir == DUNG_IDX_LONG_UPSTAIRS )
    {
        g_pGame->GetMsgs()->Printf("You enter a long maze of up staircases.\n");
        // dungeon_level-- (a bunch), make sure not to go less than 0
        // respawn new dungeon level
        return JSUCCESS;
    }
    // if on >, go down stairs
    else if( keysym->sym == SDLK_PERIOD && stair_dir == DUNG_IDX_DOWNSTAIRS )
    {
        g_pGame->GetMsgs()->Printf("You enter a maze of down staircases.\n");
        // dungeon_level++
        // respawn new dungeon level
        return JSUCCESS;
    }
    else if( keysym->sym == SDLK_PERIOD && stair_dir == DUNG_IDX_LONG_DOWNSTAIRS )
    {
        g_pGame->GetMsgs()->Printf("You enter a long maze of down staircases.\n");
        // dungeon_level++ (a bunch)
        // respawn new dungeon level
        return JSUCCESS;
    }
    else
    {
        g_pGame->GetMsgs()->Printf("You can't do that here.\n");
        return JSUCCESS;
    }
}

int CCmdState::TestStairs()
{
    return( g_pGame->GetDungeon()->IsStairs(m_vNewPos) );
}

