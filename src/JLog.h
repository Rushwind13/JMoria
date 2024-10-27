#ifndef __JLOG_H__
#define __JLOG_H__
#include <cstdarg>
#include <stdio.h>

static JResult JLog( eLogLevel eLogLevel, const char *format, ... )
{
    if( eLogLevel >= g_eLogLevel )
    {
        va_list args;
        va_start( args, format );
        vprintf( format, args );
        va_end( args );
    }
    if( eLogLevel >= LOG_LEVEL_WARN )
    {
        return JBOGUSKEY;
    }
    return JSUCCESS;
}
#endif // __JLOG_H__