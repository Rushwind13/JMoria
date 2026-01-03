#include "Monster.h"
#include "DisplayText.h"
#include "Dungeon.h"
#include "Game.h"
CMonster::CMonster()
    : m_fHP( 0.0f ),
      m_fCurHP( 0.0f ),
      m_fCurAC( 0.0f ),
      m_fLastHPTime( 0.0f ),
      m_dwFecundity( 5 ),
      m_md( NULL ),
      m_pBrain( NULL ),
      m_fColorChangeInterval( COLOR_CHANGE_TIMEOUT + 1 ),
      m_fLastBreed( BREED_INTERVAL )
{
    m_pBrain = new CAIBrain;
}
CMonster::~CMonster()
{
    if( m_pBrain )
    {
        g_pGame->GetAIMgr()->DestroyBrain( m_pBrain );
        m_pBrain = NULL;
    }
}

void CMonster::Init( CMonsterDef *pmd )
{
    m_md = pmd;
    if( pmd->m_fBaseHP != 0.0f )
    {
        m_fHP = pmd->m_fBaseHP;
    }
    else
    {
        m_fHP = Util::Roll( pmd->m_szHD );
    }
    m_fCurHP = m_fHP;
    m_fCurAC = pmd->m_fBaseAC;
    m_pCurrentAttack = NULL;
    m_bIsPlayerTarget = false;
    memset( m_szCurrentFlavorText, 0, sizeof( char ) * 64 );
}

void CMonster::InitBrain( CMonsterDef *pmd )
{
    m_pBrain->m_fSpeed = pmd->m_fSpeed;
    m_pBrain->m_dwMoveType = pmd->m_dwMoveType;
    m_pBrain->SetState( BRAINSTATE_SEEK );
    m_pBrain->SetParent( this );
}

JResult CMonster::CreateMonster( CMonsterDef *pmd, JIVector vSpawnPoint, bool bNear )
{
    JResult retval = JSUCCESS;
    int desired = Util::Roll( pmd->m_szAppear );
    JRect rcNearby = Util::Nearby( vSpawnPoint );
    for( int count = 0; count < desired; count++ )
    {
        CMonster *pMon;
        pMon = new CMonster;
        retval = pMon->InitAndSpawn( pmd, bNear ? Util::GetRandomPoint( rcNearby ) : vSpawnPoint );
        if( retval == JSUCCESS )
        {
            // force additional monsters of the same type to appear near each other
            bNear = true;
            JIVector vMon( VEC_EXPAND( pMon->GetPos() ) );
            rcNearby = Util::Nearby( vMon );
        }
        else
        {
            JLog( LOG_LEVEL_WARN, true, "monster spawn failed.\n" );
            // That spawn failed; clean up
            // delete pMon;
            // pMon = NULL;
            // return retval;
        }
    }

    return retval;
}

JResult CMonster::InitAndSpawn( CMonsterDef *pmd, JIVector vRequestedSpawnPoint )
{
    JResult retval = JSUCCESS;

    JLog( LOG_LEVEL_INFO, false, "Trying to spawn monster type: %s...", pmd->m_szName );

    // Find a good spawn point
    JIVector vSpawnPoint = GetSpawnPoint( vRequestedSpawnPoint );

    if( vSpawnPoint.IsWithinWorld() )
    {
        // Initialize the Monster from the MonsterDef
        Init( pmd );

        // Initialize the Brain
        // TODO: move to AIBrain::Init()
        InitBrain( pmd );

        // Put the monster in the world
        SpawnAt( vSpawnPoint );

        // Now that the monster is set up, add it to the global lists (monsters, brains)
        m_pllLink = g_pGame->GetDungeon()->m_llMonsters->Add( this );
        m_pBrain->m_pllLink = g_pGame->GetAIMgr()->m_llAIBrains->Add( m_pBrain );
    }
    else
    {
        retval = JBOGUSKEY;
    }

    return retval;
}

