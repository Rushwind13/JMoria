#ifndef __AILOG_H__
#define __AILOG_H__

#include <stdio.h>
#include <time.h>

// AILog - JSON Lines logging for AI observability
// Writes structured JSON events to ai-logs/session-<timestamp>.jsonl

// Initialize AI logging - opens log file
// basedir: Base directory for the game (e.g., "../JMoria/")
void AILog_Init( const char *basedir );

// Terminate AI logging - closes log file
void AILog_Term();

// Check if AI logging is active
bool AILog_IsActive();

// Write a raw JSON line to the log
// json: Complete JSON object as a string (without trailing newline)
void AILog_Write( const char *json );

// Convenience function to write a simple JSON object
// type: Event type string
// format: printf-style format for additional JSON fields
// Example: AILog_Event("turn", "\"turn\":%d", 42)
// Produces: {"type":"turn","turn":42}
void AILog_Event( const char *type, const char *format, ... );

// Get current timestamp in ISO 8601 format
// buffer: Output buffer (should be at least 32 bytes)
void AILog_Timestamp( char *buffer );

#endif // __AILOG_H__
