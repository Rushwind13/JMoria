#ifndef __STRINGS_H__
#define __STRINGS_H__

// ---------------------------------------------------------------------------
// eStringId — plain sequential enum used as direct index into g_Strings[].
// STR_INVALID = -1 means "not set" / "no string".
// All values >= 0 are valid array indices.
// ---------------------------------------------------------------------------
enum eStringId
{
    STR_INVALID = -1,

    // Shared composition templates
    STR_STATUS_CHANGE = 0, // "You %s."
    STR_STATUS_CHANGE_MON, // "The %s %s."

    // Status verb fragments — player (used with STR_STATUS_CHANGE)
    STR_STATUS_SLEEP,    // "fall asleep"
    STR_STATUS_PARALYZE, // "are paralyzed"
    STR_STATUS_AFRAID,   // "flee in terror"
    STR_STATUS_CONFUSE,  // "feel confused"

    // Status verb fragments — monster (used with STR_STATUS_CHANGE_MON)
    STR_STATUS_SLEEP_MON,    // "falls asleep"
    STR_STATUS_PARALYZE_MON, // "is paralyzed"
    STR_STATUS_AFRAID_MON,   // "flees in terror"
    STR_STATUS_CONFUSE_MON,  // "looks confused"

    // Attack / combat
    STR_ATTACK_MISS,  // "misses"
    STR_ATTACK_HIT,   // "The %s %s you."
    STR_CRITICAL_HIT, // "It was an excellent hit! (x2 damage)"
    STR_YOU_MISS,     // "You miss the %s."
    STR_YOU_HIT,      // "You %s the %s."
    STR_BUMPED_INTO,  // "Ouch! You bumped into %s!"

    // Monster state messages
    STR_MON_WAKES,      // "The %s wakes up!"
    STR_MON_DIES,       // "The %s dies."
    STR_MON_IS_HIT,     // "The %s is hit."
    STR_MON_SHRIVELS,   // "The %s shrivels away in the bright light!"
    STR_MON_SCREAMS,    // "The %s screams in agony."
    STR_MON_UNAFFECTED, // "The %s is unaffected."
    STR_MON_STRIKES,    // "The %s strikes the %s with %s."
    STR_MON_SHRUGS,     // "The %s shrugs off the attack!"
    STR_MON_VULNERABLE, // "The %s is especially vulnerable!"
    STR_MON_DESTROYED,  // "The %s is destroyed!"
    STR_MON_AFFECTS,    // "The %s affects %d creature%s."
    STR_MON_CRUMBLES,   // "The %s crumbles to dust!"
    STR_MON_CRACKS,     // "The %s cracks!"
    STR_MON_VANISHES,   // "The %s vanishes!"
    STR_MON_INFO,       // "The %s: HP %d/%d  AC %d  Lvl %d  Spd %.1f"
    STR_MON_HEALTHIER,  // "The %s looks healthier."
    STR_IT_DISAPPEARS,  // "The %s disappears."
    STR_MON_EMITS,      // "The %s emits a %s."
    STR_MON_GLOWS,      // "The %s glows."

    // Dungeon / environment
    STR_IN_TOWN,               // "You are in town."
    STR_SPAWN,                 // "You pass through a one-way door, to arrive on level %d."
    STR_FOUND_SECRET_DOOR,     // "You have found a secret door!"
    STR_LIGHT_AREA,            // "The room is flooded with light!"
    STR_NOTHING_HAPPENS,       // "Nothing happens."
    STR_BALL_HARMLESS,         // "The ball explodes harmlessly."
    STR_WALL_COLLAPSES,        // "The wall turns to mud and collapses!"
    STR_DOOR_DISSOLVES,        // "The door dissolves!"
    STR_SECRET_DOOR_DISSOLVES, // "The secret door dissolves!"

