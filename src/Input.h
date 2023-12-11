#ifndef __INPUT_H__
#define __INPUT_H__


//------------------------------------------------------------------------------------------------------------------------------------------
// Enum representing an input source on a non-generic game controller recognized by SDL (axis or button)
//------------------------------------------------------------------------------------------------------------------------------------------
enum class GamepadInput : uint8_t {
	BTN_A,
	BTN_B,
	BTN_X,
	BTN_Y,
	BTN_BACK,
	BTN_GUIDE,
	BTN_START,
	BTN_LEFT_STICK,
	BTN_RIGHT_STICK,
	BTN_LEFT_SHOULDER,
	BTN_RIGHT_SHOULDER,
	BTN_DPAD_UP,
	BTN_DPAD_DOWN,
	BTN_DPAD_LEFT,
	BTN_DPAD_RIGHT,
	AXIS_LEFT_X,
	AXIS_LEFT_Y,
	AXIS_RIGHT_X,
	AXIS_RIGHT_Y,
	AXIS_TRIG_LEFT,
	AXIS_TRIG_RIGHT,
	BTN_LAXIS_UP,		// L-Axis Up
	BTN_LAXIS_DOWN,		// L-Axis Down
	BTN_LAXIS_LEFT,		// L-Axis Left
	BTN_LAXIS_RIGHT,	// L-Axis Right
	BTN_RAXIS_UP,		// R-Axis Up
	BTN_RAXIS_DOWN,		// R-Axis Down
	BTN_RAXIS_LEFT,		// R-Axis Left
	BTN_RAXIS_RIGHT,	// R-Axis Right
	// N.B: must keep last for 'NUM_GAMEPAD_INPUTS' constant!
	INVALID
};

static constexpr uint8_t NUM_GAMEPAD_INPUTS = (uint32_t)GamepadInput::INVALID;

// Direction for a joystick hat (d-pad)
enum JoyHatDir : uint8_t {
	Up = 0,
	Down = 1,
	Left = 2,
	Right = 3
};

// Holds a joystick hat (d-pad) direction and hat number
union JoyHat {
	struct {
		JoyHatDir   hatDir;
		uint8_t     hatNum;
	} fields;

	uint16_t bits;

	inline JoyHat() noexcept : bits() {}
	inline JoyHat(const uint16_t bits) noexcept : bits(bits) {}

	inline JoyHat(const JoyHatDir dir, const uint8_t hatNum) noexcept : bits() {
		fields.hatDir = dir;
		fields.hatNum = hatNum;
	}

	inline operator uint16_t() const noexcept { return bits; }

	inline bool operator == (const JoyHat& other) const noexcept { return (bits == other.bits); }
	inline bool operator != (const JoyHat& other) const noexcept { return (bits != other.bits); }
};

//static_assert(sizeof(JoyHat) == 2);

// Holds the current state of a generic joystick axis
struct JoystickAxis {
	uint32_t    axis;
	float       value;
};
//---------------

extern char buttonNames[][NUM_GAMEPAD_INPUTS];

#define KEYBINDS_MAX 10
#define IS_MOUSE_BUTTON			0x100000
#define IS_CONTROLLER_BUTTON	0x200000
typedef struct keyMapping_s {
	int avk_action;
	int keyBinds[KEYBINDS_MAX];
} keyMapping_t;

#define KEY_MAPPIN_MAX 16
extern keyMapping_t keyMapping[KEY_MAPPIN_MAX];
extern keyMapping_t keyMappingTemp[KEY_MAPPIN_MAX];
extern keyMapping_t keyMappingDefault[KEY_MAPPIN_MAX];

extern int      gDeadZone;
extern int		gVibrationIntensity;
extern float    gBegMouseX;
extern float    gBegMouseY;
extern float    gCurMouseX;
extern float    gCurMouseY;

enum _AVKType {
	AVK_UNDEFINED = -1,	// hex 0xE010; dec 57360
	//AVK_FIRST = 1,		// hex 0xE020; dec 57376

	AVK_0,		// hex 0xE021; dec 57377
	AVK_1,		// hex 0xE022; dec 57378
	AVK_2,		// hex 0xE023; dec 57379
	AVK_3,		// hex 0xE024; dec 57380
	AVK_4,		// hex 0xE025; dec 57381
	AVK_5,		// hex 0xE026; dec 57382
	AVK_6,		// hex 0xE027; dec 57383
	AVK_7,		// hex 0xE028; dec 57384
	AVK_8,		// hex 0xE029; dec 57385
	AVK_9,		// hex 0xE02A; dec 57386
	AVK_STAR,	// hex 0xE02B; dec 57387
	AVK_POUND,	// hex 0xE02C; dec 57388

	AVK_POWER,	// hex 0xE02D; dec 57389
	AVK_SELECT,	// hex 0xE02E; dec 57390
	//AVK_SEND,	// hex 0xE02F; dec 57391

	AVK_UP,		// hex 0xE031; dec 57393
	AVK_DOWN,	// hex 0xE032; dec 57394
	AVK_LEFT,	// hex 0xE033; dec 57395
	AVK_RIGHT,	// hex 0xE034; dec 57396

	AVK_CLR,	// hex 0xE030; dec 57392

	AVK_SOFT1,	// hex 0xE036; dec 57398
	AVK_SOFT2,	// hex 0xE037; dec 57399

	AVK_UNK = 26,	// IOS
	AVK_VOLUME_UP = 27,	// IOS
	AVK_VOLUME_DOWN = 28,	// IOS

	// New Types Only on port
	AVK_MENUOPEN = 30,
	AVK_AUTOMAP,
	AVK_MOVELEFT,
	AVK_MOVERIGHT,
	AVK_PREVWEAPON,
	AVK_NEXTWEAPON,
	AVK_PASSTURN,
	AVK_BOTDISCARD,
	

	// New Flags Menu Only on port
	AVK_MENU_UP = 0x40,
	AVK_MENU_DOWN = 0x80,
	AVK_MENU_PAGE_UP = 0x100,
	AVK_MENU_PAGE_DOWN = 0x200,
	AVK_MENU_SELECT = 0x400,
	AVK_MENU_OPEN = 0x800,
	AVK_MENU_NUMBER = 0x2000,
	AVK_ITEMS_INFO = 0x4000,
	AVK_DRINKS = 0x8000,
	AVK_PDA = 0x10000
};

extern void controllerVibrate(int duration_ms) noexcept;

class Input
{
private:

public:

	// Constructor
	Input();
	// Destructor
	~Input();

	void init();
	void unBind(int* keyBinds, int keycode);
	void setBind(int* keyBinds, int keycode);
	void setInputBind(int scancode);
	void handleEvents() noexcept;
	void consumeEvents() noexcept;
};
#endif