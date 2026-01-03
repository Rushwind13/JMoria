/**
 * AIRender.cpp
 *
 * Implementation of AI log rendering utilities.
 */
#include "AIRender.h"
#include <cstdio>

/**
 * RLE encode a row of characters for compact output.
 * Format: character followed by count (if > 1).
 * Example: "####...." becomes "#4.4"
 * Escapes quotes and backslashes for JSON compatibility.
 */
int AIRender_RLEEncodeRow( const char *row, int len, char *output )
{
    int outIdx = 0;
    int i = 0;

    while( i < len )
    {
        char c = row[i];
        int count = 1;

        // Count consecutive identical characters
        while( i + count < len && row[i + count] == c )
        {
            count++;
        }

        // Write character (escape for JSON if needed)
        if( c == '"' )
        {
            output[outIdx++] = '\\';
            output[outIdx++] = '"';
        }
        else if( c == '\\' )
        {
            output[outIdx++] = '\\';
            output[outIdx++] = '\\';
        }
        else
        {
            output[outIdx++] = c;
        }

        // Write count if > 1
        if( count > 1 )
        {
            outIdx += sprintf( output + outIdx, "%d", count );
        }

        i += count;
    }

    return outIdx;
}

/**
 * Check if a row is all walls (can be skipped in output to reduce size).
 */
bool AIRender_IsAllWalls( const char *row, int len )
{
    for( int i = 0; i < len; i++ )
    {
        if( row[i] != '#' )
        {
            return false;
        }
    }
    return true;
}
