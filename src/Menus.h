#ifndef __MENUS_H__
#define __MENUS_H__

class Menus
{
public:
    static constexpr int MENUTYPE_LIST = 1;
    static constexpr int MENUTYPE_CONFIRM = 2;
    static constexpr int MENUTYPE_CONFIRM2 = 3;
    static constexpr int MENUTYPE_MAIN = 4;
    static constexpr int MENUTYPE_HELP = 5;
    static constexpr int MENUTYPE_VCENTER = 6;
    static constexpr int MENUTYPE_NOTEBOOK = 7;
    static constexpr int MENUTYPE_MAIN_LIST = 8;
    static constexpr int MENUTYPE_VENDING_MACHINE = 9;
    static constexpr int ITEM_NORMAL = 0;
    static constexpr int ITEM_NOSELECT = 1;
    static constexpr int ITEM_NODEHYPHENATE = 2;
    static constexpr int ITEM_DISABLED = 4;
    static constexpr int ITEM_ALIGN_CENTER = 8;
    static constexpr int ITEM_SHOWDETAILS = 32;
    static constexpr int ITEM_DIVIDER = 64;
    static constexpr int ITEM_SELECTOR = 128;
    static constexpr int ITEM_BLOCK_TEXT = 256;
    static constexpr int ITEM_HIGHLIGHT = 512;
    static constexpr int ITEM_CHECKED = 1024;
    static constexpr int ITEM_RIGHT_ARROW = 8192;
    static constexpr int ITEM_LEFT_ARROW = 16384;
    static constexpr int ITEM_HIDDEN = 32768;
    static constexpr int ITEM_SCROLLBAR = 0x20000; // [GEC]
    static constexpr int ITEM_SCROLLBARTWO = 0x40000; // [GEC]
    static constexpr int ITEM_DISABLEDTWO = 0x80000; // [GEC]
    static constexpr int ITEM_PADDING = 0x100000; // [GEC]
    static constexpr int ITEM_BINDING = 0x200000; // [GEC]
    static constexpr int VENDING_MACHINE_LEFT_INDENT_AMOUNT = 9;
    static constexpr int VENDING_MACHINE_RIGHT_INDENT_AMOUNT = 9;
    static constexpr int VENDING_MACHINE_TOP_MARGIN_AMOUNT = 13;
    static constexpr int VENDING_MACHINE_ITEM_PADDING = 3;
    static constexpr int VENDING_MACHINE_HELP_PADDING = 13;
    static constexpr int VENDING_MACHINE_DIVIDER_SPACE = 2;
    static constexpr int INVENTORY_SCREEN_TOP_PADDING = 8;
    static constexpr int INVENTORY_SCREEN_HELP_PADDING = 10;
    static constexpr int INVENTORY_SCREEN_STATUS_BAR_OFFSET_Y = -4;
#if 0 // Old
    static constexpr int MENU_NONE = 0;
    static constexpr int MENU_LEVEL_STATS = 1;
    static constexpr int MENU_DRAWSWORLD = 2;
    static constexpr int MENU_MAIN_BEGIN = 3;
    static constexpr int MENU_MAIN = 3;
    static constexpr int MENU_MAIN_HELP = 4;
    static constexpr int MENU_MAIN_ARMORHELP = 5;
    static constexpr int MENU_MAIN_EFFECTHELP = 6;
    static constexpr int MENU_MAIN_ITEMHELP = 7;
    static constexpr int MENU_MAIN_ABOUT = 8;
    static constexpr int MENU_MAIN_GENERAL = 9;
    static constexpr int MENU_MAIN_MOVE = 10;
    static constexpr int MENU_MAIN_ATTACK = 11;
    static constexpr int MENU_MAIN_SNIPER = 12;
    static constexpr int MENU_MAIN_EXIT = 13;
    static constexpr int MENU_MAIN_CONFIRMNEW = 14;
    static constexpr int MENU_MAIN_CONFIRMNEW2 = 15;
    static constexpr int MENU_MAIN_DIFFICULTY = 16;
    static constexpr int MENU_MAIN_OPTIONS = 17;
    static constexpr int MENU_MAIN_MINIGAME = 18;
    static constexpr int MENU_MAIN_MORE_GAMES = 19;
    static constexpr int MENU_MAIN_HACKER_HELP = 20;
    static constexpr int MENU_MAIN_MATRIX_SKIP_HELP = 21;
    static constexpr int MENU_MAIN_POWER_UP_HELP = 22;
    static constexpr int MENU_SELECT_LANGUAGE = 23;
    static constexpr int MENU_MAIN_END = 23;
    static constexpr int MENU_END_RANKING = 24;
    static constexpr int MENU_ENABLE_SOUNDS = 25;
    static constexpr int MENU_END_= 26;
    static constexpr int MENU_END_FINALQUIT = 27;
    static constexpr int MENU_INHERIT_BACKMENU = 28;
    static constexpr int MENU_INGAME = 29;
    static constexpr int MENU_INGAME_STATUS = 30;
    static constexpr int MENU_INGAME_PLAYER = 31;
    static constexpr int MENU_INGAME_LEVEL = 32;
    static constexpr int MENU_INGAME_GRADES = 33;
    static constexpr int MENU_INGAME_OPTIONS = 35;
    static constexpr int MENU_INGAME_LANGUAGE = 36;
    static constexpr int MENU_INGAME_HELP = 37;
    static constexpr int MENU_INGAME_GENERAL = 38;
    static constexpr int MENU_INGAME_MOVE = 39;
    static constexpr int MENU_INGAME_ATTACK = 40;
    static constexpr int MENU_INGAME_SNIPER = 41;
    static constexpr int MENU_INGAME_EXIT = 42;
    static constexpr int MENU_INGAME_ARMORHELP = 43;
    static constexpr int MENU_INGAME_EFFECTHELP = 44;
    static constexpr int MENU_INGAME_ITEMHELP = 45;
    static constexpr int MENU_INGAME_QUESTLOG = 46;
    static constexpr int MENU_INGAME_RECIPES = 47;
    static constexpr int MENU_INGAME_SAVE = 48;
    static constexpr int MENU_INGAME_LOAD = 49;
    static constexpr int MENU_INGAME_LOADNOSAVE = 50;
    static constexpr int MENU_INGAME_DEAD = 51;
    static constexpr int MENU_INGAME_RESTARTLVL = 52;
    static constexpr int MENU_INGAME_SAVEQUIT = 53;
    static constexpr int MENU_INGAME_KICKING = 57;
    static constexpr int MENU_INGAME_SPECIAL_EXIT = 58;
    static constexpr int MENU_INGAME_HACKER_HELP = 59;
    static constexpr int MENU_INGAME_MATRIX_SKIP_HELP = 60;
    static constexpr int MENU_INGAME_POWER_UP_HELP = 61;
    static constexpr int MENU_INGAME_CONTROLS = 62;
    //static constexpr int MENU_SMS_PROMPT = 61;
    //static constexpr int MENU_SMS_RETRY = 63;
    //static constexpr int MENU_SMS_ERROR = 64;
    //static constexpr int MENU_SMS_PHONES = 65;
    static constexpr int MENU_DEBUG = 65;
    static constexpr int MENU_DEBUG_MAPS = 66;
    static constexpr int MENU_DEBUG_STATS = 67;
    static constexpr int MENU_DEBUG_CHEATS = 68;
    static constexpr int MENU_DEVELOPER = 68;
    static constexpr int MENU_DEVELOPER_VARS = 69;
    static constexpr int MENU_DEBUG_SYS = 70;
    static constexpr int MENU_SHOWDETAILS = 71;
    static constexpr int MENU_ITEMS = 72;
    static constexpr int MENU_ITEMS_WEAPONS = 73;
    static constexpr int MENU_ITEMS_DRINKS = 75;
    static constexpr int MENU_ITEMS_CONFIRM = 77;
    static constexpr int MENU_ITEMS_HEALTHMSG = 79;
    static constexpr int MENU_ITEMS_ARMORMSG = 80;
    static constexpr int MENU_ITEMS_SYRINGEMSG = 81;
    static constexpr int MENU_ITEMS_HOLY_WATER_MAX = 82;
    static constexpr int MENU_VENDING_MACHINE = 83;
    static constexpr int MENU_VENDING_MACHINE_DRINKS = 84;
    static constexpr int MENU_VENDING_MACHINE_SNACKS = 85;
    static constexpr int MENU_VENDING_MACHINE_CONFIRM = 86;
    static constexpr int MENU_VENDING_MACHINE_CANT_BUY = 87;
    static constexpr int MENU_VENDING_MACHINE_DETAILS = 88;
    static constexpr int MENU_VENDING_MACHINE_LAST = 88;
    static constexpr int MENU_COMIC_BOOK = 89;
#endif
    static constexpr int ACTION_NONE = 0;
    static constexpr int ACTION_GOTO = 1;
    static constexpr int ACTION_BACK = 2;
    static constexpr int ACTION_LOAD = 3;
    static constexpr int ACTION_SAVE = 4;
    static constexpr int ACTION_BACKTOMAIN = 5;
    static constexpr int ACTION_TOGSOUND = 6;
    static constexpr int ACTION_NEWGAME = 7;
    static constexpr int ACTION_EXIT = 8;
    static constexpr int ACTION_CHANGESTATE = 9;
    static constexpr int ACTION_DIFFICULTY = 10;
    static constexpr int ACTION_RETURNTOGAME = 11;
    static constexpr int ACTION_RESTARTLEVEL = 12;
    static constexpr int ACTION_SAVEQUIT = 13;
    static constexpr int ACTION_OFFERSUCCESS = 14;
    //static constexpr int ACTION_TOGVIBRATE = 15; //J2ME
    static constexpr int ACTION_CHANGESFXVOLUME = 15;
    static constexpr int ACTION_SHOWDETAILS = 16;
    static constexpr int ACTION_CHANGEMAP = 17;
    static constexpr int ACTION_USEITEMWEAPON = 18;
    static constexpr int ACTION_SELECT_LANGUAGE = 19;
    static constexpr int ACTION_USEITEMSYRING = 20;
    static constexpr int ACTION_USEITEMOTHER = 21;
    static constexpr int ACTION_CONTINUE = 22;
    static constexpr int ACTION_MAIN_SPECIAL = 23;
    static constexpr int ACTION_CONFIRMUSE = 24;
    static constexpr int ACTION_SAVEEXIT = 25;
    static constexpr int ACTION_BACKTWO = 26;
    static constexpr int ACTION_MINIGAME = 27;
    static constexpr int ACTION_CONFIRMBUY = 28;
    static constexpr int ACTION_BUYDRINK = 29;
    static constexpr int ACTION_BUYSNACK = 30;
    static constexpr int ACTION_RETURN_TO_PLAYER = 33;
    static constexpr int ACTION_FLIP_CONTROLS = 35; // IOS
    static constexpr int ACTION_CONTROL_LAYOUT = 36; // IOS
    static constexpr int ACTION_CHANGEMUSICVOLUME = 37; // [GEC]
    static constexpr int ACTION_CHANGEALPHA = 38; // [GEC]
    static constexpr int ACTION_CHANGE_VID_MODE = 39; // [GEC]
    static constexpr int ACTION_TOG_VSYNC = 40; // [GEC]
    static constexpr int ACTION_CHANGE_RESOLUTION = 41; // [GEC]
    static constexpr int ACTION_APPLY_CHANGES = 42; // [GEC]
    static constexpr int ACTION_SET_BINDING = 43; // [GEC]
    static constexpr int ACTION_DEFAULT_BINDINGS = 44; // [GEC]
    static constexpr int ACTION_TOG_VIBRATION = 45; // [GEC]
    static constexpr int ACTION_CHANGE_VIBRATION_INTENSITY = 46; // [GEC]
    static constexpr int ACTION_CHANGE_DEADZONE = 47; // [GEC]
    static constexpr int ACTION_TOG_TINYGL = 48; // [GEC]
    static constexpr int ACTION_DEBUG = 100;
    static constexpr int ACTION_GIVEALL = 102;
    static constexpr int ACTION_GIVEMAP = 103;
    static constexpr int ACTION_NOCLIP = 104;
    static constexpr int ACTION_DISABLEAI = 105;
    static constexpr int ACTION_NOHELP = 106;
    static constexpr int ACTION_GODMODE = 107;
    static constexpr int ACTION_SHOWLOCATION = 108;
    static constexpr int ACTION_RFRAMES = 109;
    static constexpr int ACTION_RSPEEDS = 110;
    static constexpr int ACTION_RSKIPFLATS = 111;
    static constexpr int ACTION_RSKIPCULL = 112;
    static constexpr int ACTION_RSKIPBSP = 114;
    static constexpr int ACTION_RSKIPLINES = 115;
    static constexpr int ACTION_RSKIPSPRITES = 116;
    static constexpr int ACTION_RONLYRENDER = 117;
    static constexpr int ACTION_RSKIPDECALS = 118;
    static constexpr int ACTION_RSKIP2DSTRETCH = 119;
    static constexpr int ACTION_DRIVING_MODE = 120;
    static constexpr int ACTION_RENDER_MODE = 121;
    static constexpr int ACTION_EQUIPFORMAP = 122;
    static constexpr int ACTION_ONESHOT = 123;
    static constexpr int ACTION_DEBUG_FONT = 124;
    static constexpr int ACTION_SYS_TEST = 125;
    static constexpr int ACTION_SKIP_MINIGAMES = 126;
    static constexpr int ACTION_SHOW_HEAP = 127;
    static constexpr int LIST_TEXT_OFSX = 12;

