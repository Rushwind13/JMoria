# WORKLIST: FileParse JSON Migration

## Overview
Convert the proprietary text-based resource file format (Items.txt, Monsters.txt, Scores.txt) to JSON format while maintaining the project's philosophy of minimal third-party dependencies and full feature implementation.

## Current State Analysis

### Proprietary Format Characteristics
- **Format**: Custom text format with angle-bracket delimited values `<value>`
- **Structure**: Entry-based with opening tags (`Item`, `Monster`, `Score`) followed by `{...}` blocks
- **Fields**: Key-value pairs with type-specific parsing (string, int, float, comma-delimited lists)
- **Special Cases**: 
  - Multi-hued colors using nested angle brackets `<<rgb1>,<rgb2>,...,<rgbn>>`
  - Complex nested structures (attacks, effects) with multiple comma-delimited fields
  - Comments with `#` prefix
  - Whitespace-flexible parsing

### Current FileParse Class Capabilities
- **Read Operations**: 
  - `ReadMonster(CMonsterDef &mdIn)` - Sequential monster reading
  - `ReadItem(CItemDef &idIn)` - Sequential item reading
  - `ReadScore(CScore &sIn)` - Sequential score reading
- **Write Operations**:
  - `WriteScore(CScore *sIn)` - Appends score to file
- **Parsing Utilities**:
  - `Strip(char *szLine)` - Removes comments, whitespace, newlines
  - `chomp(const char *szLine, char *szIn)` - Extracts `<value>` from angle brackets
  - `GetValue(...)` - Overloaded for char*, int, long, float extraction
  - `ParseColors(char *szLine)` - Handles multi-hued color lists
- **Internal State**:
  - Shuffle arrays for potions, scrolls, wands, staves (randomization)
  - Counters for tracking potion/scroll/wand/staff assignments

### Current Usage
- **Dungeon.cpp**: Loads monster and item definitions at initialization
- **EndGameState.cpp**: Reads/writes high scores
- **Test suites**: ItemSteps.cpp, MonsterSteps.cpp use CDataFile for test fixture setup
- All parsing is done sequentially with while loops until EOF

## JSON Format Design

### Rationale for Custom JSON Parser
Following project philosophy:
1. **Minimal Dependencies**: No external JSON libraries (no jsoncpp, rapidjson, nlohmann/json)
2. **Full Implementation**: Hand-rolled parser with complete control
3. **Consistency**: Matches existing code style (C-style strings, manual memory management)
4. **Zero Cost**: No additional build dependencies or licensing concerns

### Proposed JSON Schema

#### Items.json
```json
{
  "items": [
    {
      "name": "Chain Mail",
      "plural": "Sets of Chain Mail",
      "type": "ITEM_IDX_ARMOR",
      "value": 40.0,
      "level": 26,
      "ac": 22.0,
      "weight": 2.0,
      "color": "225,225,50,255",
      "flags": ["ITEM_FLAG_EQUIPMENT"]
    },
    {
      "name": "Potion of Healing",
      "plural": "Potions of Healing",
      "type": "ITEM_IDX_POTION",
      "value": 50.0,
      "level": 1,
      "weight": 0.5,
      "effects": [
        {
          "effect": "EFFECT_TYPE_HEAL",
          "flags": "EFFECT_FLAG_HP",
          "modifier": "EFFECT_MOD_NONE"
        }
      ]
    }
  ]
}
```

#### Monsters.json
```json
{
  "monsters": [
    {
      "name": "Giant Frog",
      "plural": "Giant Frogs",
      "appear": "1d1",
      "speed": 2.0,
      "moveType": "MON_AI_SEEKPLAYER",
      "hd": "3d8",
      "ac": 40.0,
      "level": 5,
      "expValue": 35.0,
      "type": "MON_IDX_FROG",
      "flags": ["MON_FLAG_WARM"],
      "attacks": [
        {
          "effect": "EFFECT_TYPE_HIT",
          "type": "MON_FLAG_BITE",
          "damage": "2d8"
        }
      ],
      "color": "71,165,0,255"
    },
    {
      "name": "Red Dragon",
      "plural": "Red Dragons",
      "color": ["255,0,0,255", "200,0,0,255", "255,50,50,255"]
    }
  ]
}
```

#### Scores.json
```json
{
  "scores": [
    {
      "name": "Aragorn",
      "class": "Warrior",
      "race": "Human",
      "level": 10,
      "depth": 15,
      "reason": "Killed by a Red Dragon",
      "score": 15420,
      "date": 1735689600
    }
  ]
}
```

## Implementation Plan

### Phase 1: JSON Parser Core (New Files)

#### 1.1 Create JSONParser.h
**File**: `src/JSONParser.h`
**Responsibilities**:
- Lightweight JSON parsing without external dependencies
- Match project style: C-style strings, manual memory management
- Support for: objects, arrays, strings, numbers, booleans, null