JIVector CMonster::GetSpawnPoint( JIVector vRequestedSpawnPoint )
{
    JIVector vTryPos;
    JIVector *vOpen;
    bool bNear = vRequestedSpawnPoint.IsWithinWorld();
    if( bNear )
    {
        JLog( LOG_LEVEL_INFO, false, "given <%d %d>...", VEC_EXPAND( vRequestedSpawnPoint ) );
        // return SpawnAt( vSpawnPoint );
        vTryPos.Init( VEC_EXPAND( vRequestedSpawnPoint ) );
    }
    else
    {
        vOpen = g_pGame->GetDungeon()->AnyOpenTile();
        vTryPos.Init( VEC_EXPAND( *vOpen ) );
    }

    int dwTries = 0;
    while( dwTries < DUNG_CFG_MAX_SPAWN_TRIES )
    {
        JLog( LOG_LEVEL_INFO, false, "." );

        JVector vTryIt( VEC_EXPAND( vTryPos ) );
        // target spawn point must be walkable and also within sight of the parent (no spawning past
        // walls or doors!)
        if( g_pGame->GetDungeon()->IsWalkableFor( vTryIt ) == DUNG_COLL_NO_COLLISION &&
            ( !bNear || g_pGame->GetDungeon()->CanSeeEachOther( vRequestedSpawnPoint, vTryPos ) ) )
        {
            return vTryPos;
        }
        if( bNear )
        {
            JRect rcNearby = Util::Nearby( vRequestedSpawnPoint );
            vTryPos = Util::GetRandomPoint( rcNearby );
        }
        else
        {
            vOpen = g_pGame->GetDungeon()->AnyOpenTile();
            vTryPos.Init( VEC_EXPAND( *vOpen ) );
        }
        dwTries++;
    }

    return JIVector( -1, -1 );
}

JResult CMonster::SpawnAt( JIVector vPos )
{
    JVector vSpawn( VEC_EXPAND( vPos ) );
    if( vSpawn.IsInWorld() )
    {
        SetPos( vSpawn );
        g_pGame->GetDungeon()->GetTile( vSpawn )->m_pCurMonster = this;
        JLog( LOG_LEVEL_INFO, false, "Success! Spawned at <%d %d>\n", VEC_EXPAND( vPos ) );
        // g_pGame->GetMsgs()->Printf( "Success!\n" );

        return JSUCCESS;
    }
    return JBOGUSKEY;
}

void CMonster::ChooseAttack()
{
    float which_attack = Util::Roll( 0, m_md->m_llAttacks->length() - 1 );

    m_pCurrentAttack = m_md->m_llAttacks->GetNthLink( which_attack )->m_lpData;

    JLog( LOG_LEVEL_INFO, true, "%s choosing attack: %s\n", GetName(),
          g_Constants.IndexToString( MON_FLAG, m_pCurrentAttack->m_dwType ) );
}

float CMonster::Attack()
{
    // TODO: this should be modified by resistances, immunities, and other intrinsics.
    float fRoll = Util::Roll( "1d100" );

    JLog( LOG_LEVEL_INFO, true, "%s rolled: %.2f\n", GetName(), fRoll );

    return fRoll;
}

const char *CMonster::AttackEffect()
{
    if( m_pCurrentAttack == NULL )
        return "thoughts and prayers";
    switch( m_pCurrentAttack->m_dwEffectFlags )
    {
    case EFFECT_FLAG_ACID:
        return "acid";
    case EFFECT_FLAG_COLD:
        return "cold";
    case EFFECT_FLAG_ELECTRICITY:
        return "lightning";
    case EFFECT_FLAG_FIRE:
        return "fire";
    case EFFECT_FLAG_POISON:
        return "poison gas";
    }
    return "hot air";
}

const char *CMonster::AttackFlavorText()
{
    if( m_pCurrentAttack == NULL )
    {
        Util::jstrcpy( m_szCurrentFlavorText, "misses" );
        return m_szCurrentFlavorText;
    }
    switch( m_pCurrentAttack->m_dwType )
    {
    case MON_FLAG_BITE:
        Util::jstrcpy( m_szCurrentFlavorText, "bites" );
        break;
    case MON_FLAG_CRAWL:
        Util::jstrcpy( m_szCurrentFlavorText, "crawls on" );
        break;
    case MON_FLAG_CLAW:
        Util::jstrcpy( m_szCurrentFlavorText, "claws" );
        break;
    case MON_FLAG_TRAMPLE:
        Util::jstrcpy( m_szCurrentFlavorText, "tramples" );
        break;
    case MON_FLAG_SPORE:
        Util::jstrcpy( m_szCurrentFlavorText, "releases a cloud of spores at" );
        break;
    case MON_FLAG_TOUCH:
        Util::jstrcpy( m_szCurrentFlavorText, "touches" );
        break;
    case MON_FLAG_DROOL:
        Util::jstrcpy( m_szCurrentFlavorText, "drools on" );
        break;
    case MON_FLAG_BREATHE:
        char retval[64];
        sprintf( retval, "breathes %s on", AttackEffect() );
        Util::jstrcpy( m_szCurrentFlavorText, retval );
        break;
    default:
        Util::jstrcpy( m_szCurrentFlavorText, "hits" );
        break;
    }
    return m_szCurrentFlavorText;
}