    enum menus {
        MENU_MAIN_CONTROLLER = -6, // [GEC]
        MENU_MAIN_BINDINGS = -5, // [GEC]
        MENU_MAIN_CONTROLS = -4, // [GEC]
        MENU_MAIN_OPTIONS_SOUND = -3, // [GEC]
        MENU_MAIN_OPTIONS_VIDEO = -2, // [GEC]
        MENU_MAIN_OPTIONS_INPUT = -1, // [GEC]
        MENU_NONE = 0,
        MENU_LEVEL_STATS,
        MENU_DRAWSWORLD,
        MENU_MAIN_BEGIN,
        MENU_MAIN = MENU_MAIN_BEGIN,
        MENU_MAIN_HELP,
        MENU_MAIN_ARMORHELP,
        MENU_MAIN_EFFECTHELP,
        MENU_MAIN_ITEMHELP,
        MENU_MAIN_ABOUT,
        MENU_MAIN_GENERAL,
        MENU_MAIN_MOVE,
        MENU_MAIN_ATTACK,
        MENU_MAIN_SNIPER,
        MENU_MAIN_EXIT,
        MENU_MAIN_CONFIRMNEW,
        MENU_MAIN_CONFIRMNEW2,
        MENU_MAIN_DIFFICULTY,
        MENU_MAIN_OPTIONS,
        MENU_MAIN_MINIGAME,
        MENU_MAIN_MORE_GAMES,
        MENU_MAIN_HACKER_HELP,
        MENU_MAIN_MATRIX_SKIP_HELP,
        MENU_MAIN_POWER_UP_HELP,
        MENU_MAIN_END,
        MENU_SELECT_LANGUAGE = MENU_MAIN_END,
        MENU_END_RANKING,
        MENU_ENABLE_SOUNDS,
        MENU_END_,
        MENU_END_FINALQUIT,
        MENU_INHERIT_BACKMENU,
        MENU_INGAME,
        MENU_INGAME_STATUS,
        MENU_INGAME_PLAYER,
        MENU_INGAME_LEVEL,
        MENU_INGAME_GRADES,
        MENU_NULL01,
        MENU_INGAME_OPTIONS,
        MENU_INGAME_LANGUAGE,
        MENU_INGAME_HELP,
        MENU_INGAME_GENERAL,
        MENU_INGAME_MOVE,
        MENU_INGAME_ATTACK,
        MENU_INGAME_SNIPER,
        MENU_INGAME_EXIT,
        MENU_INGAME_ARMORHELP,
        MENU_INGAME_EFFECTHELP,
        MENU_INGAME_ITEMHELP,
        MENU_INGAME_QUESTLOG,
        MENU_INGAME_RECIPES,
        MENU_INGAME_SAVE,
        MENU_INGAME_LOAD,
        MENU_INGAME_LOADNOSAVE,
        MENU_INGAME_DEAD,
        MENU_INGAME_RESTARTLVL,
        MENU_INGAME_SAVEQUIT,
        MENU_INGAME_BINDINGS, // [GEC] MENU_NULL02,
        MENU_INGAME_OPTIONS_SOUND, // [GEC] MENU_NULL03,
        MENU_INGAME_OPTIONS_VIDEO, // [GEC] MENU_NULL04,
        MENU_INGAME_KICKING,
        MENU_INGAME_SPECIAL_EXIT,
        MENU_INGAME_HACKER_HELP,
        MENU_INGAME_MATRIX_SKIP_HELP,
        MENU_INGAME_POWER_UP_HELP,
        MENU_INGAME_CONTROLS,
        MENU_INGAME_OPTIONS_INPUT, // [GEC] MENU_NULL05,
        MENU_INGAME_CONTROLLER, // [GEC] MENU_NULL06,
        MENU_DEBUG,
        MENU_DEBUG_MAPS,
        MENU_DEBUG_STATS,
        MENU_DEBUG_CHEATS,
        MENU_DEVELOPER = MENU_DEBUG_CHEATS,
        MENU_DEVELOPER_VARS,
        MENU_DEBUG_SYS,
        MENU_SHOWDETAILS,
        MENU_ITEMS,
        MENU_ITEMS_WEAPONS,
        MENU_NULL07,
        MENU_ITEMS_DRINKS,
        MENU_NULL08,
        MENU_ITEMS_CONFIRM,
        MENU_NULL09,
        MENU_ITEMS_HEALTHMSG,
        MENU_ITEMS_ARMORMSG,
        MENU_ITEMS_SYRINGEMSG,
        MENU_ITEMS_HOLY_WATER_MAX,
        MENU_VENDING_MACHINE,
        MENU_VENDING_MACHINE_DRINKS,
        MENU_VENDING_MACHINE_SNACKS,
        MENU_VENDING_MACHINE_CONFIRM,
        MENU_VENDING_MACHINE_CANT_BUY,
        MENU_VENDING_MACHINE_DETAILS,
        MENU_VENDING_MACHINE_LAST = MENU_VENDING_MACHINE_DETAILS,
        MENU_COMIC_BOOK,
    };
};

#endif