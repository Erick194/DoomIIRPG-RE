#ifndef __MENUSYSTEM_H__
#define __MENUSYSTEM_H__

#include "Text.h"
#include "MenuItem.h"
#include "MenuStrings.h"

class EntityDef;
class Image;
class MenuItem;
class Text;
class fmButtonContainer;
class fmScrollButton;
class Graphics;
class MenuSystem;

class MenuSystem
{
private:

public:
	static constexpr int NO = 0;
	static constexpr int YES = 1;

	static constexpr int INDEX_UAC_CREDITS = 0;
	static constexpr int INDEX_WEAPONS = 1;
	static constexpr int INDEX_ENERGY_DRINKS = 2;
	static constexpr int INDEX_OTHER = 3;
	static constexpr int INDEX_ITEMS = 4;
	static constexpr int MAX_SAVED_INDEXES = 5;

	static constexpr int BLOCK_LINE_MASK = 63;
	static constexpr int BLOCK_NUMLINES_SHIFT = 26;
	static constexpr int BLOCK_CURLINE_SHIFT = 20;
	static constexpr int BLOCK_OFS_MASK = 1023;
	static constexpr int BLOCK_CHARS_TO_DRAW_SHIFT = 10;
	static constexpr int MAX_MENUITEMS = 50;
	static constexpr int EMPTY_TEXT = Localization::STRINGID(Strings::FILE_MENUSTRINGS, MenuStrings::EMPTY_STRING);
	static constexpr short MAIN_BG_OFFSET_Y = 20;
	static constexpr int MAXSTACKCOUNT = 10;
	static constexpr int TOP_Y = 1;
	static constexpr int MAX_MORE_GAMES = 4;
	static constexpr int TITLE_VENDING_Y_OFFSET = 8;
	static constexpr int TOUCH_MENU_TYPE_MAIN = 0;
	static constexpr int TOUCH_MENU_TYPE_INGAME = 1;
	static constexpr int TOUCH_MENU_TYPE_VENDING = 2;
    static constexpr int MAIN_MENU_PADDING_LEFT = 22;
    static constexpr int MAIN_MENU_PADDING_RIGHT = 23;
    static constexpr int MAIN_MENU_PADDING_TOP = 23;
    static constexpr int MAIN_MENU_PADDING_BOTTOM = 18;
    static constexpr int MAIN_MENU_SCROLL_BAR_WIDTH = 15;
    static constexpr int MAIN_MENU_SCROLL_HANDLE_WIDTH = 30;
    static constexpr int MAIN_MENU_SCROLL_HANDLE_HEIGHT = 36;
    static constexpr int MAIN_MENU_SCROLL_HANDLE_PADDING_TOP = -3;
    static constexpr int MAIN_MENU_SCROLL_HANDLE_PADDING_LEFT = -10;
    static constexpr int MAIN_MENU_ITEM_HEIGHT = 41;
    static constexpr int MAIN_MENU_ITEM_PADDING_BOTTOM = 9;
    static constexpr int MAIN_MENU_ITEM_INFO_WIDTH = 0;
    static constexpr int MAIN_MENU_ITEM_INFO_PADDING_LEFT = 0;
    static constexpr int MAIN_MENU_TITLE_PADDING_ABOVE = 10;
    static constexpr int MAIN_MENU_TITLE_PADDING_BELOW = 25;
    static constexpr int INGAME_MENU_PADDING_LEFT = 17;
    static constexpr int INGAME_MENU_PADDING_RIGHT = 24;
    static constexpr int INGAME_MENU_PADDING_TOP = 23;
    static constexpr int INGAME_MENU_PADDING_BOTTOM = 19;
    static constexpr int INGAME_MENU_SCROLL_BAR_WIDTH = 20;
    static constexpr int INGAME_MENU_SCROLL_HANDLE_WIDTH = 24;
    static constexpr int INGAME_MENU_SCROLL_HANDLE_HEIGHT = 62;
    static constexpr int INGAME_MENU_SCROLL_HANDLE_PADDING_TOP = 10;
    static constexpr int INGAME_MENU_SCROLL_HANDLE_PADDING_LEFT = -3;
    static constexpr int INGAME_MENU_ITEM_HEIGHT = 20;
    static constexpr int INGAME_MENU_ITEM_PADDING_BOTTOM = 17;
    static constexpr int INGAME_MENU_ITEM_INFO_WIDTH = 24;
    static constexpr int INGAME_MENU_ITEM_INFO_PADDING_LEFT = 3;
    static constexpr int INGAME_MENU_TITLE_PADDING_ABOVE = 80;
    static constexpr int INGAME_MENU_TITLE_PADDING_BELOW = 25;
    static constexpr int VENDING_MENU_PADDING_LEFT = 10;
    static constexpr int VENDING_MENU_PADDING_RIGHT = 11;
    static constexpr int VENDING_MENU_PADDING_TOP = 15;
    static constexpr int VENDING_MENU_PADDING_BOTTOM = 4;
    static constexpr int VENDING_MENU_SCROLL_BAR_WIDTH = 31;
    static constexpr int VENDING_MENU_SCROLL_HANDLE_WIDTH = 0;
    static constexpr int VENDING_MENU_SCROLL_HANDLE_HEIGHT = 0;
    static constexpr int VENDING_MENU_SCROLL_HANDLE_PADDING_TOP = 0;
    static constexpr int VENDING_MENU_SCROLL_HANDLE_PADDING_LEFT = 0;
    static constexpr int VENDING_MENU_ITEM_HEIGHT = 59;
    static constexpr int VENDING_MENU_ITEM_PADDING_BOTTOM = 10;
    static constexpr int VENDING_MENU_ITEM_INFO_WIDTH = 38;
    static constexpr int VENDING_MENU_ITEM_INFO_PADDING_LEFT = 4;
    static constexpr int VENDING_MAIN_MENU_ITEM_HEIGHT = 36;
    static constexpr int VENDING_MAIN_MENU_ITEM_PADDING_BOTTOM = 16;
    static constexpr int VENDING_MENU_TITLE_PADDING_ABOVE = 0;
    static constexpr int VENDING_MENU_TITLE_PADDING_BELOW = 11;
    static constexpr int VENDING_MAIN_MENU_TITLE_PADDING_BELOW = 21;
    static constexpr int MENU_TOP_SECTION_HEIGHT = 250;
    static constexpr int MENU_TOP_SECTION_OVERLAP = 0;
    static constexpr int MENU_SOFTKEY_WIDTH = 99;
    static constexpr int MENU_SOFTKEY_HEIGHT = 37;
    static constexpr int MENU_SOFTKEY_PADDING_ABOVE = 16;
    static constexpr int MENU_SOFTKEY_PADDING_SIDE = 19;
    static constexpr int MENU_TOP_SECTION_HEIGHT_VENDING = 223;
    static constexpr int MENU_TOP_SECTION_OVERLAP_VENDING = 0;
    static constexpr int MENU_SOFTKEY_WIDTH_VENDING = 104;
    static constexpr int MENU_SOFTKEY_HEIGHT_VENDING = 36;
    static constexpr int MENU_SOFTKEY_PADDING_ABOVE_VENDING = 0;
    static constexpr int MENU_SOFTKEY_PADDING_SIDE_VENDING = 14;
    static constexpr int MENU_NON_FULL_SCREEN_OFFSET_Y = 144;
    static constexpr int POP_UP_WIDTH = 240;
    static constexpr int POP_UP_TEXT_OFFSET_X = 25;
    static constexpr int UP_ARROW_OFFSET_X = 151;
    static constexpr int UP_ARROW_OFFSET_Y = 100;
    static constexpr int DOWN_ARROW_PADDING_ABOVE = 32;
    static constexpr int ARROW_WIDTH = 45;
    static constexpr int ARROW_HEIGHT = 39;
    static constexpr int SOFTKEY_PRESS_LEFT = 0;
    static constexpr int SOFTKEY_PRESS_RIGHT = 1;