    // Doors / obstacles
    STR_DOOR_SMASH,      // "You hear a door smash open."
    STR_DOOR_CREAK,      // "You hear a door creak open."
    STR_LOCK_PICKED,     // "You have picked the lock."
    STR_LOCK_FAILED,     // "You failed to pick the lock."
    STR_DOOR_HELD,       // "The door is held fast."
    STR_NO_OPEN_THERE,   // "I do not see anything to open there."
    STR_DOOR_CLOSED_MSG, // "You have closed the door."
    STR_CLOSE_FAILED,    // "You failed to close the door."
    STR_NO_CLOSE_THERE,  // "I do not see anything to close there."
    STR_NO_CLOSED_DOOR,  // "I do not see a closed door there."
    STR_SPIKE_BREAKS,    // "The spike breaks."
    STR_DOOR_SPIKED,     // "You spike the door shut."
    STR_NO_SPIKES,       // "You have no iron spikes."
    STR_NO_DOOR_THERE,   // "I do not see a door there."
    STR_DOOR_BASHED,     // "You bash the door open!"
    STR_DOOR_HOLDS,      // "You slam against the door but it holds."

    // Digging / rubble
    STR_RUBBLE_REMOVED, // "You have removed the rubble."
    STR_DIG_RUBBLE,     // "You dig in the rubble..."
    STR_TUNNEL_AIR,     // "Tunnel through what? Empty air?."

    // Navigation / stairs
    STR_NO_STAIRS,       // "I do not see any stairs here."
    STR_STAIR_UP,        // "You enter a maze of up staircases."
    STR_STAIR_UP_LONG,   // "You enter a long maze of up staircases."
    STR_STAIR_DOWN,      // "You enter a maze of down staircases."
    STR_STAIR_DOWN_LONG, // "You enter a long maze of down staircases."
    STR_CANT_DO_THAT,    // "You can't do that here."

    // Looking
    STR_DIRECTION_PROMPT, // "Direction(1 2 3 4 6 7 8 9):"
    STR_CANT_SEE_THAT,    // "You can't see that from here."
    STR_LOOK_SEE,         // "You see here a %s"
    STR_YOU_SEE,          // "You see %s."

    // Dungeon tile descriptions (for look mode)
    STR_DUNGEON_OPEN_DOOR,        // "an open door"
    STR_DUNGEON_SECRET_DOOR,      // "You can't see that from here"
    STR_DUNGEON_STAIRS_UP,        // "a staircase, going up"
    STR_DUNGEON_STAIRS_UP_LONG,   // "a long staircase, going up"
    STR_DUNGEON_STAIRS_DOWN,      // "a set of stairs, going down"
    STR_DUNGEON_STAIRS_DOWN_LONG, // "a long set of stairs, going down"
    STR_DUNGEON_FLOOR,            // "open floor"
    STR_DUNGEON_UNKNOWN,          // "... what is *that*?! .."

    // Light
    STR_LIGHT_OUT,        // "Your light has gone out!"
    STR_LIGHT_VERY_FAINT, // "Your light is growing very faint."
    STR_LIGHT_FAINT,      // "Your light is growing faint."
    STR_CANT_REFUEL,      // "Your %s cannot be refueled."

    // Inventory / equipment actions
    STR_FOUND_NOTHING,        // "You found nothing."
    STR_HAVE_ITEMS,           // "You have %d %s."
    STR_HAVE_ITEM,            // "You have a %s."
    STR_WAS_WIELDING,         // "You were wielding the %s..."
    STR_CANT_REMOVE_CURSED,   // "You can't remove the %s... it seems to be cursed."
    STR_SWITCH_WEAPON,        // "You switch to your %s."
    STR_SWITCH_BARE_HANDS,    // "You switch to your bare hands."
    STR_ORGANIZE_PACK,        // "You organize your pack."
    STR_NOW_WIELDING,         // "You are now wielding the %s."
    STR_NOW_WEARING,          // "You are now wearing the %s."
    STR_CANT_WIELD,           // "You can't wield a %s!"
    STR_REMOVED,              // "You take off the %s."
    STR_CANT_REMOVE,          // "You can't remove that!"
    STR_HOW_MANY,             // "How many? (1-%d, * for all): "
    STR_DROPPED_ITEM,         // "You dropped the %s."
    STR_DROPPED_N_ITEMS,      // "You dropped %d %s."
    STR_CANT_DROP_HERE,       // "You can't drop a %s here!"
    STR_COULD_NOT_DROP,       // "Could not drop items."
    STR_YOU_READ,             // "You read the %s."
    STR_CANT_READ,            // "You can't read a %s!"
    STR_USE_FAIL,             // "The %s slips from your fingers and returns to your pack!"
    STR_YOU_DRANK,            // "You drank the %s."
    STR_CANT_DRINK,           // "You can't drink a %s!"
    STR_SELECT_VALID,         // "Please select a valid item."
    STR_NO_SUCH_ITEM,         // "No such item. %s"
    STR_INVALID_QTY,          // "Invalid quantity."
    STR_CANT_USE_STAFF,       // "You can't use a %s as a staff!"
    STR_FILL_LANTERN,         // "You fill your lantern with the %s."
    STR_NO_LANTERN,           // "You have no lantern to fill."
    STR_CANT_USE_FUEL,        // "You can't use a %s as fuel!"
    STR_CANT_FIRE,            // "You can't fire a %s!"
    STR_CANT_ZAP,             // "You can't zap a %s!"
    STR_NOTHING_VALID,        // "Nothing valid to %s."
    STR_NOTHING_TO_FIRE_WITH, // "You have nothing to fire with."
    STR_NOTHING_TO_FIRE,      // "You have nothing to fire."
    STR_ARROW_BREAKS,         // "The arrow breaks."
    STR_CANCELLED,            // "Cancelled."

