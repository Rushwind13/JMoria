#ifndef __FILEPARSE_H__
#define __FILEPARSE_H__
#include "Effect.h"
#include "JColor.h"
#include "JLinkList.h"
#include "TextEntry.h"

class CMonsterDef;
class CItemDef;
class CDungeon;
class CScore;
class CDataFile
{
    // Member variables
public:
    CDataFile() : m_pDungeon( NULL ) {}
    ~CDataFile() {}

    bool Open( const char *szFilename );
    bool Append( const char *szFilename );
    bool Close();

    void SetDungeon( CDungeon *pDungeon ) { m_pDungeon = pDungeon; }
    CMonsterDef *ReadMonster( CMonsterDef &mdIn );
    CItemDef *ReadItem( CItemDef &idIn );
    CEffectDef *ReadEffect( CEffectDef &edIn );
    CScore *ReadScore( CScore &sIn );
    bool WriteScore( CScore *sIn );

protected:
    char *Strip( char *szLine );
    char *chomp( const char *szLine, char *szIn );
    JLinkList<JColor> *ParseColors( char *szLine );
    char *GetValue( char *szLine, char *szIn );
    int GetValue( char *szLine, int &dwIn );
    int GetValue( char *szLine, long &dwIn );
    float GetValue( char *szLine, float &fIn );
    CEffect *EffectFromName( const char *szName );

private:
    FILE *m_fp;
    CDungeon *m_pDungeon;
    int PotionIndex[NUM_POTION_TYPES];
    int ScrollIndex[NUM_SCROLL_TYPES];
    int WandIndex[NUM_LUMBER_TYPES];
    int StaffIndex[NUM_LUMBER_TYPES];

    uint32 m_dwPotionCount = 0;
    uint32 m_dwScrollCount = 0;
    uint32 m_dwWandCount = 0;
    uint32 m_dwStaffCount = 0;
    // Member functions
public:
protected:
private:
};
#endif // __FILEPARSE_H__
