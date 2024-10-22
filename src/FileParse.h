#ifndef __FILEPARSE_H__
#define __FILEPARSE_H__
#include "JColor.h"
#include "JLinkList.h"
#include "TextEntry.h"
#include <stdio.h>

class CMonsterDef;
class CItemDef;
class CScore;
class CDataFile
{
    // Member variables
public:
    CDataFile() {}
    ~CDataFile() {}

    bool Open( const char *szFilename );
    bool Append( const char *szFilename );
    bool Close();

    CMonsterDef *ReadMonster( CMonsterDef &mdIn );
    CItemDef *ReadItem( CItemDef &idIn );
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

private:
    FILE *m_fp;
    // Member functions
public:
protected:
private:
};
#endif // __FILEPARSE_H__