**Class Structure**:
```cpp
#ifndef __JSONPARSER_H__
#define __JSONPARSER_H__
#include "JLinkList.h"
#include "JMDefs.h"

// JSON Value Types
enum JSONType {
    JSON_NULL = 0,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
};

// Forward declarations
class JSONValue;
class JSONObject;
class JSONArray;

// Base JSON value container
class JSONValue {
public:
    JSONValue();
    ~JSONValue();
    
    JSONType GetType() const { return m_type; }
    
    // Type checking
    bool IsNull() const { return m_type == JSON_NULL; }
    bool IsBool() const { return m_type == JSON_BOOL; }
    bool IsNumber() const { return m_type == JSON_NUMBER; }
    bool IsString() const { return m_type == JSON_STRING; }
    bool IsArray() const { return m_type == JSON_ARRAY; }
    bool IsObject() const { return m_type == JSON_OBJECT; }
    
    // Value accessors (with type checking)
    bool GetBool(bool defaultVal = false) const;
    int GetInt(int defaultVal = 0) const;
    long GetLong(long defaultVal = 0) const;
    float GetFloat(float defaultVal = 0.0f) const;
    const char* GetString(const char* defaultVal = "") const;
    JSONArray* GetArray() const;
    JSONObject* GetObject() const;
    
protected:
    JSONType m_type;
    union {
        bool m_boolVal;
        float m_numberVal;
        char* m_stringVal;
        JSONArray* m_arrayVal;
        JSONObject* m_objectVal;
    };
};

// JSON Object (key-value pairs)
class JSONObject {
public:
    JSONObject();
    ~JSONObject();
    
    bool HasKey(const char* key) const;
    JSONValue* Get(const char* key) const;
    
    // Convenience accessors (delegating to JSONValue)
    bool GetBool(const char* key, bool defaultVal = false) const;
    int GetInt(const char* key, int defaultVal = 0) const;
    long GetLong(const char* key, long defaultVal = 0) const;
    float GetFloat(const char* key, float defaultVal = 0.0f) const;
    const char* GetString(const char* key, const char* defaultVal = "") const;
    JSONArray* GetArray(const char* key) const;
    JSONObject* GetObject(const char* key) const;
    
    void Set(const char* key, JSONValue* value);
    JLinkList<char>* GetKeys() const;
    
private:
    // Using JLinkList for consistency with project
    struct KeyValuePair {
        char* key;
        JSONValue* value;
    };
    JLinkList<KeyValuePair>* m_pairs;
};

// JSON Array
class JSONArray {
public:
    JSONArray();
    ~JSONArray();
    
    int GetSize() const { return m_size; }
    JSONValue* Get(int index) const;
    void Add(JSONValue* value);
    
    // Convenience accessors for array of primitives
    bool GetBool(int index, bool defaultVal = false) const;
    int GetInt(int index, int defaultVal = 0) const;
    float GetFloat(int index, float defaultVal = 0.0f) const;
    const char* GetString(int index, const char* defaultVal = "") const;
    
private:
    JLinkList<JSONValue>* m_values;
    int m_size;
};

// Main JSON Parser
class JSONParser {
public:
    JSONParser();
    ~JSONParser();
    
    // Parse from file
    JSONValue* ParseFile(const char* filename);
    
    // Parse from string
    JSONValue* ParseString(const char* jsonStr);
    
    // Error handling
    bool HasError() const { return m_errorMsg != NULL; }
    const char* GetError() const { return m_errorMsg; }
    
private:
    char* m_errorMsg;
    const char* m_cursor;
    int m_line;
    int m_col;
    
    // Parsing internals
    void SkipWhitespace();
    bool Match(char c);
    char Peek() const;
    char Advance();
    
    JSONValue* ParseValue();
    JSONObject* ParseObject();
    JSONArray* ParseArray();
    char* ParseString();
    JSONValue* ParseNumber();
    JSONValue* ParseKeyword(const char* keyword, JSONType type, bool boolVal = false);
    
    void SetError(const char* msg);
};

#endif // __JSONPARSER_H__
```

#### 1.2 Create JSONParser.cpp
**File**: `src/JSONParser.cpp`
**Implementation Details**:
- Hand-written recursive descent parser
- Careful memory management (manual new/delete)
- Error reporting with line/column tracking
- Tolerant of trailing commas (developer-friendly)
- No external dependencies

**Key Implementation Points**:
- Use `fopen/fread/fclose` for file I/O (consistent with current code)
- Use `Util::jstrlen`, `Util::jstrcpy` for string operations
- Implement proper escape sequence handling in strings
- Handle floating point parsing with `atof`
- Line-by-line error tracking for debugging

#### 1.3 Create JSONWriter.h and JSONWriter.cpp
**Files**: `src/JSONWriter.h`, `src/JSONWriter.cpp`
**Purpose**: Write JSON files (primarily for Scores.json)

**Class Structure**:
```cpp
class JSONWriter {
public:
    JSONWriter();
    ~JSONWriter();
    
    // Write to file with pretty formatting
    bool WriteFile(const char* filename, JSONValue* root, bool prettyPrint = true);
    
    // Write to string
    char* WriteString(JSONValue* root, bool prettyPrint = true);
    
private:
    char* m_buffer;
    int m_bufferSize;
    int m_bufferPos;
    int m_indentLevel;
    bool m_prettyPrint;
    
    void WriteValue(JSONValue* value);
    void WriteObject(JSONObject* obj);
    void WriteArray(JSONArray* arr);
    void WriteString(const char* str);
    void WriteNumber(float num);
    void WriteBool(bool val);
    void WriteNull();
    
    void Append(const char* str);
    void AppendChar(char c);
    void Indent();
    void Newline();
    void EnsureCapacity(int needed);
};
```

### Phase 2: Enhanced FileParse with JSON Support

#### 2.1 Update FileParse.h
**File**: `src/FileParse.h`