    int touchMe;
    uint32_t* menuData;
    uint32_t menuDataCount;
    uint32_t* menuItems;
    uint32_t menuItemsCount;
    short LEVEL_STATS_nextMap;
    int menuParam;
    EntityDef* detailsDef;
    int indexes[10];
    Image* imgMainBG;
    Image* imgLogo;
    Image* background;
    bool drawLogo;
    int lastOffer;
    MenuItem items[50];
    int numItems;
    int menu;
    int oldMenu;
    int selectedIndex;
    int scrollIndex;
    int type;
    int maxItems;
    int cheatCombo;
    int startTime;
    int menuMode;
    int stackCount;
    int menuStack[10];
    uint8_t menuIdxStack[10];
    int poppedIdx[1];
    Text* detailsTitle;
    Text* detailsHelpText;
    bool goBackToStation;
    int moreGamesPage;
    bool changeSfxVolume;
    int oldLanguageSetting;
    int sfxVolumeScroll;
    int musicVolumeScroll;
    int alphaScroll;
    bool updateSlider;
    int sliderID;
    bool drawHelpText;
    int selectedHelpIndex;
    fmButtonContainer* m_menuButtons;
    fmButtonContainer* m_infoButtons;
    fmScrollButton* m_scrollBar;
    fmButtonContainer* m_vendingButtons;
    bool isMainMenuScrollBar;
    bool isMainMenu;
    int menuItem_fontPaddingBottom;
    int menuItem_paddingBottom;
    int menuItem_height;
    int menuItem_width;
    Image* imgVendingButtonLarge;
    Image* imgInGameMenuOptionButton;
    Image* imgMenuButtonBackground;
    Image* imgMenuButtonBackgroundOn;
    Image* imgMenuArrowDown;
    Image* imgMenuArrowUp;
    Image* imgVendingArrowUpGlow;
    Image* imgVendingArrowDownGlow;
    Image* imgMenuDial;
    Image* imgMenuDialKnob;
    Image* imgMenuMainBOX;
    Image* imgMainMenuOverLay;
    Image* imgMainHelpOverLay;
    Image* imgMainAboutOverLay;
    Image* imgMenuYesNoBOX;
    Image* imgMenuChooseDIFFBOX;
    Image* imgMenuLanguageBOX;
    Image* imgSwitchLeftNormal;
    Image* imgSwitchLeftActive;
    Image* imgMenuOptionBOX3;
    Image* imgMenuOptionBOX4;
    Image* imgMenuOptionSliderBar;
    Image* imgMenuOptionSliderON;
    Image* imgMenuOptionSliderOFF;
    Image* imgHudNumbers;
    Image* imgGameMenuPanelbottom;
    Image* imgGameMenuPanelBottomSentrybot;
    Image* imgGameMenuHealth;
    Image* imgGameMenuShield;
    Image* imgGameMenuInfoButtonPressed;
    Image* imgGameMenuInfoButtonNormal;
    Image* imgVendingButtonHelp;
    Image* imgGameMenuTornPage;
    Image* imgMainMenuDialA_Anim;
    Image* imgMainMenuDialC_Anim;
    Image* imgMainMenuSlide_Anim;
    Image* imgGameMenuScrollBar;
    Image* imgGameMenuTopSlider;
    Image* imgGameMenuMidSlider;
    Image* imgGameMenuBottomSlider;
    Image* imgVendingScrollBar;
    Image* imgVendingScrollButtonTop;
    Image* imgVendingScrollButtonMiddle;
    Image* imgVendingScrollButtonBottom;
    Image* imgGameMenuBackground;
    int dialA_Anim1;
    int dialC_Anim1;
    int dialA_Anim2;
    int dialC_Anim2;
    int slideAnim1;
    int slideAnim2;
    int animTime;

