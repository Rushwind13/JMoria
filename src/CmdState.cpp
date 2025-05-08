#include "CmdState.h"

#include "DisplayText.h"
#include "Dungeon.h"
#include "DungeonTile.h"
#include "Game.h"
#include "JMDefs.h"
#include "Player.h"

extern CGame *g_pGame;

int CCmdState::OnHandleKey( SDL_Keysym *keysym )
{
    // If you haven't handled the key by the end of this function,
    // it's an invalid key, so return an error.
    int retval = -1;
    if( IsDirectional( keysym ) )
    {
        JVector vTestDir( 0, 0 );
        int dwCollideType;

        GetDir( keysym, vTestDir );

        // handle "run" movement
        if( keysym->mod & KMOD_SHIFT )
        {
            g_pGame->GetPlayer()->m_vVel = vTestDir;
            g_pGame->SetState( STATE_RUN );
            g_pGame->GetGameState()->HandleKey( keysym );
            retval = 0;
        }
        else // single step movement
        {
            g_pGame->GetPlayer()->Move( vTestDir );
            retval = 0;
        }
    }
    // Not a directional key; check for other commands
    // (many will induce state changes)

    // Some commands need a directional modifier:
    // Open, Tunnel, Close, Run, Bash, Look, Search, Fire, Hurl, Disarm
    else if( IsModifierNeeded( keysym ) )
    {
        // Add "modify" to the top of the state stack
        g_pGame->SetState( STATE_MODIFY );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

    // Use commands allow you to choose from lists of items:
    // inventory, equipment, stores, chests?, bag of holding, &c
    else if( IsUseCommand( keysym ) )
    {
        // Add "modify" to the top of the state stack
        g_pGame->SetState( STATE_USE );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

    // StringInput commands allow you to enter several characters as a single string
    else if( IsStringInputCommand( keysym ) )
    {
        // Add "stringinput" to the top of the state stack
        g_pGame->SetState( STATE_STRINGINPUT );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

    else if( IsLookCommand( keysym ) )
    {
        // Add "look" to the top of the state stack
        g_pGame->SetState( STATE_LOOK );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

    else if( IsTargetCommand( keysym ) )
    {
        // Add "target" to the top of the state stack
        g_pGame->SetState( STATE_TARGET );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

    else if( IsStairsCommand( keysym ) )
    {
        m_vNewPos = g_pGame->GetPlayer()->m_vPos;
        OnHandleStairs( keysym );
        retval = 0;
    }

    else if( IsRestCommand( keysym ) )
    {
        switch( keysym->sym )
        {
        case SDLK_r:
            JLog( LOG_LEVEL_WARN, true, "R)est not implemented yet.\n" );
            g_pGame->SetState( STATE_REST );
            g_pGame->GetGameState()->HandleKey( keysym );
            break;
        case SDLK_PERIOD:
            // Do nothing; rest one turn
            break;
        default:
            break;
        }

        retval = 0;
    }

    else if( IsZapCommand( keysym ) )
    {
        g_pGame->SetState( STATE_RANGED );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = -1;
    }

    // Wizard-mode commands

    else if( IsTeleportCommand( keysym ) )
    {
        g_pGame->GetPlayer()->SetWizard();
        g_pGame->GetPlayer()->m_bHasSpawned = false;
        g_pGame->GetPlayer()->SpawnPlayer();
        retval = 0;
    }

    else if( IsSetIntrinsicCommand( keysym ) )
    {
        g_pGame->SetState( STATE_STRINGINPUT );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

    else if( IsCreateItemCommand( keysym ) )
    {
        g_pGame->SetState( STATE_STRINGINPUT );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

    else if( IsSummonMonsterCommand( keysym ) )
    {
        g_pGame->SetState( STATE_STRINGINPUT );
        g_pGame->GetGameState()->HandleKey( keysym );
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
            JLog( LOG_LEVEL_WARN, true,  "Press ? for help.\n" );
    break;
    }/**/

#ifdef TURN_BASED
    if( retval != -1 )
    {
        g_pGame->SetReadyForUpdate( true );
    }
#endif // TURN_BASED
    return retval;
}

bool CCmdState::IsModifierNeeded( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
    case SDLK_c:
    case SDLK_o:
        return true;
        break;
        // T ( but not t  or ^t)
    case SDLK_t:
        if( keysym->mod & KMOD_SHIFT && !( keysym->mod & KMOD_CTRL ) )
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

bool CCmdState::IsUseCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
        // t (but not T or ^t)
    case SDLK_d:
    case SDLK_t:
    case SDLK_q:
    case SDLK_r:
    case SDLK_w:
    {
        if( !( keysym->mod & KMOD_SHIFT ) && !( keysym->mod & KMOD_CTRL ) )
        {
            return true;
        }
        break;
    }
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsStringInputCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
    case SDLK_n: // name your character
        if( keysym->mod & KMOD_SHIFT )
        {
            return true;
        }
        break;
    case SDLK_p: // purchase something in a store
        return true;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsLookCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
    case SDLK_SEMICOLON:
        if( keysym->mod & KMOD_SHIFT )
        {
            return true;
        }
    }
    return false;
}

bool CCmdState::IsTargetCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
    case SDLK_8:
        if( keysym->mod & KMOD_SHIFT )
        {
            return true;
        }
    }
    return false;
}

bool CCmdState::IsStairsCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
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

bool CCmdState::IsRestCommand( SDL_Keysym *keysym )
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

bool CCmdState::IsTeleportCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
    case SDLK_t:
        // want ^t not t
        return ( keysym->mod & KMOD_CTRL ) ? true : false;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsSetIntrinsicCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
    case SDLK_f:
        // want ^f not f
        return ( keysym->mod & KMOD_CTRL ) ? g_pGame->GetPlayer()->IsWizard() : false;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsZapCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
    case SDLK_z:
        if( keysym->mod == 0 )
        {
            return true;
        }
    }
    return false;
}

bool CCmdState::IsCreateItemCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
    case SDLK_i:
        // want ^t not t
        return ( keysym->mod & KMOD_CTRL ) ? g_pGame->GetPlayer()->IsWizard() : false;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsSummonMonsterCommand( SDL_Keysym *keysym )
{
    switch( keysym->sym )
    {
    case SDLK_s:
        // want ^t not t
        return ( keysym->mod & KMOD_CTRL ) ? g_pGame->GetPlayer()->IsWizard() : false;
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
        g_pGame->GetMsgs()->Printf( "I do not see any stairs here.\n" );
        return JSUCCESS;
    }

    // if on <, go up stairs
    if( keysym->sym == SDLK_COMMA && stair_dir == DUNG_IDX_UPSTAIRS )
    {
        g_pGame->GetMsgs()->Printf( "You enter a maze of up staircases.\n" );
        // dungeon_level--, make sure not to go less than 0
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel( -Util::Roll( 1, 5 ) );
        return JSUCCESS;
    }
    else if( keysym->sym == SDLK_COMMA && stair_dir == DUNG_IDX_LONG_UPSTAIRS )
    {
        g_pGame->GetMsgs()->Printf( "You enter a long maze of up staircases.\n" );
        // dungeon_level-- (a bunch), make sure not to go less than 0
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel( -1 );
        return JSUCCESS;
    }
    // if on >, go down stairs
    else if( keysym->sym == SDLK_PERIOD && stair_dir == DUNG_IDX_DOWNSTAIRS )
    {
        g_pGame->GetMsgs()->Printf( "You enter a maze of down staircases.\n" );
        // dungeon_level++
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel( 1 );
        return JSUCCESS;
    }
    else if( keysym->sym == SDLK_PERIOD && stair_dir == DUNG_IDX_LONG_DOWNSTAIRS )
    {
        g_pGame->GetMsgs()->Printf( "You enter a long maze of down staircases.\n" );
        // dungeon_level++ (a bunch)
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel( Util::Roll( 1, 5 ) );
        return JSUCCESS;
    }
    else
    {
        g_pGame->GetMsgs()->Printf( "You can't do that here.\n" );
        return JSUCCESS;
    }
}

int CCmdState::TestStairs() { return ( g_pGame->GetDungeon()->IsStairs( m_vNewPos ) ); }

void CCmdState::DisplayInventory() { g_pGame->GetPlayer()->DisplayInventory( PLACEMENT_INV ); }