    // Item damage messages
    STR_ITEM_BURNS,        // "Your %s %s!"
    STR_ITEM_PITTED_TOHIT, // "Your %s is pitted by %s! (to-hit reduced)"
    STR_ITEM_PITTED_TODAM, // "Your %s is pitted by %s! (to-damage reduced)"
    STR_ITEM_DAMAGED,      // "Your %s is damaged by %s!"

    // Item effect messages
    STR_IDENTIFY,   // "It is %s."
    STR_OVERCHARGE, // "The %s explodes in a shower of sparks!"
    STR_RECHARGE,   // "The %s glows with magical energy. (%d charges)"
    STR_ENCHANT,    // "It glows with power."

    // Sensing / detection
    STR_MAGIC_MAP,      // "The area around you is revealed."
    STR_FULL_MAP,       // "The dungeon is revealed to you."
    STR_SENSE_DOORS,    // "You sense the presence of doors!"
    STR_SENSE_STAIRS,   // "You sense the presence of stairs!"
    STR_SENSE_TRAPS,    // "You sense traps."
    STR_SENSE_MONSTERS, // "You sense the presence of monsters!"
    STR_SENSE_TREASURE, // "You sense the presence of treasure!"
    STR_NO_TREASURE,    // "You sense no treasure nearby."

    // Recall / teleport
    STR_TELEPORT,          // "You feel a brief shimmer."
    STR_RECALL,            // "You feel a wrenching sensation."
    STR_RECALL_ACTIVE,     // "You already feel the pull of recall."
    STR_BEGIN_RECALL,      // "You feel yourself starting to drift..."
    STR_RECALL_RESET,      // "Recall depth reset (was: %dft)"
    STR_RECALL_TO_TOWN,    // "The world spins and you find yourself in town."
    STR_RECALL_TO_DUNGEON, // "The world spins and you are back at %d ft."

    // Curses
    STR_CURSED,   // "It is now cursed."
    STR_UNCURSED, // "It is no longer cursed."

    // General combat results
    STR_IMMUNE,   // "You are immune!"
    STR_RESIST,   // "You resist!"
    STR_LEVEL_UP, // "Welcome to level %d."

    // Health / healing messages
    STR_GAIN_HP_XLARGE,  // "You feel amazing!"
    STR_GAIN_HP_LARGE,   // "You feel a lot better."
    STR_GAIN_HP_MEDIUM,  // "You feel better."
    STR_GAIN_HP_SMALL,   // "You feel a bit better."
    STR_PROTECTION,      // "You feel more protected. (+%d AC)"
    STR_LOSE_PROTECTION, // "You feel less protected."
    STR_FULL_HP,         // "You feel completely healthy."

