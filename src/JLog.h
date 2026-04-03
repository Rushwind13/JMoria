#ifndef __JLOG_H__
#define __JLOG_H__
#include <cstdarg>
#include <stdio.h>

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

    if( eLogLevel >= g_eLogLevel )
    {
        va_list args;
        va_start( args, format );
        vprintf( mod_format, args );
        va_end( args );
    }
    if( eLogLevel >= LOG_LEVEL_WARN )
    {
        return JBOGUSKEY;
    }
    return JSUCCESS;
}
#endif // __JLOG_H__