**Changes**:
```cpp
#ifndef __FILEPARSE_H__
#define __FILEPARSE_H__
#include "JColor.h"
#include "JLinkList.h"
#include "TextEntry.h"
#include "JSONParser.h"  // NEW

class CMonsterDef;
class CItemDef;
class CScore;

enum FileFormat {
    FORMAT_LEGACY = 0,  // Current proprietary format
    FORMAT_JSON = 1      // New JSON format
};

class CDataFile
{
public:
    CDataFile();
    ~CDataFile();

    // Opening files - auto-detect format
    bool Open(const char *szFilename);
    bool Append(const char *szFilename);
    bool Close();
    
    // Format detection
    FileFormat DetectFormat(const char *szFilename);
    void SetFormat(FileFormat format) { m_format = format; }
    FileFormat GetFormat() const { return m_format; }

    // Legacy format - existing methods
    CMonsterDef *ReadMonster(CMonsterDef &mdIn);
    CItemDef *ReadItem(CItemDef &idIn);
    CScore *ReadScore(CScore &sIn);
    bool WriteScore(CScore *sIn);
    
    // JSON format - new batch methods
    JLinkList<CMonsterDef>* ReadMonstersJSON(const char *szFilename);
    JLinkList<CItemDef>* ReadItemsJSON(const char *szFilename);
    JLinkList<CScore>* ReadScoresJSON(const char *szFilename);
    bool WriteScoresJSON(const char *szFilename, JLinkList<CScore>* scores);
    bool AppendScoreJSON(const char *szFilename, CScore *score);

protected:
    // Legacy parsing helpers
    char *Strip(char *szLine);
    char *chomp(const char *szLine, char *szIn);
    JLinkList<JColor> *ParseColors(char *szLine);
    char *GetValue(char *szLine, char *szIn);
    int GetValue(char *szLine, int &dwIn);
    int GetValue(char *szLine, long &dwIn);
    float GetValue(char *szLine, float &fIn);
    
    // JSON parsing helpers (NEW)
    CMonsterDef* ParseMonsterJSON(JSONObject* obj);
    CItemDef* ParseItemJSON(JSONObject* obj);
    CScore* ParseScoreJSON(JSONObject* obj);
    JLinkList<JColor>* ParseColorsJSON(JSONValue* value);
    void ParseAttacksJSON(JSONArray* arr, CMonsterDef* md);
    void ParseEffectsJSON(JSONArray* arr, CItemDef* id);
    
    // JSON writing helpers (NEW)
    JSONObject* MonsterToJSON(CMonsterDef* md);
    JSONObject* ItemToJSON(CItemDef* id);
    JSONObject* ScoreToJSON(CScore* score);
    JSONValue* ColorsToJSON(JLinkList<JColor>* colors);
    JSONArray* AttacksToJSON(JLinkList<CAttack>* attacks);
    JSONArray* EffectsToJSON(JLinkList<CEffect>* effects);

private:
    FILE *m_fp;
    FileFormat m_format;
    JSONParser* m_jsonParser;
    JSONValue* m_jsonRoot;  // Cached root for sequential reading
    int m_jsonIndex;         // Current index for sequential reading
    
    // Legacy shuffle state
    int PotionIndex[NUM_POTION_TYPES];
    int ScrollIndex[NUM_SCROLL_TYPES];
    int WandIndex[NUM_LUMBER_TYPES];
    int StaffIndex[NUM_LUMBER_TYPES];
    uint32 m_dwPotionCount = 0;
    uint32 m_dwScrollCount = 0;
    uint32 m_dwWandCount = 0;
    uint32 m_dwStaffCount = 0;
};
#endif // __FILEPARSE_H__
```

**Key Design Decisions**:
1. **Backward Compatibility**: Keep all legacy methods intact
2. **Auto-Detection**: `Open()` detects format by examining first non-whitespace character (`{` = JSON, other = legacy)
3. **Dual Mode**: Can work with both formats simultaneously
4. **Batch Operations**: JSON methods read entire file at once (more efficient than sequential)
5. **Progressive Migration**: Can switch formats per-file (Items.json, Monsters.txt, etc.)

#### 2.2 Implement FileParse.cpp JSON Methods
**File**: `src/FileParse.cpp`

**Implementation Highlights**:

