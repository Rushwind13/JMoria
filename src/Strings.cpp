#include "Strings.h"
#include "FileParse.h"
#include "JMDefs.h"

const char *g_Strings[STR_MAX] = {};

void LoadStrings( const char *szBasedir )
{
    char szPath[512];
    snprintf( szPath, sizeof( szPath ), "%sResources/Strings.txt", szBasedir );

    CDataFile df;
    if( !df.Open( szPath ) )
    {
        JLog( LOG_LEVEL_ERROR, true, "LoadStrings: could not open %s\n", szPath );
        return;
    }

    int key = -1;
    char *text = NULL;
    int count = 0;
    while( df.ReadStringEntry( key, text ) )
    {
        if( key >= 0 && key < STR_MAX )
        {
            g_Strings[key] = text; // owned: allocated by CDataFile::chomp (new char[])
            count++;
        }
        else
        {
            JLog( LOG_LEVEL_WARN, true, "LoadStrings: key %d out of range, skipping\n", key );
            delete[] text;
        }
        key = -1;
        text = NULL;
    }
    df.Close();

    JLog( LOG_LEVEL_INFO, false, "LoadStrings: loaded %d strings\n", count );

    // Warn on any missing entries so omissions surface early
    for( int i = 0; i < STR_MAX; i++ )
    {
        if( g_Strings[i] == NULL )
        {
            JLog( LOG_LEVEL_WARN, true, "LoadStrings: STR id %d has no entry\n", i );
        }
    }
}