    // Intrinsics / status gained
    STR_RES_FEAR,         // "You feel resistant to fear."
    STR_AFRAID,           // "You are afraid!"
    STR_YOU_BLIND,        // "You are blind."
    STR_CONFUSED,         // "You are confused."
    STR_RES_POISON,       // "You feel resistant to poison."
    STR_POISON,           // "You are poisoned."
    STR_PARALYZE,         // "You can't move!"
    STR_SLEEP,            // "You fall asleep."
    STR_GAIN_INFRAVISION, // "Your eyes feel tingly."
    STR_GAIN_ESP,         // "You sense stray thoughts around you."
    STR_RES_FIRE,         // "You feel resistant to fire."
    STR_RES_COLD,         // "You feel resistant to cold."
    STR_RES_ELEC,         // "You feel resistant to electricity."
    STR_RES_ACID,         // "You feel resistant to acid."
    STR_GAIN_INVISIBLE,   // "You fade from view."
    STR_GAIN_LEVITATE,    // "You feel light on your feet."
    STR_GAIN_FREEACTION,  // "You feel free to move."
    STR_GAIN_SPEED,       // "You feel yourself moving faster."

    // Intrinsics / status lost
    STR_HEAL_FEAR,         // "You are no longer afraid."
    STR_HEAL_BLIND,        // "You can see again."
    STR_HEAL_CONFUSE,      // "You can think clearly again."
    STR_HEAL_POISON,       // "You are no longer poisoned."
    STR_HEAL_PARALYZE,     // "You can move again."
    STR_HEAL_ASLEEP,       // "You wake up."
    STR_LOSE_RES_FEAR,     // "You are no longer resistant to fear."
    STR_LOSE_RES_POISON,   // "You are no longer resistant to poison."
    STR_LOSE_INFRAVISION,  // "Your eyes stop tinging."
    STR_LOSE_ESP,          // "You no longer sense stray thoughts."
    STR_LOSE_RES_FIRE,     // "You no longer feel resistant to fire."
    STR_LOSE_RES_COLD,     // "You no longer feel resistant to cold."
    STR_LOSE_RES_ELEC,     // "You no longer feel resistant to electricity."
    STR_LOSE_RES_ACID,     // "You no longer feel resistant to acid."
    STR_LOSE_INVISIBILITY, // "You reappear."
    STR_LOSE_LEVITATE,     // "You float gently to the ground."
    STR_LOSE_FREEACTION,   // "You feel sluggish."
    STR_LOSE_SPEED,        // "You feel yourself slowing down."

    // Experience / stats
    STR_GAIN_XP, // "You feel more experienced."
    STR_LOSE_XP, // "You feel less experienced."
    STR_LOSE_HP, // "You feel weakened."

    // Inventory / prompt headers
    STR_CHOOSE_POTION,   // "Quaff which potion?"
    STR_CHOOSE_SCROLL,   // "Read which scroll?"
    STR_CHOOSE_WIELD,    // "Wield which item?"
    STR_CHOOSE_STAFF,    // "Use which staff?"
    STR_INV_CARRYING,    // "You are Carrying:"
    STR_INV_PAST_PAGE,   // "Inventory past first page not shown."
    STR_EQUIP_LIMIT,     // "Equipment is limited to 10 items..."
    STR_CHOOSE_WEAPON,   // "Fire which weapon?"
    STR_INV_WEARING,     // "You are wearing:"
    STR_CHOOSE_WAND,     // "Zap which wand? [a-z]"
    STR_CHOOSE_AMMO,     // "Choose ammo from inventory (a to z):"
    STR_CHOOSE_INV,      // "Choose an item from inventory (a to z):"
    STR_CHOOSE_EQUIP,    // "Choose an item from equipment(a to z):"
    STR_CHOOSE_TARGET,   // "Choose target: * or Direction (1 2 3 4 6 7 8 9)"
    STR_TARGET_PROMPT,   // "(* for target, . to choose, ESC to exit):"
    STR_TARGET_SELECTED, // "Target selected."
    STR_TARGET_LOST,     // "You can no longer see that target."

