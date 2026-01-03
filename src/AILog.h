#ifndef __AILOG_H__
#define __AILOG_H__

#include <stdio.h>
#include <time.h>

// AILog - Text logging for AI observability
// Writes human-readable events to ai-logs/session-<NNN>-<timestamp>.log
// See ai-logs/FORMAT.example for complete format documentation

// Initialize AI logging - opens log file
// basedir: Base directory for the game (e.g., "../JMoria/")
void AILog_Init( const char *basedir );

// Terminate AI logging - closes log file
void AILog_Term();

// Check if AI logging is active
bool AILog_IsActive();

// Write a line of text to the log (adds newline)
void AILog_Write( const char *text );

// Write formatted text to the log (printf-style, adds newline)
// Example: AILog_Text("TURN %d", 42)
void AILog_Text( const char *format, ... );

// Write a blank line to separate events
void AILog_BlankLine();

// Write formatted text + blank line (for standalone events like TURN, MOVE, COMBAT)
// Example: AILog_Event("TURN %d", 42) writes "TURN 42\n\n"
void AILog_Event( const char *format, ... );

// Get current timestamp in ISO 8601 format
// buffer: Output buffer (should be at least 32 bytes)
void AILog_Timestamp( char *buffer );

// Helper: Convert name to underscore format (spaces to underscores)
// Returns pointer to static buffer - NOT thread safe
const char *AILog_Name( const char *name );

#endif // __AILOG_H__