```cpp
// Constructor initialization
CDataFile::CDataFile() 
    : m_fp(NULL),
      m_format(FORMAT_LEGACY),
      m_jsonParser(NULL),
      m_jsonRoot(NULL),
      m_jsonIndex(0)
{
    // ... existing initialization
}

// Format detection
FileFormat CDataFile::DetectFormat(const char *szFilename) {
    FILE* fp = fopen(szFilename, "r");
    if (!fp) return FORMAT_LEGACY;
    
    // Skip whitespace and comments
    char c;
    while ((c = fgetc(fp)) != EOF) {
        if (c == '{') {
            fclose(fp);
            return FORMAT_JSON;
        }
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '#') {
            break;
        }
        // Skip comment lines
        if (c == '#') {
            while ((c = fgetc(fp)) != EOF && c != '\n') {}
        }
    }
    fclose(fp);
    return FORMAT_LEGACY;
}

// Enhanced Open with format detection
bool CDataFile::Open(const char *szFilename) {
    m_format = DetectFormat(szFilename);
    
    if (m_format == FORMAT_JSON) {
        // JSON mode - parse entire file upfront
        m_jsonParser = new JSONParser();
        m_jsonRoot = m_jsonParser->ParseFile(szFilename);
        
        if (m_jsonParser->HasError()) {
            JLog(LOG_LEVEL_ERROR, true, "JSON Parse Error: %s\n", 
                 m_jsonParser->GetError());
            return false;
        }
        
        m_jsonIndex = 0;
        return true;
    } else {
        // Legacy mode - existing behavior
        m_fp = fopen(szFilename, "r");
        if (m_fp == NULL) return false;
        
        Util::Shuffle(PotionIndex, NUM_POTION_TYPES);
        Util::Shuffle(ScrollIndex, NUM_SCROLL_TYPES);
        Util::Shuffle(WandIndex, NUM_LUMBER_TYPES);
        Util::Shuffle(StaffIndex, NUM_LUMBER_TYPES);
        
        return true;
    }
}

// JSON Monster Reading (batch)
JLinkList<CMonsterDef>* CDataFile::ReadMonstersJSON(const char *szFilename) {
    JSONParser parser;
    JSONValue* root = parser.ParseFile(szFilename);
    
    if (parser.HasError() || !root || !root->IsObject()) {
        JLog(LOG_LEVEL_ERROR, true, "Failed to parse monsters JSON: %s\n",
             parser.GetError());
        return NULL;
    }
    
    JSONObject* rootObj = root->GetObject();
    JSONArray* monstersArray = rootObj->GetArray("monsters");
    
    if (!monstersArray) {
        JLog(LOG_LEVEL_ERROR, true, "No 'monsters' array in JSON\n");
        return NULL;
    }
    
    JLinkList<CMonsterDef>* result = new JLinkList<CMonsterDef>;
    
    for (int i = 0; i < monstersArray->GetSize(); i++) {
        JSONValue* val = monstersArray->Get(i);
        if (val && val->IsObject()) {
            CMonsterDef* md = ParseMonsterJSON(val->GetObject());
            if (md) {
                result->Add(md);
            }
        }
    }
    
    delete root;
    return result;
}

// Parse individual monster from JSON
CMonsterDef* CDataFile::ParseMonsterJSON(JSONObject* obj) {
    CMonsterDef* md = new CMonsterDef();
    
    // String fields
    const char* name = obj->GetString("name", NULL);
    if (name) {
        md->m_szName = new char[Util::jstrlen(name) + 1];
        Util::jstrcpy(md->m_szName, name);
    }
    
    const char* plural = obj->GetString("plural", NULL);
    if (plural) {
        md->m_szPlural = new char[Util::jstrlen(plural) + 1];
        Util::jstrcpy(md->m_szPlural, plural);
    }
    
    const char* appear = obj->GetString("appear", "1d1");
    md->m_szAppear = new char[Util::jstrlen(appear) + 1];
    Util::jstrcpy(md->m_szAppear, appear);
    
    const char* hd = obj->GetString("hd", NULL);
    if (hd) {
        md->m_szHD = new char[Util::jstrlen(hd) + 1];
        Util::jstrcpy(md->m_szHD, hd);
    }
    
    // Numeric fields
    md->m_fSpeed = obj->GetFloat("speed", 1.0f);
    md->m_fBaseAC = obj->GetFloat("ac", 0.0f);
    md->m_dwLevel = obj->GetInt("level", 1);
    md->m_fExpValue = obj->GetFloat("expValue", 0.0f);
    
    // Type lookup
    const char* moveType = obj->GetString("moveType", NULL);
    if (moveType && g_Constants.CompareType("MON_AI", moveType)) {
        md->m_dwMoveType = g_Constants.LookupString(moveType);
    }
    
    const char* type = obj->GetString("type", NULL);
    if (type && g_Constants.CompareType("MON_IDX", type)) {
        md->m_dwIndex = g_Constants.LookupString(type);
    }
    
    // Flags array
    JSONArray* flagsArr = obj->GetArray("flags");
    if (flagsArr) {
        for (int i = 0; i < flagsArr->GetSize(); i++) {
            const char* flag = flagsArr->GetString(i, NULL);
            if (flag) {
                md->m_dwFlags |= g_Constants.LookupString(flag);
            }
        }
    }
    
    // Color (single or multi-hued)
    JSONValue* colorVal = obj->Get("color");
    if (colorVal) {
        if (colorVal->IsArray()) {
            // Multi-hued
            md->m_Colors = ParseColorsJSON(colorVal);
            md->m_dwFlags |= MON_COLOR_MULTI;
        } else if (colorVal->IsString()) {
            // Single color
            md->m_Color.SetColor(colorVal->GetString());
        }
    }
    
    // Attacks array
    JSONArray* attacksArr = obj->GetArray("attacks");
    if (attacksArr) {
        ParseAttacksJSON(attacksArr, md);
    }
    
    return md;
}

// Parse attacks from JSON array
void CDataFile::ParseAttacksJSON(JSONArray* arr, CMonsterDef* md) {
    for (int i = 0; i < arr->GetSize(); i++) {
        JSONValue* val = arr->Get(i);
        if (!val || !val->IsObject()) continue;
        
        JSONObject* attackObj = val->GetObject();
        CAttack* attack = new CAttack();
        
        const char* effect = attackObj->GetString("effect", NULL);
        if (effect) {
            attack->m_dwEffect = g_Constants.LookupString(effect);
        }
        
        const char* type = attackObj->GetString("type", NULL);
        if (type) {
            attack->m_dwType = g_Constants.LookupString(type);
        }
        
        const char* effectFlags = attackObj->GetString("effectFlags", NULL);
        if (effectFlags) {
            attack->m_dwEffectFlags = g_Constants.LookupString(effectFlags);
        }
        
        const char* damage = attackObj->GetString("damage", NULL);
        if (damage) {
            attack->m_szDamage = new char[Util::jstrlen(damage) + 1];
            Util::jstrcpy(attack->m_szDamage, damage);
        }
        
        md->m_llAttacks->Add(attack);
    }
}
```

**Similar implementations needed for**:
- `ReadItemsJSON()` and `ParseItemJSON()`
- `ReadScoresJSON()` and `ParseScoreJSON()`
- `WriteScoresJSON()`, `AppendScoreJSON()`, and `ScoreToJSON()`

### Phase 3: Resource File Conversion

#### 3.1 Create Conversion Utility
**File**: `scripts/convert_to_json.sh`
**Purpose**: Convert legacy files to JSON format

