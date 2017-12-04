#include "Monster.h"
#include "Game.h"
#include "DisplayText.h"
#include "Dungeon.h"
CMonster::CMonster()
:m_fCurHP(0.0f),
m_fCurAC(0.0f),
m_md(NULL),
m_pBrain(NULL)
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

JResult CMonster::Init( CMonsterDef *pmd )
{
	m_md = pmd;
	m_fCurHP = pmd->m_fBaseHP;
	m_fCurAC = pmd->m_fBaseAC;
	
	return SpawnMonster();
}

JResult CMonster::SpawnMonster()
{
	if( !m_md )
	{
		return JERROR();
	}
	
	bool bMonsterSpawned = false;
	printf("Trying to spawn monster type: %d...", m_md->m_dwType);
	JVector vTryPos;
	while( !bMonsterSpawned )
	{
		vTryPos.Init( (float)(Util::GetRandom(0, DUNG_WIDTH-1)), (float)(Util::GetRandom(0, DUNG_HEIGHT-1)) );
		
		//printf("Trying to spawn monster type: %d at <%.2f %.2f>...\n", m_md->m_dwType, vTryPos.x, vTryPos.y );
		//g_pGame->GetMsgs()->Printf( "Trying to spawn monster type: %d at <%.2f %.2f>...\n", m_md->m_dwType, vTryPos.x, vTryPos.y );
		
		if( g_pGame->GetDungeon()->IsWalkable(vTryPos) == DUNG_COLL_NO_COLLISION )
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


unsigned char MonIDs[MON_IDX_MAX+1] = ",Ji";
void CMonster::Draw()
{
	Uint8 monster_tile = MonIDs[m_md->m_dwIndex] - ' ' - 1;
	JVector vSize(1,1);

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