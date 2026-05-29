#!/bin/sh

# List all strings in key: value format (one per line)
# Usage: ./util/list_string.sh [optional search term]

SEARCH_TERM="${1:-.}"  # Default to all entries if no search term

perl -ne '
  if (/^String <(STR_[^>]+)>/) {
    $name = $1;
    $in_string = 1;
  } elsif ($in_string && /^\s*Text\s*<(.+)>/) {
    $value = $1;
    # Convert escape sequences to readable format
    $value =~ s/\\n/[LF]/g;
    $value =~ s/\\t/[TAB]/g;
    # Check if matches search term (case-insensitive)
    if ($name =~ /'"$SEARCH_TERM"'/i || $value =~ /'"$SEARCH_TERM"'/i) {
      printf "%s: %s\n", $name, $value;
    }
    $in_string = 0;
  }
' Resources/Strings.txt