```bash
#!/bin/bash
# Convert legacy resource files to JSON format

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RESOURCES_DIR="$SCRIPT_DIR/../Resources"

# Build the converter tool
echo "Building JSON converter..."
cd "$SCRIPT_DIR/.."
make json_converter

# Convert files
echo "Converting Items.txt to Items.json..."
./json_converter --type items \
    --input "$RESOURCES_DIR/Items.txt" \
    --output "$RESOURCES_DIR/Items.json"

echo "Converting Monsters.txt to Monsters.json..."
./json_converter --type monsters \
    --input "$RESOURCES_DIR/Monsters.txt" \
    --output "$RESOURCES_DIR/Monsters.json"

echo "Converting Scores.txt to Scores.json (if exists)..."
if [ -f "$RESOURCES_DIR/Scores.txt" ]; then
    ./json_converter --type scores \
        --input "$RESOURCES_DIR/Scores.txt" \
        --output "$RESOURCES_DIR/Scores.json"
fi

echo "Conversion complete!"
```

#### 3.2 Create Converter Tool
**File**: `util/json_converter.cpp`
**Purpose**: Standalone tool to convert legacy files to JSON

```cpp
#include "../src/FileParse.h"
#include "../src/JSONWriter.h"
#include "../src/Item.h"
#include "../src/Monster.h"
#include "../src/EndGameState.h"
#include <stdio.h>
#include <string.h>

void PrintUsage() {
    printf("Usage: json_converter --type <items|monsters|scores> "
           "--input <file> --output <file>\n");
}

int main(int argc, char** argv) {
    const char* type = NULL;
    const char* inputFile = NULL;
    const char* outputFile = NULL;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            type = argv[++i];
        } else if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            inputFile = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            outputFile = argv[++i];
        }
    }
    
    if (!type || !inputFile || !outputFile) {
        PrintUsage();
        return 1;
    }
    
    g_Constants.Init();
    
    CDataFile reader;
    reader.SetFormat(FORMAT_LEGACY);
    
    if (strcmp(type, "items") == 0) {
        // Convert items
        if (!reader.Open(inputFile)) {
            printf("Error: Could not open %s\n", inputFile);
            return 1;
        }
        
        JLinkList<CItemDef>* items = new JLinkList<CItemDef>;
        CItemDef* item = new CItemDef();
        
        while (reader.ReadItem(*item)) {
            items->Add(item);
            item = new CItemDef();
        }
        delete item;
        reader.Close();
        
        // Write JSON
        JSONObject* root = new JSONObject();
        JSONArray* itemsArr = new JSONArray();
        
        CLink<CItemDef>* link = items->m_pllHead;
        while (link) {
            CItemDef* id = link->m_Contents;
            JSONObject* itemObj = reader.ItemToJSON(id);
            JSONValue* val = new JSONValue();
            val->SetObject(itemObj);
            itemsArr->Add(val);
            link = link->m_pllNext;
        }
        
        JSONValue* arrVal = new JSONValue();
        arrVal->SetArray(itemsArr);
        root->Set("items", arrVal);
        
        JSONWriter writer;
        JSONValue* rootVal = new JSONValue();
        rootVal->SetObject(root);
        writer.WriteFile(outputFile, rootVal, true);
        
        printf("Converted %d items to %s\n", items->m_dwCount, outputFile);
        
    } else if (strcmp(type, "monsters") == 0) {
        // Similar to items...
        
    } else if (strcmp(type, "scores") == 0) {
        // Similar to items...
        
    } else {
        printf("Error: Unknown type '%s'\n", type);
        PrintUsage();
        return 1;
    }
    
    return 0;
}
```

#### 3.3 Add to Makefile
**File**: `Makefile`
**Addition**:
```makefile
JSON_CONVERTER = json_converter
JSON_CONVERTER_SOURCES = util/json_converter.cpp
JSON_CONVERTER_OBJECTS = $(JSON_CONVERTER_SOURCES:.cpp=.o)

$(JSON_CONVERTER): $(JSON_CONVERTER_OBJECTS) $(filter-out src/main.o, $(OBJECTS))
	$(CC) $(JSON_CONVERTER_OBJECTS) $(filter-out src/main.o, $(OBJECTS)) $(LD_FLAGS) -o $(JSON_CONVERTER)

.PHONY: json_converter
json_converter: $(JSON_CONVERTER)
```

### Phase 4: Integration and Migration

#### 4.1 Update Dungeon.cpp
**File**: `src/Dungeon.cpp`
**Changes**:
```cpp
// Line 77-89: Replace legacy loading with JSON-aware loading
CDataFile dfMonsters;
char szMonsterFile[256];
sprintf(szMonsterFile, "%s%s", szBasedir, "Resources/Monsters.json");  // Try JSON first

if (!dfMonsters.Open(szMonsterFile)) {
    // Fallback to legacy format
    sprintf(szMonsterFile, "%s%s", szBasedir, "Resources/Monsters.txt");
    if (!dfMonsters.Open(szMonsterFile)) {
        JLog(LOG_LEVEL_ERROR, true, "Could not open monster file\n");
        return false;
    }
}

if (dfMonsters.GetFormat() == FORMAT_JSON) {
    // JSON batch loading
    m_llMonsterDefs = dfMonsters.ReadMonstersJSON(szMonsterFile);
    if (!m_llMonsterDefs) {
        JLog(LOG_LEVEL_ERROR, true, "Failed to parse monsters JSON\n");
        return false;
    }
} else {
    // Legacy sequential loading
    m_llMonsterDefs = new JLinkList<CMonsterDef>;
    CMonsterDef *pmd = new CMonsterDef;
    while (dfMonsters.ReadMonster(*pmd)) {
        m_llMonsterDefs->Add(pmd);
        pmd = new CMonsterDef;
    }
    delete pmd;
}

dfMonsters.Close();

// Similar changes for items (lines 96-107)
```