float CMonster::Damage( float fDamageMult )
{
    char *szDamage = m_pCurrentAttack->m_szDamage;
    float fDamageModifier = 0.0f;

    float fDamage = ( Util::Roll( szDamage ) + fDamageModifier ) * fDamageMult;
    JLog( LOG_LEVEL_INFO, true, "%s did %.2f damage (rolled %s)(damagemult: %.2f). ", GetName(),
          fDamage, szDamage, fDamageMult );

    return fDamage;
}

void CMonster::AttackDone() { m_pCurrentAttack = NULL; }

bool CMonster::Hit( float &fRoll ) { return ( fRoll >= m_fCurAC ); }

int CMonster::TakeDamage( float fDamage )
{
    int retval = STATUS_INVALID;

    if( (int)fDamage < (int)m_fCurHP )
    {
        m_fCurHP -= fDamage;
        retval = STATUS_ALIVE;
    }
    else
    {
        m_fCurHP = 0;
        retval = STATUS_DEAD;
    }

    JLog( LOG_LEVEL_INFO, true, "Remaining HP: %.2f \n", m_fCurHP );

    return retval;
}

// draw routines
void CMonster::Breed()
{
    if( ( m_md->m_dwFlags & MON_FLAG_BREED ) != MON_FLAG_BREED )
        return;

    if( g_pGame->GetTime() < m_fLastBreed + BREED_INTERVAL )
        return;

    if( m_dwFecundity > 0 )
    {
        JLog( LOG_LEVEL_NOISE, true, "still going: %d ", m_dwFecundity );
        if( Util::GetRandom( 0.0f, 1.0f ) <= BREED_CHANCE )
        {
            JLog( LOG_LEVEL_INFO, false, "spawnd!" );
            // Spawn a new copy
            JIVector vSpawn( VEC_EXPAND( GetPos() ) );
            CreateMonster( m_md, vSpawn, true );
        }
        JLog( LOG_LEVEL_NOISE, false, "\n" );
        m_dwFecundity--;
    }
    else
    {
        JLog( LOG_LEVEL_INFO, true, "sterilizing\n" );
        // sterilize
        m_md->m_dwFlags = m_md->m_dwFlags & ~MON_FLAG_BREED;
    }
    m_fLastBreed = g_pGame->GetTime();
}

// draw routines
void CMonster::SetColor()
{
    if( m_fColorChangeInterval < COLOR_CHANGE_TIMEOUT )
        return;

    if( ( m_md->m_dwFlags & MON_COLOR_MULTI ) == MON_COLOR_MULTI )
    {
        int which_color = Util::GetRandom( 0, m_md->m_Colors->length() - 1 );
        ( m_md->m_Color ).SetColor( *( m_md->m_Colors->GetNthLink( which_color )->m_lpData ) );
    }
    m_fColorChangeInterval = 0.0f;
}

unsigned char MonIDs[MON_IDX_MAX + 1] = "abcddefghhikllmnoprsuwxyzABCDFFFGGHIJKLOPRSTUVWWXY&.,$t";

char CMonster::GetChar()
{
    if( !m_md )
    {
        return '?';
    }
    return MonIDs[m_md->m_dwIndex];
}

void CMonster::Draw()
{
    Uint8 monster_tile = MonIDs[m_md->m_dwIndex] - ' ' - 1;
    JVector DUNG_ASPECT;

    SetColor();

    JColor color = m_md->m_Color;

    if( m_bIsPlayerTarget )
    {
        JLog( LOG_LEVEL_NOISE, false, "drawing the target monster: %s\n", GetName() );
        color = JColor( 100, 0, 0, 255 );
    }

    // PreDraw();
    g_pGame->GetDungeon()->m_TileSet->SetTileColor( color );
    g_pGame->GetDungeon()->m_TileSet->DrawTile( monster_tile, GetPos(), vSize, false );
    // PostDraw();
}

void CMonster::PreDraw() { g_pGame->GetDungeon()->PreDraw(); }

void CMonster::PostDraw() { g_pGame->GetDungeon()->PostDraw(); }
