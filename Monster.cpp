 #include "Monster.h"
#include "Game.h"
#include "DisplayText.h"
#include "Dungeon.h"
CMonster::CMonster()
:m_fCurHP(0.0f),
m_fCurAC(0.0f),
m_md(NULL),
m_pBrain(NULL),
m_fColorChangeInterval(COLOR_CHANGE_TIMEOUT+1)
{
	m_pBrain = new CAIBrain;
}
CMonster::~CMonster()
{
	if( m_md )
	{
		delete m_md;
		m_md = NULL;
	}
	if(m_pBrain)
	{
		delete m_pBrain;
		m_pBrain = NULL;
	}
}

void CMonster::Init( CMonsterDef *pmd )
{
    m_md = pmd;
    if( pmd->m_fBaseHP != 0.0f )
    {
        m_fCurHP = pmd->m_fBaseHP;
    }
    else
    {
        m_fCurHP = Util::Roll(pmd->m_szHD);
    }
    m_fCurAC = pmd->m_fBaseAC;
}

void CMonster::InitBrain( CMonsterDef *pmd )
{
    m_pBrain->m_fSpeed = pmd->m_fSpeed;
    m_pBrain->m_dwMoveType = pmd->m_dwMoveType;
    m_pBrain->SetState(BRAINSTATE_SEEK);
    m_pBrain->SetParent(this);
}

JResult CMonster::CreateMonster( CMonsterDef *pmd )
{
    int desired = Util::Roll(pmd->m_szAppear);
    for( int count=0; count < desired; count++ )
    {
        CMonster *pMon;
        pMon = new CMonster;
        
        // Initialize the Monster from the MonsterDef
        pMon->Init(pmd);
        
        // Initialize the Brain
        // TODO: move to AIBrain::Init()
        pMon->InitBrain(pmd);
        
        // Put the monster in the world
        pMon->SpawnMonster();

        // Now that the monster is set up, add it to the global lists (monsters, brains)
        pMon->m_pllLink = g_pGame->GetDungeon()->m_llMonsters->Add(pMon);
        g_pGame->GetAIMgr()->m_llAIBrains->Add(pMon->m_pBrain);
    }
    
    return JSUCCESS;
}

JResult CMonster::SpawnMonster()
{
	bool bMonsterSpawned = false;
	printf("Trying to spawn monster type: %s...", m_md->m_szName);
	JVector vTryPos;
	while( !bMonsterSpawned )
	{
		vTryPos.Init( (float)(Util::GetRandom(0, DUNG_WIDTH-1)), (float)(Util::GetRandom(0, DUNG_HEIGHT-1)) );
		
		//printf("Trying to spawn monster type: %d at <%.2f %.2f>...\n", m_md->m_dwType, vTryPos.x, vTryPos.y );
		//g_pGame->GetMsgs()->Printf( "Trying to spawn monster type: %d at <%.2f %.2f>...\n", m_md->m_dwType, vTryPos.x, vTryPos.y );
		
		if( g_pGame->GetDungeon()->IsWalkableFor(vTryPos) == DUNG_COLL_NO_COLLISION )
		{
			SetPos(vTryPos);
			g_pGame->GetDungeon()->GetTile(GetPos())->m_pCurMonster = this;
			bMonsterSpawned = true;
			printf( "Success!\n" );
			//g_pGame->GetMsgs()->Printf( "Success!\n" );
		}
	}
	
	return JSUCCESS;
}


bool CMonster::Hit( float &fRoll )
{
	fRoll = Util::Roll( "1d100" );

	printf( "You rolled: %.2f\n", fRoll );

	return ( fRoll >= m_fCurAC );
}

int CMonster::Damage( float fDamageMult )
{
	int retval = STATUS_INVALID;

	float fDamage = Util::Roll( "1d2" ) * fDamageMult;

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

	printf( "You did %.2f damage (damagemult: %.2f). Remaining HP: %.2f \n", fDamage, fDamageMult, m_fCurHP );

	return retval;
}

// draw routines
void CMonster::SetColor()
{
    if( m_fColorChangeInterval < COLOR_CHANGE_TIMEOUT ) return;
    
    if( (m_md->m_dwFlags & MON_COLOR_MULTI) == MON_COLOR_MULTI )
    {
        int which_color = Util::GetRandom(0,m_md->m_Colors->length()-1);
        (m_md->m_Color).SetColor(*(m_md->m_Colors->GetLink(which_color)->m_lpData));
    }
    m_fColorChangeInterval = 0.0f;
}


unsigned char MonIDs[MON_IDX_MAX+1] = ",JidDSkxwdaH";
void CMonster::Draw()
{
	Uint8 monster_tile = MonIDs[m_md->m_dwIndex] - ' ' - 1;
	JVector vSize(1,1);

	SetColor();

	//PreDraw();
	g_pGame->GetDungeon()->m_TileSet->SetTileColor( m_md->m_Color );
	g_pGame->GetDungeon()->m_TileSet->DrawTile( monster_tile, GetPos(), vSize, false );
	//PostDraw();
}

void CMonster::PreDraw()
{
	g_pGame->GetDungeon()->PreDraw();
}

void CMonster::PostDraw()
{
	g_pGame->GetDungeon()->PostDraw();
}