#### 4.2 Update EndGameState.cpp
**File**: `src/EndGameState.cpp`
**Changes**:
```cpp
// Line 189-230: Replace score handling
bool CEndGameState::InitScores() {
    CDataFile dfScores;
    
    // Write new score
    if (m_pScore->m_dwScore > 0) {
        // Try JSON first
        if (dfScores.DetectFormat("Resources/Scores.json") == FORMAT_JSON) {
            dfScores.AppendScoreJSON("Resources/Scores.json", m_pScore);
        } else {
            // Create new JSON file or fallback to legacy
            dfScores.Append("Resources/Scores.txt");
            dfScores.WriteScore(m_pScore);
            dfScores.Close();
        }
    }
    
    // Read scores
    m_llScores = new JLinkList<CScore>;
    
    if (dfScores.Open("Resources/Scores.json")) {
        // JSON batch read
        m_llScores = dfScores.ReadScoresJSON("Resources/Scores.json");
    } else if (dfScores.Open("Resources/Scores.txt")) {
        // Legacy sequential read
        CScore *ps = new CScore;
        while (dfScores.ReadScore(*ps)) {
            ps->InitToString();
            m_llScores->Add(ps, ps->m_dwScore, false);
            ps = new CScore;
        }
        delete ps;
    }
    
    dfScores.Close();
    return true;
}
```

#### 4.3 Update Test Fixtures
**File**: `test/features/step_definitions/ItemSteps.cpp`
**Changes**:
```cpp
GIVEN("^I have an ItemDef$") {
    ScenarioScope<TestCtx> context;
    g_Constants.Init();

    CItemDef *pid;
    CDataFile dfItems;
    
    // Try JSON first, fallback to legacy
    if (dfItems.Open("../../JMoria/Resources/Items.json")) {
        JLinkList<CItemDef>* items = dfItems.ReadItemsJSON("../../JMoria/Resources/Items.json");
        if (items && items->m_dwCount > 0) {
            context->ItemDef = items->m_pllHead->m_Contents;
        }
    } else {
        dfItems.Open("../../JMoria/Resources/Items.txt");
        pid = new CItemDef;
        context->ItemDef = dfItems.ReadItem(*pid);
    }
}
```

**Similar changes needed for**: `MonsterSteps.cpp`

### Phase 5: Testing and Validation

#### 5.1 Unit Tests for JSON Parser
**File**: `test/features/json_parser.feature`
```gherkin
Feature: JSON Parser
  As a developer
  I want to parse JSON files
  So that I can read game data in JSON format

  Scenario: Parse simple JSON object
    Given I have a JSON string '{"name": "Test", "value": 42}'
    When I parse the JSON
    Then the object should have key "name" with value "Test"
    And the object should have key "value" with value 42

  Scenario: Parse JSON array
    Given I have a JSON string '{"items": ["sword", "shield", "potion"]}'
    When I parse the JSON
    Then the array "items" should have 3 elements

  Scenario: Parse nested JSON
    Given I have a JSON string '{"monster": {"name": "Orc", "hp": 10}}'
    When I parse the JSON
    Then the nested object "monster" should have key "name"

  Scenario: Handle parse errors gracefully
    Given I have an invalid JSON string '{"name": "Test"'
    When I parse the JSON
    Then I should get a parse error
```

**File**: `test/features/step_definitions/JSONSteps.cpp`
```cpp
#include "TestContext.hpp"
#include "JSONParser.h"

GIVEN("^I have a JSON string '(.*)'$") {
    ScenarioScope<TestCtx> context;
    const char* jsonStr = matches[1].str().c_str();
    context->jsonString = new char[strlen(jsonStr) + 1];
    strcpy(context->jsonString, jsonStr);
}

WHEN("^I parse the JSON$") {
    ScenarioScope<TestCtx> context;
    context->jsonParser = new JSONParser();
    context->jsonValue = context->jsonParser->ParseString(context->jsonString);
}

THEN("^the object should have key \"(.*)\" with value \"(.*)\"$") {
    ScenarioScope<TestCtx> context;
    EXPECT_TRUE(context->jsonValue != NULL);
    EXPECT_TRUE(context->jsonValue->IsObject());
    
    JSONObject* obj = context->jsonValue->GetObject();
    const char* key = matches[1].str().c_str();
    const char* expectedVal = matches[2].str().c_str();
    
    EXPECT_TRUE(obj->HasKey(key));
    EXPECT_STREQ(obj->GetString(key), expectedVal);
}

// ... more step definitions
```

#### 5.2 Integration Tests
**File**: `test/features/fileparse_json.feature`
```gherkin
Feature: FileParse JSON Support
  As a developer
  I want to load game resources from JSON files
  So that data is easier to edit and maintain

  Scenario: Load items from JSON
    Given I have a JSON items file
    When I read the items
    Then all items should be loaded correctly

  Scenario: Load monsters from JSON
    Given I have a JSON monsters file
    When I read the monsters
    Then all monsters should be loaded correctly

  Scenario: Write and read scores in JSON
    Given I have a score
    When I write it to JSON
    And I read scores from JSON
    Then my score should be in the list

  Scenario: Auto-detect JSON format
    Given I have both Items.txt and Items.json
    When I open the file with auto-detect
    Then it should use the JSON format

  Scenario: Fallback to legacy format
    Given I only have Items.txt
    When I open the file with auto-detect
    Then it should use the legacy format
```

