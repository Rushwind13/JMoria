#!/bin/sh

# Find strings by name or content
# Usage: ./util/find_string.sh [search term]

if [ -z "$1" ]; then
    read -p "Find which string? " SEARCH_TERM
else
    SEARCH_TERM="$1"
fi

perl -ne '
  if (/^String <(STR_[^>]+)>/) {
    $name = $1;
    $in_string = 1;
    $content = "";
  } elsif ($in_string && /^\s*Text\s*<(.+)>/) {
    $value = $1;
    $in_string = 0;
    # Convert escape sequences to readable format
    $display_value = $value;
    $display_value =~ s/\\n/[LF]/g;
    $display_value =~ s/\\t/[TAB]/g;
    # Check if matches search term (case-insensitive)
    if ($name =~ /'"$SEARCH_TERM"'/i || $display_value =~ /'"$SEARCH_TERM"'/i) {
      print "$name: $display_value\n";
    }
  }
' Resources/Strings.txt
