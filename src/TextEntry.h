#ifndef __TEXTENTRY_H__
#define __TEXTENTRY_H__

#include "string.h"

class TextEntry
{
	// Member variables
public:
	TextEntry():m_szString(0), m_dwValue(0){}
	TextEntry( char *szIn, int dwIn ) {Init(szIn, dwIn);}

	~TextEntry() { if( m_szString != NULL ) delete [] m_szString; m_szString = NULL; }

	void Init( const char *szIn, int dwIn )
	{
		m_szString = new char[strlen(szIn)+1];
		strcpy( m_szString, szIn );
		m_dwValue = dwIn;
	};

	char	*m_szString;
	int		m_dwValue;
protected:
private:

	// Member functions
public:
protected:
private:
};
#endif // __TEXTENTRY_H__