#ifndef __TEXTENTRY_H__
#define __TEXTENTRY_H__

#include "string.h"

class TextEntry
{
    // Member variables
public:
    TextEntry() : m_szString( 0 ), m_dwValue( 0 ), m_dwFlagSet( 0 ) {}
    TextEntry( const char *szIn, int dwIn ) { Init( szIn, dwIn ); }

    ~TextEntry()
    {
        if( m_szString != NULL )
            delete[] m_szString;
        m_szString = NULL;
    }

    void Init( const char *szIn, int dwIn )
    {
        m_szString = new char[Util::jstrlen( szIn ) + 1];
        Util::jstrcpy( m_szString, szIn );
        m_dwValue = dwIn;
        m_dwFlagSet = 0;
    };

    void Init( const char *szIn, int dwIn, int dwFlagSet )
    {
        Init( szIn, dwIn );
        m_dwFlagSet = dwFlagSet;
    };

    char *m_szString;
    int m_dwValue;
    int m_dwFlagSet;

protected:
private:
    // Member functions
public:
protected:
private:
};
#endif // __TEXTENTRY_H__