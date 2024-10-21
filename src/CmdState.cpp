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
    // If you haven't handled the key by the end of this function,
    // it's an invalid key, so return an error.
    int retval = -1;
	if( IsDirectional( keysym ) )
	 {
		JVector vTestDir(0, 0);
		int dwCollideType;

		GetDir( keysym, vTestDir );
		m_vNewPos = g_pGame->GetPlayer()->m_vPos + vTestDir;

		dwCollideType = TestCollision(m_vNewPos);
		if( dwCollideType == DUNG_COLL_NO_COLLISION )
		 {
			UpdatePlayerPos( m_vNewPos );
		}
        else if( dwCollideType == DUNG_COLL_ITEM )
        {
            UpdatePlayerPos( m_vNewPos );
            HandleCollision( dwCollideType );
        }
		else
		 {
			HandleCollision( dwCollideType );
		}
		retval = 0;
	}
	// Not a directional key; check for other commands
	// (many will induce state changes)

	// Some commands need a directional modifier:
	// Open, Tunnel, Close, Run, Bash, Look, Search, Fire, Hurl, Disarm
	else if( IsModifierNeeded( keysym ) )
	 {
		// Add "modify" to the top of the state stack
		g_pGame->SetState(STATE_MODIFY);
		g_pGame->GetGameState()->HandleKey(keysym);
		retval = 0;
	}

    // Use commands allow you to choose from lists of items:
    // inventory, equipment, stores, chests?, bag of holding, &c
    else if( IsUseCommand(keysym) )
    {
        // Add "modify" to the top of the state stack
        g_pGame->SetState(STATE_USE);
        g_pGame->GetGameState()->HandleKey(keysym);
        retval = 0;
    }

    // StringInput commands allow you to enter several characters as a single string
    else if( IsStringInputCommand(keysym) )
    {
        // Add "stringinput" to the top of the state stack
        g_pGame->SetState(STATE_STRINGINPUT);
        g_pGame->GetGameState()->HandleKey(keysym);
        retval = 0;
    }

    else if( IsStairsCommand(keysym) )
    {
        m_vNewPos = g_pGame->GetPlayer()->m_vPos;
        OnHandleStairs( keysym );
        retval = 0;
    }

    else if( IsRestCommand( keysym ) )
    {
        switch( keysym-> sym )
        {
            case SDLK_r:
                // printf("R)est not implemented yet.\n");
                g_pGame->SetState(STATE_REST);
                g_pGame->GetGameState()->HandleKey(keysym);
                break;
            case SDLK_PERIOD:
                // Do nothing; rest one turn
                break;
            default:
                break;
        }

        retval = 0;
    }

	/*
	// These commands will bring up a ""
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

	// These commands put the user into some part of the help system:
	// Help, Level Info, Weapon Info, Armor Info, etc.
	if( IsHelpCommand( keysym ) )
	{
		newState = STATE_HELP;
		newState.cmd(keysym);
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

#ifdef TURN_BASED
    if( retval != -1 )
    {
        g_pGame->SetReadyForUpdate(true);
    }
#endif // TURN_BASED
	return retval;
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


        fRoll = g_pGame->GetPlayer()->Attack();
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

            float fDamage = g_pGame->GetPlayer()->Damage(fDamageMult);

			if( pMon->TakeDamage(fDamage) == STATUS_DEAD )
			 {
				sprintf( szStatus, "have slain" );
				g_pGame->GetMsgs()->Printf( "You %s the %s.\n", szStatus, szMonster );
                g_pGame->GetPlayer()->OnKillMonster(pMon);
				g_pGame->GetDungeon()->RemoveMonster(pMon);
			}
		}
	}
    else if ( dwCollideType == DUNG_COLL_ITEM )
    {
        PickUpItem(m_vNewPos);
    }
	else
	 {
		// Ouch, you bumped into a %s.
		char what[16];
		switch( dwCollideType )
		 {
		case DUNG_IDX_WALL:
		case DUNG_IDX_SECRET_DOOR:
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

int CCmdState::TestCollision( JVector &vTest )
{
	return (g_pGame->GetDungeon()->IsWalkableFor(vTest, true));
}

void CCmdState::UpdatePlayerPos( JVector &vNewPos )
{
    g_pGame->GetPlayer()->m_vPos = vNewPos;
}

void CCmdState::PickUpItem( JVector &vNewPos )
{
    g_pGame->GetPlayer()->PickUp(vNewPos);
}

bool CCmdState::IsModifierNeeded(SDL_Keysym *keysym)
{
	switch( keysym->sym )
	 {
	case SDLK_o:
	case SDLK_c:
		return true;
		break;
  // T ( but not t )
	case SDLK_t:
    if( keysym->mod & KMOD_SHIFT )
    {
      return true;
    }
    break;
	default:
		return false;
		break;
	}

	return false;
}

bool CCmdState::IsUseCommand(SDL_Keysym *keysym)
{
    switch( keysym->sym )
    {
        case SDLK_w:
            // t (but not T)
        case SDLK_t:
        case SDLK_d:
        case SDLK_q:
            return true;
            break;
        default:
            return false;
            break;
    }

    return false;
}


bool CCmdState::IsStringInputCommand(SDL_Keysym *keysym)
{
    switch( keysym->sym )
    {
        case SDLK_n: // name your character
        case SDLK_p: // purchase something in a store
            return true;
            break;
        default:
            return false;
            break;
    }

    return false;
}

bool CCmdState::IsStairsCommand( SDL_Keysym *keysym )
{
    switch(keysym->sym)
    {
        case SDLK_COMMA:
        case SDLK_PERIOD:
            if( keysym->mod & KMOD_SHIFT )
            {
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

bool CCmdState::IsRestCommand(SDL_Keysym *keysym)
{
    switch( keysym->sym )
    {
    case SDLK_PERIOD:
      // want . not >
      return ( keysym->mod & KMOD_SHIFT ) ? false : true;
      break;
    case SDLK_r:
      // want R not r
      return ( keysym->mod & KMOD_SHIFT ) ? true : false;
      break;
    default:
      return false;
      break;
    }

    return false;
}


// Handlers
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
        g_pGame->GetDungeon()->OnChangeLevel(-Util::Roll(1, 5));
        return JSUCCESS;
    }
    else if( keysym->sym == SDLK_COMMA && stair_dir == DUNG_IDX_LONG_UPSTAIRS )
    {
        g_pGame->GetMsgs()->Printf("You enter a long maze of up staircases.\n");
        // dungeon_level-- (a bunch), make sure not to go less than 0
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel(-1);
        return JSUCCESS;
    }
    // if on >, go down stairs
    else if( keysym->sym == SDLK_PERIOD && stair_dir == DUNG_IDX_DOWNSTAIRS )
    {
        g_pGame->GetMsgs()->Printf("You enter a maze of down staircases.\n");
        // dungeon_level++
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel(1);
        return JSUCCESS;
    }
    else if( keysym->sym == SDLK_PERIOD && stair_dir == DUNG_IDX_LONG_DOWNSTAIRS )
    {
        g_pGame->GetMsgs()->Printf("You enter a long maze of down staircases.\n");
        // dungeon_level++ (a bunch)
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel(Util::Roll(1, 5));
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

void CCmdState::DisplayInventory()
{
    g_pGame->GetPlayer()->DisplayInventory(PLACEMENT_INV);
}