#### 5.3 Validation Script
**File**: `scripts/validate_json.sh`
```bash
#!/bin/bash
# Validate JSON files against schema

RESOURCES_DIR="$(dirname "$0")/../Resources"

echo "Validating JSON files..."

# Check Items.json
if [ -f "$RESOURCES_DIR/Items.json" ]; then
    echo "Checking Items.json..."
    python3 -m json.tool "$RESOURCES_DIR/Items.json" > /dev/null
    if [ $? -eq 0 ]; then
        echo "  ✓ Valid JSON"
    else
        echo "  ✗ Invalid JSON"
        exit 1
    fi
fi

# Check Monsters.json
if [ -f "$RESOURCES_DIR/Monsters.json" ]; then
    echo "Checking Monsters.json..."
    python3 -m json.tool "$RESOURCES_DIR/Monsters.json" > /dev/null
    if [ $? -eq 0 ]; then
        echo "  ✓ Valid JSON"
    else
        echo "  ✗ Invalid JSON"
        exit 1
    fi
fi

echo "All JSON files valid!"
```

### Phase 6: Documentation and Migration Path

#### 6.1 Update Developer's Guide
**File**: `_JMoria Developer's Guide.md`
**Addition**:
```markdown
## Working with JSON Resource Files

JMoria now supports JSON format for resource files (Items, Monsters, Scores).

### Format Support
- **Legacy Format**: Original proprietary text format (`.txt` files)
- **JSON Format**: Standard JSON format (`.json` files)
- **Auto-Detection**: CDataFile automatically detects format on Open()

### File Structure

#### Items.json
```json
{
  "items": [
    {
      "name": "Long Sword",
      "plural": "Long Swords",
      "type": "ITEM_IDX_SWORD",
      "value": 150.0,
      "level": 1,
      "damage": "1d8",
      "weight": 6.0,
      "speed": 5.0,
      "color": "200,200,200,255"
    }
  ]
}
```

#### Monsters.json
```json
{
  "monsters": [
    {
      "name": "Giant Frog",
      "plural": "Giant Frogs",
      "appear": "1d1",
      "speed": 2.0,
      "moveType": "MON_AI_SEEKPLAYER",
      "hd": "3d8",
      "ac": 40.0,
      "level": 5,
      "expValue": 35.0,
      "type": "MON_IDX_FROG",
      "flags": ["MON_FLAG_WARM"],
      "attacks": [
        {
          "effect": "EFFECT_TYPE_HIT",
          "type": "MON_FLAG_BITE",
          "damage": "2d8"
        }
      ],
      "color": "71,165,0,255"
    }
  ]
}
```

### Converting Legacy Files

```bash
# Convert all resource files to JSON
./scripts/convert_to_json.sh

# Convert individual file
./json_converter --type items \
    --input Resources/Items.txt \
    --output Resources/Items.json
```

### Adding New Items/Monsters

1. Edit the appropriate JSON file (Items.json or Monsters.json)
2. Add your entry to the array
3. Use proper JSON syntax (commas, quotes, etc.)
4. Validate: `scripts/validate_json.sh`
5. Test in-game

### Backward Compatibility

The game will automatically use JSON files if present, otherwise falls back
to legacy .txt files. Both formats are fully supported.
```

#### 6.2 Create Migration Guide
**File**: `MIGRATION_JSON.md`
```markdown
# JSON Migration Guide

## Overview
This guide explains the migration from legacy `.txt` resource files to JSON format.

## Why Migrate?

### Benefits of JSON
1. **Standard Format**: Widely recognized, many tools available
2. **Better Validation**: Syntax errors caught immediately
3. **Easier Editing**: Clearer structure, better editor support
4. **Batch Operations**: Load entire file at once (faster)
5. **Extensibility**: Easy to add new fields without breaking parser

### Project Philosophy Maintained
- **No External Dependencies**: Custom JSON parser, no third-party libraries
- **Full Implementation**: Complete control over parsing behavior
- **Backward Compatible**: Legacy format still supported
- **Zero Cost**: No build complexity added

## Migration Path

### Phase 1: Convert Files (Non-Breaking)
```bash
# Convert to JSON (keeps original files)
./scripts/convert_to_json.sh
```

### Phase 2: Test with JSON (Dual Format)
- Run game with JSON files
- Verify all items/monsters load correctly
- Keep legacy files as backup
- Run full test suite

### Phase 3: Update Workflows
- Update documentation
- Update level editors/tools
- Train team on JSON editing

### Phase 4: Deprecate Legacy (Optional)
- Remove `.txt` files
- Remove legacy parsing code
- Update tests to JSON-only

## Rollback Plan
If issues arise, simply:
1. Delete `.json` files
2. Game automatically falls back to `.txt` files
3. No code changes required

## Technical Details

### Format Detection
```cpp
// Auto-detect in CDataFile::Open()
FileFormat format = DetectFormat(filename);
// JSON files start with '{', legacy with Item/Monster/Score keyword
```

### Performance
- **JSON**: Parse entire file once, O(1) lookups
- **Legacy**: Sequential reads, O(n) per item
- **Result**: JSON is faster for initialization

### Memory Usage
- **JSON**: Holds parsed tree in memory during read
- **Legacy**: Minimal memory, reads line-by-line
- **Trade-off**: Acceptable for game resource files (<1MB)

## FAQ

**Q: Do I need to migrate immediately?**  
A: No, both formats are supported indefinitely.

**Q: Can I mix formats?**  
A: Yes, Items.json + Monsters.txt works fine.

**Q: What if I edit JSON incorrectly?**  
A: Game will log parse error and fall back to legacy file if available.

**Q: How do I validate JSON?**  
A: Run `./scripts/validate_json.sh` or use `python3 -m json.tool Items.json`

**Q: Can I still edit .txt files?**  
A: Yes, but you'll need to reconvert to JSON or rename JSON files.
```

