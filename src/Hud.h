#ifndef __HUD_H__
#define __HUD_H__

class Image;
class Text;
class Entity;
class fmButtonContainer;
class Graphics;

class Hud
{
private:

public:
    static constexpr int MSG_DISPLAY_TIME = 700;
    static constexpr int MSG_FLASH_TIME = 100;
    static constexpr int SCROLL_START_DELAY = 750;
    static constexpr int MS_PER_CHAR = 64;
    static constexpr int MAX_MESSAGES = 5;
    static constexpr int REPAINT_EFFECTS = 1;
    static constexpr int REPAINT_TOP_BAR = 2;
    static constexpr int REPAINT_BOTTOM_BAR = 4;
    static constexpr int REPAINT_BUBBLE_TEXT = 8;
    static constexpr int REPAINT_SUBTITLES = 16;
    static constexpr int REPAINT_HUD_OVERDRAW = 32;
    static constexpr int REPAINT_DPAD = 64;
    static constexpr int REPAINT_PLAYING_FLAGS = 107;
    static constexpr int REPAINT_CAMERA_FLAGS = 24;
    static constexpr int MSG_FLAG_NONE = 0;
    static constexpr int MSG_FLAG_FORCE = 1;
    static constexpr int MSG_FLAG_CENTER = 2;
    static constexpr int MSG_FLAG_IMPORTANT = 4;
    static constexpr int STATUSBAR_ICON_PICKUP = 0;
    static constexpr int STATUSBAR_ICON_ATTACK = 1;
    static constexpr int STATUSBAR_ICON_CHAT = 2;
    static constexpr int STATUSBAR_ICON_USE = 3;
    static constexpr int HUDARROWS_SIZE = 12;
    static constexpr int BUBBLE_TEXT_TIME = 1500;
    static constexpr int SENTRY_BOT_ICONS_PADDING = 15;
    static constexpr int DAMAGE_OVERLAY_TIME = 1000;
    static constexpr int ACTION_ICON_SIZE = 18;

    static constexpr int MAX_WEAPON_BUTTONS = 15;


	int touchMe;
	int repaintFlags;
    Image* imgScope;
    Image* imgActions;
    Image* imgAttArrow;
    Image* imgDamageVignette;
    Image* imgDamageVignetteBot;
    Image* imgBottomBarIcons;
    Image* imgAmmoIcons;
    Image* imgSoftKeyFill;
    Image* imgCockpitOverlay;
    bool cockpitOverlayRaw;
    Image* imgPortraitsSM;
    Image* imgPlayerFaces;
    Image* imgPlayerActive;
    Image* imgPlayerFrameNormal;
    Image* imgPlayerFrameActive;
    Image* imgHudFill;
    Image* imgIce;
    Image* imgSentryBotFace;
    Image* imgSentryBotActive;
    bool isInWeaponSelect;
    Image* imgPanelTop;
    Image* imgPanelTopSentrybot;
    Image* imgWeaponNormal;
    Image* imgWeaponActive;
    Image* imgShieldNormal;
    Image* imgShieldButtonActive;
    Image* imgKeyNormal;
    Image* imgKeyActive;
    Image* imgHealthNormal;
    Image* imgHealthButtonActive;
    Image* imgSwitchRightNormal;
    Image* imgSwitchRightActive;
    Image* imgSwitchLeftNormal;
    Image* imgSwitchLeftActive;
    Image* imgVendingSoftkeyPressed;
    Image* imgVendingSoftkeyNormal;
    Image* imgInGameMenuSoftkey;
    Image* imgNumbers;
    Image* imgHudTest;
    Text* messages[Hud::MAX_MESSAGES];
    int messageFlags[Hud::MAX_MESSAGES];
    int msgCount;
    int msgTime;
    int msgDuration;
    int subTitleID;
    int subTitleTime;
    int cinTitleID;
    int cinTitleTime;
    Text* bubbleText;
    int bubbleTextTime;
    int bubbleColor;
    int damageTime;
    int damageCount;
    int damageDir;
    Entity* lastTarget;
    int monsterStartHealth;
    int monsterDestHealth;
    int playerStartHealth;
    int playerDestHealth;
    int monsterHealthChangeTime;
    int playerHealthChangeTime;
    bool showCinPlayer;
    int drawTime;
    fmButtonContainer* m_hudButtons;
    fmButtonContainer* m_weaponsButtons;
    int weaponPressTime;

	// Constructor
	Hud();
	// Destructor
	~Hud();

	bool startup();
    void shiftMsgs();
    void calcMsgTime();
    void addMessage(short i);
    void addMessage(short i, short i2);
    void addMessage(short i, int i2);
    void addMessage(short i, short i2, int i3);
    void addMessage(Text* text);
    void addMessage(Text* text, int flags);
    Text* getMessageBuffer();
    Text* getMessageBuffer(int flags);
    void finishMessageBuffer();
    bool isShiftingCenterMsg();
    void drawTopBar(Graphics* graphics);
    void drawImportantMessage(Graphics* graphics, Text* text, int color);
    void drawCenterMessage(Graphics* graphics, Text* text, int color);
    void drawCinematicText(Graphics* graphics);
    void drawEffects(Graphics* graphics);
    void drawDamageVignette(Graphics* graphics);
    void smackScreen(int vScrollVelocity);
    void stopScreenSmack();
    void brightenScreen(int maxLocalBrightness, int brightnessInitialBoost);
    void stopBrightenScreen();
    void drawOverlay(Graphics* graphics);
    void drawHudOverdraw(Graphics* graphics);
    void drawBottomBar(Graphics* graphics);
    void draw(Graphics* graphics);
    void drawMonsterHealth(Graphics* graphics);
    void showSpeechBubble(int i, int i2);
    void drawBubbleText(Graphics* graphics);
    void drawArrowControls(Graphics* graphics);
    void drawWeapon(Graphics* graphics, int x, int y, int weapon, bool highlighted);
    void drawNumbers(Graphics* graphics, int x, int y, int space, int num, int weapon);
    void drawCurrentKeys(Graphics* graphics, int x, int y);
    void drawWeaponSelection(Graphics* graphics);
    void handleUserMoved(int pressX, int pressY);
    void handleUserTouch(int pressX, int pressY, bool highlighted);
    void update();
};

#endif