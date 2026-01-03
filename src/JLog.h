#ifndef __JLOG_H__
#define __JLOG_H__
#include <cstdarg>
#include <stdio.h>

// AI log file pointer (set by JLog_InitAI/JLog_TermAI)
extern FILE *g_pAILogFile;

// Initialize AI logging to file
void JLog_InitAI( const char *basedir );
void JLog_TermAI();
bool JLog_IsAIActive();

static const char *Level( eLogLevel log_level )
{
    switch( log_level )
    {
    case LOG_LEVEL_NOISE:
        return "NOISE";
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    case LOG_LEVEL_INFO:
        return "INFO";
    case LOG_LEVEL_WARN:
        return "WARN";
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_AI:
        return "AI";
    default:
        return "UNKNOWN";
    }
}

static JResult JLog( eLogLevel eLogLevel, bool verbose, const char *format, ... )
{
    char mod_format[1024];
    if( verbose )
    {
        sprintf( mod_format, "[%s]: %s", Level( eLogLevel ), format );
    }
    else
    {
        sprintf( mod_format, "%s", format );
    }

    // AI logging goes to file
    if( eLogLevel == LOG_LEVEL_AI && g_pAILogFile )
    {
        va_list args;
        va_start( args, format );
        vfprintf( g_pAILogFile, format, args );
        va_end( args );
        fflush( g_pAILogFile );
        return JSUCCESS;
    }

    // Regular logging to console
    if( eLogLevel >= g_eLogLevel && eLogLevel != LOG_LEVEL_AI )
    {
        va_list args;
        va_start( args, format );
        vprintf( mod_format, args );
        va_end( args );
    }
    if( eLogLevel >= LOG_LEVEL_WARN && eLogLevel < LOG_LEVEL_AI )
    {
        return JBOGUSKEY;
    }
    return JSUCCESS;
}
#endif // __JLOG_H__