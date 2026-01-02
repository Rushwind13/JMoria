// AILog.cpp
// JSON Lines logging for AI observability
// Writes structured events to ai-logs/session-<timestamp>.jsonl

#include "AILog.h"
#include <algorithm>
#include <cstdarg>
#include <cstdlib>
#include <dirent.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <vector>

// Global file handle for AI log
static FILE *g_pAILogFile = NULL;
static char g_szLogDir[256] = {0};

/**
 * Maximum number of uncompressed session files to keep.
 * When this limit is exceeded, older sessions are compressed with gzip.
 */
#define AILOG_MAX_SESSIONS 10

void AILog_Timestamp( char *buffer )
{
    time_t now = time( NULL );
    struct tm *tm_info = localtime( &now );
    strftime( buffer, 32, "%Y-%m-%dT%H:%M:%S", tm_info );
}

/**
 * Archives old session log files when the count exceeds AILOG_MAX_SESSIONS.
 * Uses gzip compression to reduce disk usage while preserving logs.
 * Files are sorted by name (which includes timestamp) to determine age.
 */
static void AILog_ArchiveOldSessions()
{
    if( strlen( g_szLogDir ) == 0 )
    {
        return;
    }

    DIR *dir = opendir( g_szLogDir );
    if( dir == NULL )
    {
        return;
    }

    // Collect all .jsonl files
    std::vector<std::string> sessions;
    struct dirent *entry;
    while( ( entry = readdir( dir ) ) != NULL )
    {
        const char *name = entry->d_name;
        size_t len = strlen( name );

        // Check for .jsonl extension
        if( len > 6 && strcmp( name + len - 6, ".jsonl" ) == 0 )
        {
            sessions.push_back( std::string( name ) );
        }
    }
    closedir( dir );

    // If we have more than max, archive the oldest ones
    if( sessions.size() <= AILOG_MAX_SESSIONS )
    {
        return;
    }

    // Sort by name (timestamp is in the name, so alphabetical = chronological)
    std::sort( sessions.begin(), sessions.end() );

    // Archive the oldest files (all but the most recent AILOG_MAX_SESSIONS)
    size_t toArchive = sessions.size() - AILOG_MAX_SESSIONS;
    for( size_t i = 0; i < toArchive; i++ )
    {
        char filepath[512];
        sprintf( filepath, "%s%s", g_szLogDir, sessions[i].c_str() );

        // Compress with gzip
        char cmd[600];
        sprintf( cmd, "gzip \"%s\"", filepath );
        int result = system( cmd );

        if( result == 0 )
        {
            fprintf( stderr, "AILog: Archived old session: %s\n", sessions[i].c_str() );
        }
    }
}

void AILog_Init( const char *basedir )
{
    if( g_pAILogFile != NULL )
    {
        return; // Already initialized
    }

    // Store the log directory path for archiving
    sprintf( g_szLogDir, "%sai-logs/", basedir );

    // Generate filename with timestamp
    char timestamp[32];
    time_t now = time( NULL );
    struct tm *tm_info = localtime( &now );
    strftime( timestamp, sizeof( timestamp ), "%Y-%m-%dT%H-%M-%S", tm_info );

    char filename[256];
    sprintf( filename, "%ssession-%s.jsonl", g_szLogDir, timestamp );

    g_pAILogFile = fopen( filename, "w" );
    if( g_pAILogFile == NULL )
    {
        fprintf( stderr, "AILog: Could not open log file: %s\n", filename );
        return;
    }

    // Write session start event
    char ts[32];
    AILog_Timestamp( ts );
    AILog_Event( "session_start", "\"timestamp\":\"%s\"", ts );

    // Flush immediately so we don't lose the start event
    fflush( g_pAILogFile );

    // Archive old sessions if we have too many
    AILog_ArchiveOldSessions();
}

void AILog_Term()
{
    if( g_pAILogFile == NULL )
    {
        return;
    }

    // Write session end event
    char ts[32];
    AILog_Timestamp( ts );
    AILog_Event( "session_end", "\"timestamp\":\"%s\"", ts );

    fclose( g_pAILogFile );
    g_pAILogFile = NULL;
}

bool AILog_IsActive()
{
    return g_pAILogFile != NULL;
}

void AILog_Write( const char *json )
{
    if( g_pAILogFile == NULL )
    {
        return;
    }

    fprintf( g_pAILogFile, "%s\n", json );
    fflush( g_pAILogFile ); // Flush after each write for real-time visibility
}

void AILog_Event( const char *type, const char *format, ... )
{
    if( g_pAILogFile == NULL )
    {
        return;
    }

    // Build the JSON object
    char buffer[4096];
    int offset = sprintf( buffer, "{\"type\":\"%s\"", type );

    // Add additional fields if format is provided
    if( format != NULL && strlen( format ) > 0 )
    {
        offset += sprintf( buffer + offset, "," );

        va_list args;
        va_start( args, format );
        offset += vsprintf( buffer + offset, format, args );
        va_end( args );
    }

    offset += sprintf( buffer + offset, "}" );

    AILog_Write( buffer );
}