    // [GEC]
    bool changeMusicVolume;
    bool changeButtonsAlpha;
    bool changeValues;
    int old_0x44;
    int old_0x48;
    int scrollY1Stack[10];
    int scrollY2Stack[10];
    int scrollI2Stack[10];
    int nextMsgTime;
    int nextMsg;
    bool setBinding;
    Image* imgMenuButtonBackgroundExt;
    Image* imgMenuButtonBackgroundExtOn;

    Image* imgMenuBtnBackground; // [GEC]
    Image* imgMenuBtnBackgroundOn; // [GEC]
    bool changeVibrationIntensity; // [GEC]
    bool changeDeadzone; // [GEC]
    int vibrationIntensityScroll; // [GEC]
    int deadzoneScroll; // [GEC]
    int resolutionIndex; // [GEC]

	// Constructor
	MenuSystem();
	// Destructor
	~MenuSystem();

	bool startup();
    void buildDivider(Text* text, int i);
    bool enterDigit(int i);
    void scrollDown();
    void scrollUp();
    bool scrollPageDown();
    void scrollPageUp();
    bool shiftBlockText(int n, int i, int j);
    void moveDir(int n);
    void doDetailsSelect();
    void back();
    void setMenu(int menu);
    void paint(Graphics* graphics);
    void setItemsFromText(int i, Text* text, int i2, int i3, int i4);
    void returnToGame();
    void initMenu(int menu);
    void gotoMenu(int menu);
    void handleMenuEvents(int key, int keyAction);
    void select(int i);
    void infiniteLoop();
    int infiniteRecursion(int* array);
    void systemTest(int sysType);
    void startGame(bool b);
    void SetYESNO(short i, int i2, int i3, int i4);
    void SetYESNO(short i, int i2, int i3, int i4, int i5, int i6);
    void SetYESNO(Text* text, int i, int i2, int i3, int i4, int i5);
    void LoadHelpResource(short i);
    void FillRanking();
    void LoadNotebook();
    void LoadHelpItems(Text* text, int i);
    void buildFraction(int i, int i2, Text* text);
    void buildModStat(int i, int i2, Text* text);
    void buildLevelGrades(Text* text);
    void buildLevelGrade(int i, Text* text, int i2, int i3);
    void fillStatus(bool b, bool b2, bool b3);
    void saveIndexes(int i);
    void loadIndexes(int i);
    void showDetailsMenu();
    void addItem(int textField, int textField2, int flags, int action, int param, int helpField);
    void loadMenuItems(int menu, int begItem, int numItems);
    int onOffValue(bool b);
    void leaveOptionsMenu();
    int getStackCount();
    void clearStack();
    void pushMenu(int i, int i2, int Y1, int Y2, int index2);
    int popMenu(int* array, int *Y1, int* Y2, int *index2);
    int peekMenu();
    int getLastArgString();
    void fillVendingMachineSnacks(int i, Text* text);

    void setMenuSettings();
    void updateTouchButtonState();
    void handleUserTouch(int x, int y, bool b);
    void handleUserMoved(int x, int y);
    int getScrollPos();
    int getMenuItemHeight(int i);
    int getMenuItemHeight2(int i); // [GEC]
    void drawScrollbar(Graphics* graphics);
    void drawButtonFrame(Graphics* graphics);
    void drawTouchButtons(Graphics* graphics, bool b);
    void drawSoftkeyButtons(Graphics* graphics);
    int drawCustomScrollbar(Graphics* graphics, MenuItem *item, Text* text, int yPos); // [GEC]
    void drawOptionsScreen(Graphics* graphics);
    void drawNumbers(Graphics* graphics, int x, int y, int space, int number);
    bool HasVibration();
    bool isUserMusicOn();
    bool updateVolumeSlider(int buttonId, int x);
    void refresh();
    void soundClick();
};

#endif