### Phase 7: Optional Enhancements

#### 7.1 JSON Schema Validation (Future)
**File**: `Resources/schemas/items_schema.json`
**Purpose**: Define expected structure for validation
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["items"],
  "properties": {
    "items": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["name", "type", "level"],
        "properties": {
          "name": {"type": "string"},
          "plural": {"type": "string"},
          "type": {"type": "string", "pattern": "^ITEM_IDX_"},
          "value": {"type": "number", "minimum": 0},
          "level": {"type": "integer", "minimum": 0, "maximum": 100},
          "ac": {"type": "number"},
          "weight": {"type": "number", "minimum": 0}
        }
      }
    }
  }
}
```

#### 7.2 JSON Pretty-Printer Tool
**File**: `util/json_format.cpp`
**Purpose**: Reformat JSON files consistently
```cpp
// Reads JSON, writes with consistent indentation and field ordering
```

#### 7.3 JSON Diff Tool
**File**: `scripts/json_diff.sh`
**Purpose**: Show differences between legacy and JSON files
```bash
#!/bin/bash
# Convert legacy to temp JSON and diff against existing JSON
```

## Testing Strategy

### Regression Testing
1. **Existing Tests**: All current tests must pass with JSON files
2. **Cross-Format**: Test loading same data from both formats
3. **Round-Trip**: Legacy → JSON → Legacy should produce identical data

### Performance Testing
1. Measure load times for Items/Monsters (both formats)
2. Memory usage comparison
3. Verify no degradation in game startup time

### Error Handling Testing
1. Malformed JSON (missing brackets, quotes, commas)
2. Invalid field types (string where number expected)
3. Missing required fields
4. Unknown fields (should be ignored gracefully)

## Code Style Compliance

### Naming Conventions
```cpp
// Classes: PascalCase with 'C' prefix
class CDataFile;
class JSONParser;

// Methods: PascalCase
bool Open(const char *szFilename);
JSONValue* ParseFile(const char* filename);

// Variables: m_ prefix for members, lowercase with type prefix
char* m_szName;
float m_fSpeed;
int m_dwFlags;

// Constants: UPPER_SNAKE_CASE
#define FORMAT_LEGACY 0
#define FORMAT_JSON 1
```

### Memory Management
```cpp
// Manual new/delete (no smart pointers)
JSONParser* parser = new JSONParser();
// ... use parser
delete parser;

// C-style strings
char* str = new char[length + 1];
Util::jstrcpy(str, source);
delete[] str;
```

### Error Handling
```cpp
// Log errors with JLog
JLog(LOG_LEVEL_ERROR, true, "Parse error: %s\n", errorMsg);

// Return NULL on failure
if (error) return NULL;

// Boolean success indicators
bool success = ParseFile(filename);
if (!success) {
    // handle error
}
```

## Implementation Timeline

### Estimated Effort
- **Phase 1** (JSON Parser Core): 3-4 days
- **Phase 2** (FileParse Integration): 2-3 days
- **Phase 3** (Resource Conversion): 1 day
- **Phase 4** (Integration): 1-2 days
- **Phase 5** (Testing): 2-3 days
- **Phase 6** (Documentation): 1 day
- **Total**: 10-14 days

### Dependencies
1. Phase 1 must complete before Phase 2
2. Phase 2 must complete before Phase 3
3. Phases 3-5 can partially overlap
4. Phase 6 can be done anytime after Phase 2

### Milestones
1. **M1**: JSON parser passes all unit tests
2. **M2**: Can load Items.json successfully
3. **M3**: Can load Monsters.json successfully
4. **M4**: Can read/write Scores.json successfully
5. **M5**: All existing tests pass with JSON files
6. **M6**: Documentation complete

## Risk Mitigation

### Risks
1. **Breaking Existing Functionality**: Mitigated by keeping legacy code, dual format support
2. **Performance Degradation**: Mitigated by performance testing, batch loading
3. **JSON Parser Bugs**: Mitigated by extensive unit tests, gradual rollout
4. **Data Loss**: Mitigated by keeping original files, validation scripts

### Rollback Triggers
- Critical bug in JSON parser
- Significant performance degradation (>20% slower)
- Data corruption in converted files
- Test failures that can't be resolved quickly

### Rollback Procedure
1. Delete or rename `.json` files
2. Game automatically uses `.txt` files
3. Remove JSON-specific code (if necessary)
4. Revert relevant commits

## Success Criteria

### Functional
- [ ] All items load correctly from Items.json
- [ ] All monsters load correctly from Monsters.json
- [ ] Scores can be written to and read from Scores.json
- [ ] Game runs identically with JSON vs legacy files
- [ ] All tests pass with JSON files

### Non-Functional
- [ ] Load time within 10% of legacy format
- [ ] Memory usage reasonable (<5MB for JSON parsing)
- [ ] JSON parser handles edge cases gracefully
- [ ] Error messages are clear and actionable

### Documentation
- [ ] Developer guide updated
- [ ] Migration guide complete
- [ ] JSON format documented with examples
- [ ] README updated with format information

## Notes
- Keep legacy format support indefinitely for backward compatibility
- JSON files should be human-editable (pretty-printed)
- Maintain project philosophy: no external dependencies
- Use existing utility functions (Util::jstrlen, etc.)
- Follow existing code style and patterns
- Add logging for debugging (JLog with appropriate levels)
