/**
 * AIRender.h
 *
 * Utilities for AI log rendering - RLE encoding, viewport extraction.
 * Separates output concerns from game logic.
 */
#ifndef __AIRENDER_H__
#define __AIRENDER_H__

#define VIEW_RADIUS 10
#define VIEW_SIZE ( VIEW_RADIUS * 2 + 1 )

/**
 * RLE encode a row of characters for compact output.
 * Escapes quotes and backslashes for JSON compatibility.
 * Returns the number of characters written to output.
 */
int AIRender_RLEEncodeRow( const char *row, int len, char *output );

/**
 * Check if a row is all walls (can be skipped in output).
 */
bool AIRender_IsAllWalls( const char *row, int len );

#endif // __AIRENDER_H__