    // Effect targeting prompts
    STR_PROMPT_WHICH_DIRECTION, // "Which direction? [arrow keys or numpad]"
    STR_PROMPT_RECHARGE_WAND,   // "Recharge which wand/staff? [a-z inv, A-J equip]"
    STR_PROMPT_ENCHANT_WEAPON,  // "Enchant which weapon? [a-z inv, A-J equip]"
    STR_PROMPT_IDENTIFY_ITEM,   // "Identify which item? "
    STR_PROMPT_REMOVE_CURSE,    // "Remove curse from which item? "
    STR_PROMPT_USE_ON_ITEM,     // "Use on which item? "
    STR_PROMPT_CHOOSE_ITEM,     // "Choose an item "
    STR_ITEM_SELECTION_SUFFIX,  // "[a-z inv, A-J equip]:"

    // Stats panel labels
    STR_STAT_NAME,        // "Name: %s"
    STR_STAT_RACE,        // "Race: %s"
    STR_STAT_CLASS,       // "Class: %s"
    STR_STAT_AC,          // "AC: %d"
    STR_STAT_HP,          // "HP: %d / %d"
    STR_STAT_DAMAGE,      // "Damage: %s"
    STR_STAT_TOHIT,       // "+to Hit: %d"
    STR_STAT_TODAM,       // "+to Dam: %d"
    STR_STAT_SPEED_FAST,  // "Speed: Fast(+%d)"
    STR_STAT_SPEED_SLOW,  // "Speed: Slow(%d)"
    STR_STAT_LEVEL,       // "Level: %d"
    STR_STAT_DEPTH,       // "Depth: %d'"
    STR_STAT_EXP,         // "Exp: %d"
    STR_STAT_EXP_NEXT,    // "Exp to Next: %d"
    STR_STAT_LIGHT,       // "Light: %d turns"
    STR_STAT_INFRAVISION, // "Infravision"
    STR_STAT_TELEPATHY,   // "Telepathy"
    STR_STAT_RES_POISON,  // "Res: Poison"
    STR_STAT_POISONED,    // "Poisoned"
    STR_STAT_RES_FEAR,    // "Res: Fear"
    STR_STAT_AFRAID,      // "Afraid"
    STR_STAT_PARALYZED,   // "Paralyzed"
    STR_STAT_BLIND,       // "Blind"
    STR_STAT_ASLEEP,      // "Asleep"
    STR_STAT_CONFUSED,    // "Confused"
    STR_STAT_TARGET,      // "Target: %s"
    STR_STAT_POS,         // "Pos: <%.0f %.0f>"
    STR_STAT_TARGET_POS,  // "Target Pos: <%.0f %.0f>"
    STR_STAT_PLAYER_POS,  // "Player Pos: <%.0f %.0f>"

    // Wizard / debug
    STR_WIZ_ON,          // "*** Wizard Mode: On *** your score will not be saved."
    STR_WIZ_ADDED_ITEMS, // "Wizard: added %d wands/staves to inventory."
    STR_WIZ_IDENTIFIED,  // "Wizard: identified %d items."
    STR_WIZ,             // "** WIZARD MODE **"

