#ifndef __FILEPARSE_H__
#define __FILEPARSE_H__
#include "TextEntry.h"
#include <stdio.h>

class CMonsterDef;
class CDataFile
{
	// Member variables
public:
	CDataFile() {};
	~CDataFile() {};

	bool Open( const char *szFilename );
	
	CMonsterDef *ReadMonster(CMonsterDef &mdIn);
protected:
	char	*Strip( char *szLine );
	char	*GetValue(char *szLine, char *szIn);
	int		GetValue(char *szLine, int &dwIn);
	float	GetValue(char *szLine, float &fIn);
private:
	FILE	*m_fp;
	// Member functions
public:
protected:
private:
};
#endif // __FILEPARSE_H__