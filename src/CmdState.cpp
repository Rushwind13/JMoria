#include "CmdState.h"

#include "DisplayText.h"
#include "Dungeon.h"
#include "DungeonTile.h"
#include "Game.h"
#include "JMDefs.h"
#include "MonsterRecall.h"
#include "Player.h"

extern CGame *g_pGame;

int CCmdState::OnHandleKey( JKeysym *keysym )
{
    // If you haven't handled the key by the end of this function,
    // it's an invalid key, so return an error.
    int retval = -1;

    // Panel toggles are display-only and don't consume a game turn.
    if( IsToggleCommand( keysym ) )
        return JHANDLED_NOTURN;

    if( IsDirectional( keysym ) )
    {
        JVector vTestDir( 0, 0 );
        int dwCollideType;

        GetDir( keysym, vTestDir );

        // handle "run" movement
        if( keysym->mod & JMOD_SHIFT )
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
        case JKEY_r:
            JLog( LOG_LEVEL_INFO, true, "R)est not implemented yet.\n" );
            g_pGame->SetState( STATE_REST );
            g_pGame->GetGameState()->HandleKey( keysym );
            break;
        case JKEY_PERIOD:
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
        retval = 0;
    }

    else if( IsStaffCommand( keysym ) )
    {
        g_pGame->SetState( STATE_RANGED );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

    else if( IsFireCommand( keysym ) )
    {
        g_pGame->SetState( STATE_RANGED );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

    else if( IsPickupCommand( keysym ) )
    {
        g_pGame->GetPlayer()->PickUp( g_pGame->GetPlayer()->m_vPos );
        retval = 0;
    }

    else if( IsSearchCommand( keysym ) )
    {
        g_pGame->GetPlayer()->Search();
        retval = 0;
    }

    else if( IsXchangeCommand( keysym ) )
    {
        g_pGame->GetPlayer()->XchangeWeapons();
        retval = 0;
    }

    // Wizard-mode commands

    else if( IsExitWizardCommand( keysym ) )
    {
        g_pGame->SetState( STATE_STRINGINPUT );
        g_pGame->GetGameState()->HandleKey( keysym );
        retval = 0;
    }

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
        // Don't consume a turn for state transitions to selection modes.
        // These states (RANGED, USE, MODIFY, etc.) manage their own ready-for-update
        // flag and only consume a turn when the selection is complete.
        int eNewState = g_pGame->GetGameStateIndex();
        if( eNewState != STATE_RANGED && eNewState != STATE_USE && eNewState != STATE_MODIFY &&
            eNewState != STATE_LOOK && eNewState != STATE_TARGET && eNewState != STATE_STRINGINPUT )
        {
            g_pGame->SetReadyForUpdate( true );
        }
    }
#endif // TURN_BASED
    return retval;
}

bool CCmdState::IsModifierNeeded( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_c:
    case JKEY_o:
        return true;
        break;
        // T ( but not t  or ^t)
    case JKEY_t:
        if( keysym->mod & JMOD_SHIFT && !( keysym->mod & JMOD_CTRL ) )
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

bool CCmdState::IsUseCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
        // t (but not T or ^t)
    case JKEY_d:
    case JKEY_t:
    case JKEY_q:
    case JKEY_r:
    case JKEY_w:
    {
        if( !( keysym->mod & JMOD_SHIFT ) && !( keysym->mod & JMOD_CTRL ) )
        {
            return true;
        }
        break;
    }
    case JKEY_f:
    {
        // F (shift+f, not ^f) for fuel/fill
        if( ( keysym->mod & JMOD_SHIFT ) && !( keysym->mod & JMOD_CTRL ) )
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

bool CCmdState::IsStringInputCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_n: // name your character
        if( keysym->mod & JMOD_SHIFT )
        {
            return true;
        }
        break;
    case JKEY_p: // purchase something in a store
        return true;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsLookCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_SEMICOLON:
        if( keysym->mod & JMOD_SHIFT )
        {
            return true;
        }
    }
    return false;
}

bool CCmdState::IsTargetCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_8:
        if( keysym->mod & JMOD_SHIFT )
        {
            return true;
        }
    }
    return false;
}

bool CCmdState::IsStairsCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_COMMA:
    case JKEY_PERIOD:
        if( keysym->mod & JMOD_SHIFT )
        {
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

bool CCmdState::IsRestCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_PERIOD:
        // want . not >
        return ( keysym->mod & JMOD_SHIFT ) ? false : true;
        break;
    case JKEY_r:
        // want R not r
        return ( keysym->mod & JMOD_SHIFT ) ? true : false;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsTeleportCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_t:
        // want ^t not t
        return ( keysym->mod & JMOD_CTRL ) ? true : false;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsSetIntrinsicCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_f:
        // want ^f not f
        return ( keysym->mod & JMOD_CTRL ) ? g_pGame->GetPlayer()->IsWizard() : false;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsZapCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_z:
        if( keysym->mod == 0 )
        {
            return true;
        }
    }
    return false;
}