    // Printf strings (auto-generated for migration)
    STR_INDENT,
    STR_NONE,
    STR_KILLED,
    STR_NO_KNOWLEDGE,
    STR_SEEN,
    STR_DMGAVG,
    STR_LIST_ITEM_WITH_COUNT,
    STR_LIST_ITEM,
    STR_HIGH_SCORE,
    STR_FPS,
    STR_FMT_STRING,
    STR_VISIBLE_MONSTER,
    STR_FMT_STRING_LF,
    STR_RECALL_MONSTER_HEADER,
    STR_N_TIMES,
    STR_CONFLICTS,
    STR_CLOCKSTEP_MODE,
    STR_CHARACTER_CREATION_SCREEN_GOES_HERE,
    STR_NAME,
    STR_DEPTH,
    STR_DUNGEON_LEVEL_FT,
    STR_DUNGEON_CREATION_COMPLETE,
    STR_ERROR_DUNGEON_NOT_INITIALIZED,
    STR_ENTER_A_NUMBER,
    STR_FLAG_NAME,
    STR_HP_RANGE,
    STR_HP_RANGE_APPROX,
    STR_HALLS,
    STR_INCORRECT_PASSWORD,
    STR_INVENTORY,
    STR_ITEM_NAME,
    STR_MONSTER_NAME,
    STR_SUMMON_MONSTERS,
    STR_NO_CURRENT_TARGET,
    STR_NOT_YET_IMPLEMENTED,
    STR_NOTHING_STIRS,
    STR_PASSWORD,
    STR_POPULATE,
    STR_START_PLAYING,
    STR_SPAWN_PLAYER,
    STR_DUNGEON_GENERATION_COMPLETE,
    STR_STEPS_PER_SEC,
    STR_ROOMS_HALLS,
    STR_ROOMS,
    STR_SEED_U,
    STR_STACK,
    STR_SUCCESS,
    STR_THE_EMITS_A_HORRIBLE_WAIL,
    STR_THE_IS_AFFECTED,
    STR_THE_LOOKS_ENRAGED,
    STR_THE_DOOR_CLICKS_SHUT,
    STR_TICK,
    STR_TIME_MS,
    STR_TIME_MS_SEC,
    STR_UNKNOWN_ITEM,
    STR_UNKNOWN_MONSTER,
    STR_UNRECOGNIZED_COMMAND_0X,
    STR_VISIBLE_MONSTERS,
    STR_WIZARD_MODE_OFF_YOU_ARE,
    STR_N_TIMESP,
    STR_YOU_ARE_AFFECTED,
    STR_YOU_CANT_SEE_THAT_TARGET,
    STR_YOU_HEAR_A_STIRRING_IN,
    STR_YOU_RECOGNIZE_IT_AS_A,
    STR_YOU_WERE_WIELDING_THE,
    STR_YOUR_OFFER,
    STR_FMT_BRACKETED,
    STR_ITEM_RECALL,
    STR_MAP,
    STR_LF,
    STR_EQUIPMENT,
    STR_GENERATION_COMPLETE,
    STR_PLAYER_POS,
    STR_STATS_GO_HERE,
    STR_N_TAU,
    STR_SOMETHING_HAPPENED,

    // Combat action results
    STR_COMBAT_HIT,        // "hit"
    STR_COMBAT_MISS,       // "miss"
    STR_COMBAT_HAVE_SLAIN, // "have slain"

    // Combat target descriptions
    STR_DUNGEON_WALL,   // "a wall"
    STR_DUNGEON_DOOR,   // "a door"
    STR_DUNGEON_RUBBLE, // "some rubble"
    STR_COMBAT_UNKNOWN, // "um, something?"

    // Monster attack
    STR_MONSTER_BREATHES_ON, // "breathes %s on"

    // Item display format strings
    STR_ITEM_POTION,  // "%s Potion"
    STR_ITEM_POTIONS, // "%s Potions"
    STR_ITEM_SCROLL,  // "Scroll labeled %s"
    STR_ITEM_SCROLLS, // "Scrolls labeled %s"
    STR_ITEM_STAFF,   // "%s Staff"
    STR_ITEM_STAVES,  // "%s Staves"
    STR_ITEM_WAND,    // "%s Wand"
    STR_ITEM_WANDS,   // "%s Wands"
    STR_ITEM_RING,    // "%s Ring"
    STR_ITEM_RINGS,   // "%s Rings"
    STR_ITEM_AMULET,  // "%s Amulet"
    STR_ITEM_AMULETS, // "%s Amulets"

    // EndGame display
    STR_ENDGAME_HIGH_SCORES, // "High Score List\n"
    STR_ENDGAME_PAST_PAGE,   // "Scores past first page not shown.\n"

    // UI window title
    STR_WINDOW_TITLE, // "JMoria %s"

    // Dungeon generation (wizard/debug mode)
    STR_DUNGEON_OPEN,   // "open "
    STR_DUNGEON_SECRET, // "secret "

    // Item display properties
    STR_ITEM_TRIED,        // " {tried}"
    STR_ITEM_CURSED,       // " {cursed}"
    STR_ITEM_CHARGES,      // " (%d charges)"
    STR_ITEM_WEAPON_BONUS, // " (%+.0f, %+.0f)"
    STR_ITEM_AC_BONUS,     // " [%+.0f]"
    STR_ITEM_WITH_FLAVOR,  // "%s %s"

    STR_MAX
};

extern const char *g_Strings[STR_MAX];

void LoadStrings( const char *szBasedir );

#endif // __STRINGS_H__
