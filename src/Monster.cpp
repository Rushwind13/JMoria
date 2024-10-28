#include "Monster.h"
#include "DisplayText.h"
#include "Dungeon.h"
#include "Game.h"
CMonster::CMonster()
    : m_fHP( 0.0f ),
      m_fCurHP( 0.0f ),
      m_fCurAC( 0.0f ),
      m_fLastHPTime( 0.0f ),
      m_dwFecundity(5),
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
}

void CMonster::InitBrain( CMonsterDef *pmd )
{
    m_pBrain->m_fSpeed = pmd->m_fSpeed;
    m_pBrain->m_dwMoveType = pmd->m_dwMoveType;
    m_pBrain->SetState( BRAINSTATE_SEEK );
    m_pBrain->SetParent( this );
}

JResult CMonster::CreateMonster( CMonsterDef *pmd, JVector vSpawnPoint, bool bNear )
{
    int desired = Util::Roll( pmd->m_szAppear );
    for( int count = 0; count < desired; count++ )
    {
        CMonster *pMon;
        pMon = new CMonster;
        pMon->InitAndSpawn( pmd, bNear ? Util::Near(vSpawnPoint, 1): vSpawnPoint );

        // force additional monsters of the same type to appear near each other
        bNear = true;
        vSpawnPoint = pMon->GetPos();
    }

    return JSUCCESS;
}

JResult CMonster::InitAndSpawn( CMonsterDef *pmd, JVector vSpawnPoint )
{
    JResult retval = JSUCCESS;

    // Initialize the Monster from the MonsterDef
    Init( pmd );

    // Initialize the Brain
    // TODO: move to AIBrain::Init()
    InitBrain( pmd );

    // Put the monster in the world
    retval = SpawnMonster( vSpawnPoint );

    // Now that the monster is set up, add it to the global lists (monsters, brains)
    m_pllLink = g_pGame->GetDungeon()->m_llMonsters->Add( this );
    m_pBrain->m_pllLink = g_pGame->GetAIMgr()->m_llAIBrains->Add( m_pBrain );

    return retval;
}

JResult CMonster::SpawnMonster( JVector vSpawnPoint )
{
    bool bMonsterSpawned = false;
    JLog( LOG_LEVEL_INFO, true, "Trying to spawn monster type: %s...", m_md->m_szName );
    if( vSpawnPoint.IsInWorld() )
    {
        return SpawnAt( vSpawnPoint );
    }

    JVector vTryPos;
    while( !bMonsterSpawned )
    {
        JLog( LOG_LEVEL_INFO, false, "." );
        vTryPos.Init( (float)( Util::GetRandom( 0, DUNG_WIDTH - 1 ) ),
                      (float)( Util::GetRandom( 0, DUNG_HEIGHT - 1 ) ) );

        if( SpawnAt( vTryPos ) == JSUCCESS )
        {
            bMonsterSpawned = true;
        }
    }

    return JSUCCESS;
}

JResult CMonster::SpawnAt( JVector vPos )
{
    if( g_pGame->GetDungeon()->IsWalkableFor( vPos ) == DUNG_COLL_NO_COLLISION )
    {
        SetPos( vPos );
        g_pGame->GetDungeon()->GetTile( vPos )->m_pCurMonster = this;
        JLog( LOG_LEVEL_INFO, false, "Success!\n" );
        // g_pGame->GetMsgs()->Printf( "Success!\n" );

        return JSUCCESS;
    }
    return JBOGUSKEY;
}

float CMonster::Attack()
{
    float fRoll = Util::Roll( "1d100" );

    JLog( LOG_LEVEL_INFO, true, "%s rolled: %.2f\n", GetName(), fRoll );

    return fRoll;
}

float CMonster::Damage( float fDamageMult )
{
    char *szDamage = m_md->m_llAttacks->GetHead()->m_lpData->m_szDamage;
    float fDamageModifier = 0.0f;

    float fDamage = ( Util::Roll( szDamage ) + fDamageModifier ) * fDamageMult;
    JLog( LOG_LEVEL_INFO, true, "%s did %.2f damage (damagemult: %.2f). ", GetName(), fDamage,
          fDamageMult );

    return fDamage;
}

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
    if( g_pGame->GetTime() < m_fLastBreed + BREED_INTERVAL )
        return;

    if( ( m_md->m_dwFlags & MON_FLAG_BREED ) == MON_FLAG_BREED )
    {
        if( m_dwFecundity > 0 )
        {
            JLog(LOG_LEVEL_DEBUG, true, "still going: %d ", m_dwFecundity);
            if( Util::GetRandom(0.0f, 1.0f) <= BREED_CHANCE )
            {
                JLog(LOG_LEVEL_DEBUG, false, "spawnd!");
                // Spawn a new copy
                CreateMonster( m_md, GetPos(), true );
            }
            JLog(LOG_LEVEL_DEBUG, false, "\n");
            m_dwFecundity--;
        }
        else
        {
            JLog(LOG_LEVEL_WARN, true, "sterilizing\n");
            // sterilize
            m_md->m_dwFlags = m_md->m_dwFlags & ~MON_FLAG_BREED;
        }
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
        ( m_md->m_Color ).SetColor( *( m_md->m_Colors->GetLink( which_color )->m_lpData ) );
    }
    m_fColorChangeInterval = 0.0f;
}

unsigned char MonIDs[MON_IDX_MAX + 1] = "abcddefghhikllmnoprsuwxyzABCDFFFGGHIJKLOPRSTUVWWXY&.,$t";
void CMonster::Draw()
{
    Uint8 monster_tile = MonIDs[m_md->m_dwIndex] - ' ' - 1;
    JVector DUNG_ASPECT;

    SetColor();

    // PreDraw();
    g_pGame->GetDungeon()->m_TileSet->SetTileColor( m_md->m_Color );
    g_pGame->GetDungeon()->m_TileSet->DrawTile( monster_tile, GetPos(), vSize, false );
    // PostDraw();
}

void CMonster::PreDraw() { g_pGame->GetDungeon()->PreDraw(); }

void CMonster::PostDraw() { g_pGame->GetDungeon()->PostDraw(); }