bool CCmdState::IsStaffCommand( JKeysym *keysym )
{
    // Z (uppercase) — use a staff (no targeting required)
    switch( keysym->sym )
    {
    case JKEY_z:
        if( keysym->mod & JMOD_SHIFT )
        {
            return true;
        }
    }
    return false;
}

bool CCmdState::IsFireCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_f:
        if( keysym->mod == 0 )
        {
            return true;
        }
    }
    return false;
}

bool CCmdState::IsXchangeCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_x:
        if( keysym->mod == 0 )
        {
            return true;
        }
    }
    return false;
}

bool CCmdState::IsCreateItemCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_i:
        // want ^t not t
        return ( keysym->mod & JMOD_CTRL ) ? g_pGame->GetPlayer()->IsWizard() : false;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsSummonMonsterCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_s:
        // want ^t not t
        return ( keysym->mod & JMOD_CTRL ) ? g_pGame->GetPlayer()->IsWizard() : false;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsPickupCommand( JKeysym *keysym )
{
    return ( keysym->sym == JKEY_g && !( keysym->mod & ( JMOD_SHIFT | JMOD_CTRL ) ) );
}

bool CCmdState::IsSearchCommand( JKeysym *keysym )
{
    return ( keysym->sym == JKEY_s && !( keysym->mod & ( JMOD_SHIFT | JMOD_CTRL ) ) );
}

bool CCmdState::IsExitWizardCommand( JKeysym *keysym )
{
    switch( keysym->sym )
    {
    case JKEY_w:
        return ( keysym->mod & JMOD_CTRL ) ? g_pGame->GetPlayer()->IsWizard() : false;
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool CCmdState::IsToggleCommand( JKeysym *keysym )
{
    if( keysym->sym == JKEY_i && keysym->mod == JMOD_NONE )
    {
        g_pGame->ToggleInv();
        return true;
    }
    if( keysym->sym == JKEY_e && keysym->mod == JMOD_NONE )
    {
        g_pGame->ToggleEquip();
        return true;
    }
    if( keysym->sym == JKEY_c && ( keysym->mod & JMOD_SHIFT ) )
    {
        g_pGame->ToggleStats();
        return true;
    }
    if( keysym->sym == JKEY_v && keysym->mod == JMOD_NONE )
    {
        g_pGame->ToggleMonsters();
        return true;
    }

    // Check for Shift+lowercase v - monster recall (any non-zero mod besides caps lock)
    // When Shift+V is pressed, many systems send lowercase 'v' with a SHIFT modifier
    if( keysym->sym == JKEY_v && keysym->mod == JMOD_SHIFT )
    {
        g_pGame->ToggleMonsterRecall();
        return true;
    }
    // Check for '(' (Item Recall) - Shift+9 produces this character
    if( keysym->sym == '(' )
    {
        g_pGame->ToggleItemRecall();
        return true;
    }
    // Check for ')' (Map Overview) - Shift+0 produces this character
    if( keysym->sym == ')' )
    {
        g_pGame->ToggleMap();
        return true;
    }
    return false;
}

int CCmdState::OnHandleStairs( JKeysym *keysym )
{
    int stair_dir = TestStairs();
    if( stair_dir == DUNG_IDX_INVALID )
    {
        g_pGame->GetMsgs()->Printf( "I do not see any stairs here.\n" );
        return JSUCCESS;
    }

    // if on <, go up stairs
    if( keysym->sym == JKEY_COMMA && stair_dir == DUNG_IDX_UPSTAIRS )
    {
        g_pGame->GetMsgs()->Printf( "You enter a maze of up staircases.\n" );
        // dungeon_level--, make sure not to go less than 0
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel( -Util::Roll( 1, 5 ) );
        return JSUCCESS;
    }
    else if( keysym->sym == JKEY_COMMA && stair_dir == DUNG_IDX_LONG_UPSTAIRS )
    {
        g_pGame->GetMsgs()->Printf( "You enter a long maze of up staircases.\n" );
        // dungeon_level-- (a bunch), make sure not to go less than 0
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel( -1 );
        return JSUCCESS;
    }
    // if on >, go down stairs
    else if( keysym->sym == JKEY_PERIOD && stair_dir == DUNG_IDX_DOWNSTAIRS )
    {
        g_pGame->GetMsgs()->Printf( "You enter a maze of down staircases.\n" );
        // dungeon_level++
        // respawn new dungeon level
        g_pGame->GetDungeon()->OnChangeLevel( 1 );
        return JSUCCESS;
    }
    else if( keysym->sym == JKEY_PERIOD && stair_dir == DUNG_IDX_LONG_DOWNSTAIRS )
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
