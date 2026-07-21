#ifndef __FILEPARSE_H__
#define __FILEPARSE_H__
#include "Effect.h"
#include "JColor.h"
#include "JLinkList.h"
#include "MonsterRecall.h"
#include "TextEntry.h"

class CMonsterDef;
class CItemDef;
class CDungeon;
class CScore;
struct CRecallEntry;

class CDataFile
{
    // Member variables
public:
    CDataFile() {}
    ~CDataFile() {}

    bool Open( const char *szFilename );
    bool Append( const char *szFilename );
    bool Write( const char *szFilename );
    bool Close();

    CPalette *ReadPalette( CPalette &ceIn );
    CMonsterDef *ReadMonster( CMonsterDef &mdIn );
    CItemDef *ReadItem( CItemDef &idIn );
    CEffectDef *ReadEffect( CEffectDef &edIn );
    bool ReadStringEntry( int &outKey, char *&outText );
    CScore *ReadScore( CScore &sIn );
    bool WriteScore( CScore *sIn );
    CRecallEntry *ReadMonsterRecall( CRecallEntry &rIn );
    bool WriteMonsterRecall( const CRecallEntry *rIn );

protected:
    char *Strip( char *szLine );
    char *chomp( const char *szLine, char *szIn );
    JLinkList<JColor> *ParseColors( char *szLine );
    char *GetValue( char *szLine, char *szIn );
    int GetValue( char *szLine, int &dwIn );
    int GetValue( char *szLine, long &dwIn );
    float GetValue( char *szLine, float &fIn );
    CEffect *EffectFromName( const char *szName );

    // Copies all colors from the named palette entry (via g_pGame->GetDungeon()) into dest.
    // Returns true if the name was found.
    bool ApplyPalette( const char *szName, JLinkList<JColor> *dest );

    // Parse a chomped color value (named, single raw, or multi raw) into outSingle / outMulti.
    // Named palette or multi raw → colors appended to outMulti, returns true.
    // Single raw → outSingle set, returns false.
    bool ReadColor( const char *color, JColor &outSingle, JLinkList<JColor> *outMulti );

private:
    FILE *m_fp;
    int PotionIndex[NUM_POTION_TYPES];
    int ScrollIndex[NUM_SCROLL_TYPES];
    int WandIndex[NUM_LUMBER_TYPES];
    int StaffIndex[NUM_LUMBER_TYPES];
    int AmuletIndex[NUM_METAL_TYPES];
    int RingIndex[NUM_METAL_TYPES];

    uint32 m_dwPotionCount = 0;
    uint32 m_dwScrollCount = 0;
    uint32 m_dwWandCount = 0;
    uint32 m_dwStaffCount = 0;
    uint32 m_dwAmuletCount = 0;
    uint32 m_dwRingCount = 0;
    // Member functions
public:
protected:
private:
};
#endif // __FILEPARSE_H__
