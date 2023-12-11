#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Canvas.h"
#include "Input.h"
#include "SDLGL.h"
#include "GLES.h"
#include "TinyGL.h"
#include "MenuSystem.h"
#include "Player.h"
#include "Game.h"

#include <algorithm>
#include <cmath>
#include <vector>

char buttonNames[][NUM_GAMEPAD_INPUTS] = {
    "Gamepad A",
    "Gamepad B",
    "Gamepad X",
    "Gamepad Y",
    "Back",
    "Guide",
    "Start",
    "Left Stick",
    "Right Stick",
    "Left Bumper",
    "Right Bumper",
    "D-Pad Up",
    "D-Pad Down",
    "D-Pad Left",
    "D-Pad Right",
    "Axis Left X",
    "Axis Left Y",
    "Axis Right X",
    "Axis Right Y",
    "Left Trigger",
    "Right Trigger",
    "L-Stick Up",
    "L-Stick Down",
    "L-Stick Left",
    "L-Stick Right",
    "R-Stick Up",
    "R-Stick Down",
    "R-Stick Left",
    "R-Stick Right"
};

static std::vector<uint16_t>        gKeyboardKeysPressed;
static std::vector<uint16_t>        gKeyboardKeysJustPressed;
static std::vector<uint16_t>        gKeyboardKeysJustReleased;
static float                        gGamepadInputs[NUM_GAMEPAD_INPUTS];
static std::vector<GamepadInput>    gGamepadInputsPressed;
static std::vector<GamepadInput>    gGamepadInputsJustPressed;
static std::vector<GamepadInput>    gGamepadInputsJustReleased;
static std::vector<JoystickAxis>    gJoystickAxes;
static std::vector<uint32_t>        gJoystickAxesPressed;
static std::vector<uint32_t>        gJoystickAxesJustPressed;
static std::vector<uint32_t>        gJoystickAxesJustReleased;
static std::vector<uint32_t>        gJoystickButtonsPressed;
static std::vector<uint32_t>        gJoystickButtonsJustPressed;
static std::vector<uint32_t>        gJoystickButtonsJustReleased;

static SDL_GameController* gpGameController;
static SDL_Joystick* gpJoystick;         // Note: if there is a game controller then this joystick will be managed by that and not closed manually by this module!
static SDL_JoystickID               gJoystickId;
static SDL_Haptic* gJoyHaptic;

int     gDeadZone;
int     gVibrationIntensity;
float   gBegMouseX;
float   gBegMouseY;
float   gCurMouseX;
float   gCurMouseY;

keyMapping_t keyMapping[KEY_MAPPIN_MAX];
keyMapping_t keyMappingTemp[KEY_MAPPIN_MAX];
keyMapping_t keyMappingDefault[KEY_MAPPIN_MAX] = {
    {AVK_UP | AVK_MENU_UP,				{SDL_SCANCODE_UP,SDL_SCANCODE_W,-1,-1,-1,-1,-1,-1,-1,-1}},	// Move forward
    {AVK_DOWN | AVK_MENU_DOWN,			{SDL_SCANCODE_DOWN,SDL_SCANCODE_S,-1,-1,-1,-1,-1,-1,-1,-1}},	// Move backward
    {AVK_LEFT | AVK_MENU_PAGE_UP,		{SDL_SCANCODE_LEFT,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	// Turn left/page up
    {AVK_RIGHT | AVK_MENU_PAGE_DOWN,	{SDL_SCANCODE_RIGHT,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	// Turn right/page down
    {AVK_MOVELEFT,						{SDL_SCANCODE_A,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Move left
    {AVK_MOVERIGHT,						{SDL_SCANCODE_D,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Move right
    {AVK_NEXTWEAPON,					{SDL_SCANCODE_Z,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Next weapon
    {AVK_PREVWEAPON,					{SDL_SCANCODE_X,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Prev weapon
    {AVK_SELECT | AVK_MENU_SELECT,		{SDL_SCANCODE_RETURN,-1,-1,-1,-1,-1,-1,-1,-1,-1}},  // Attack/Talk/Use
    {AVK_PASSTURN,						{SDL_SCANCODE_C,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Pass Turn
    {AVK_AUTOMAP,						{SDL_SCANCODE_TAB,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	    // Automap
    {AVK_MENUOPEN | AVK_MENU_OPEN,		{SDL_SCANCODE_ESCAPE,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	// Open menu/back
    {AVK_ITEMS_INFO,                    {SDL_SCANCODE_I,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	    // Menu items and info
    {AVK_DRINKS,		                {SDL_SCANCODE_O,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	    // Menu Dirnks
    {AVK_PDA,		                    {SDL_SCANCODE_P,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	    // Menu PDA
    {AVK_BOTDISCARD,		            {SDL_SCANCODE_B,-1,-1,-1,-1,-1,-1,-1,-1,-1}}	    // Bot discart
};

//--------------------

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL button to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlButtonToInput(const uint8_t button) noexcept {
    switch (button) {
    case SDL_CONTROLLER_BUTTON_A:               return GamepadInput::BTN_A;
    case SDL_CONTROLLER_BUTTON_B:               return GamepadInput::BTN_B;
    case SDL_CONTROLLER_BUTTON_X:               return GamepadInput::BTN_X;
    case SDL_CONTROLLER_BUTTON_Y:               return GamepadInput::BTN_Y;
    case SDL_CONTROLLER_BUTTON_BACK:            return GamepadInput::BTN_BACK;
    case SDL_CONTROLLER_BUTTON_GUIDE:           return GamepadInput::BTN_GUIDE;
    case SDL_CONTROLLER_BUTTON_START:           return GamepadInput::BTN_START;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:       return GamepadInput::BTN_LEFT_STICK;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:      return GamepadInput::BTN_RIGHT_STICK;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:    return GamepadInput::BTN_LEFT_SHOULDER;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:   return GamepadInput::BTN_RIGHT_SHOULDER;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:         return GamepadInput::BTN_DPAD_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:       return GamepadInput::BTN_DPAD_DOWN;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:       return GamepadInput::BTN_DPAD_LEFT;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:      return GamepadInput::BTN_DPAD_RIGHT;

    default:
        return GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL button to a joy controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t sdlJoyButtonToInput(const uint32_t button) noexcept {
    switch (button) {
    case 0: return (int)GamepadInput::BTN_A;
    case 1: return (int)GamepadInput::BTN_B;
    case 2: return (int)GamepadInput::BTN_X;
    case 3: return (int)GamepadInput::BTN_Y;
    case 4: return (int)GamepadInput::BTN_LEFT_SHOULDER;
    case 5: return (int)GamepadInput::BTN_RIGHT_SHOULDER;
    case 6: return (int)GamepadInput::BTN_BACK;
    case 7: return (int)GamepadInput::BTN_START;
    case 8: return (int)GamepadInput::BTN_LEFT_STICK;
    case 9: return (int)GamepadInput::BTN_RIGHT_STICK;
    case 10: return (int)GamepadInput::BTN_GUIDE;
    default:
        return (int)GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlAxisToInput(const uint8_t axis) noexcept {
    switch (axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:             return GamepadInput::AXIS_LEFT_X;
    case SDL_CONTROLLER_AXIS_LEFTY:             return GamepadInput::AXIS_LEFT_Y;
    case SDL_CONTROLLER_AXIS_RIGHTX:            return GamepadInput::AXIS_RIGHT_X;
    case SDL_CONTROLLER_AXIS_RIGHTY:            return GamepadInput::AXIS_RIGHT_Y;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:       return GamepadInput::AXIS_TRIG_LEFT;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:      return GamepadInput::AXIS_TRIG_RIGHT;

    default:
        return GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlAxisToInput2(const GamepadInput axis, const float value) noexcept {
    //float deadZL = (float)deadZoneLeft / 100.f;
    //float deadZR = (float)deadZoneRight / 100.f;

    switch (axis) {
    case GamepadInput::AXIS_LEFT_X: {
        // X axis motion
        float xVal = value;
        // Below of dead zone
        if (xVal < -0) {
            return GamepadInput::BTN_LAXIS_LEFT;
        }
        // Above of dead zone
        else if (xVal > 0) {
            return GamepadInput::BTN_LAXIS_RIGHT;
        }
    }
    case GamepadInput::AXIS_LEFT_Y: {
        // Y axis motion
        float yVal = value;
        // Below of dead zone
        if (yVal < -0) {
            return GamepadInput::BTN_LAXIS_UP;
        }
        // Above of dead zone
        else if (yVal > 0) {
            return GamepadInput::BTN_LAXIS_DOWN;
        }
    }
    case GamepadInput::AXIS_RIGHT_X: {
        // X axis motion
        float xVal = value;
        // Left of dead zone
        if (xVal < -0) {
            return GamepadInput::BTN_RAXIS_LEFT;
        }
        // Right of dead zone
        else if (xVal > 0) {
            return GamepadInput::BTN_RAXIS_RIGHT;
        }
    }
    case GamepadInput::AXIS_RIGHT_Y: {
        // Y axis motion
        float yVal = value;
        // Below of dead zone
        if (yVal < -0) {
            return GamepadInput::BTN_RAXIS_UP;
        }
        // Above of dead zone
        else if (yVal > 0) {
            return GamepadInput::BTN_RAXIS_DOWN;
        }
    }
    case GamepadInput::AXIS_TRIG_LEFT:       return GamepadInput::AXIS_TRIG_LEFT;
    case GamepadInput::AXIS_TRIG_RIGHT:      return GamepadInput::AXIS_TRIG_RIGHT;
    default:
        return GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis to a joy controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlJoyAxisToInput(const uint8_t axis, const float value) noexcept {
    //float deadZL = (float)deadZoneLeft / 100.f;
    //float deadZR = (float)deadZoneRight / 100.f;
    int numAxes = SDL_JoystickNumAxes(gpJoystick);

    switch (axis) {
    case 0: {
        // X axis motion
        float xVal = value;
        // Below of dead zone
        if (xVal < -0) {
            return (numAxes <= 2) ? GamepadInput::BTN_DPAD_LEFT : GamepadInput::BTN_LAXIS_LEFT;
        }
        // Above of dead zone
        else if (xVal > 0) {
            return (numAxes <= 2) ? GamepadInput::BTN_DPAD_RIGHT : GamepadInput::BTN_LAXIS_RIGHT;
        }
    }
    case 1: {
        // Y axis motion
        float yVal = value;
        // Below of dead zone
        if (yVal < -0) {
            return (numAxes <= 2) ? GamepadInput::BTN_DPAD_UP : GamepadInput::BTN_LAXIS_UP;
        }
        // Above of dead zone
        else if (yVal > 0) {
            return (numAxes <= 2) ? GamepadInput::BTN_DPAD_DOWN : GamepadInput::BTN_LAXIS_DOWN;
        }
    }
    case 3: {
        // X axis motion
        float xVal = value;
        // Left of dead zone
        if (xVal < -0) {
            return GamepadInput::BTN_RAXIS_LEFT;
        }
        // Right of dead zone
        else if (xVal > 0) {
            return GamepadInput::BTN_RAXIS_RIGHT;
        }
    }
    case 2: {
        // Y axis motion
        float yVal = value;
        // Below of dead zone
        if (yVal < -0) {
            return GamepadInput::BTN_RAXIS_UP;
        }
        // Above of dead zone
        else if (yVal > 0) {
            return GamepadInput::BTN_RAXIS_DOWN;
        }
    }
    case 4: return GamepadInput::AXIS_TRIG_LEFT;
    case 5: return GamepadInput::AXIS_TRIG_RIGHT;
    default:
        return GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Vector utility functions
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static inline bool vectorContainsValue(const std::vector<T>& vec, const T val) noexcept {
    const auto endIter = vec.end();
    const auto iter = std::find(vec.begin(), endIter, val);
    return (iter != endIter);
}

template <class T>
static inline void removeValueFromVector(const T val, std::vector<T>& vec) noexcept {
    auto endIter = vec.end();
    auto iter = std::find(vec.begin(), endIter, val);

    while (iter != endIter) {
        iter = vec.erase(iter);
        endIter = vec.end();
        iter = std::find(iter, endIter, val);
    }
}

template <class T>
static inline void emptyAndShrinkVector(std::vector<T>& vec) noexcept {
    vec.clear();
    vec.shrink_to_fit();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Close the currently open game controller or generic joystick (if any).
// Also clears up any related inputs.
//------------------------------------------------------------------------------------------------------------------------------------------
static void closeCurrentGameController() noexcept {
    std::memset(gGamepadInputs, 0, sizeof(gGamepadInputs));
    gGamepadInputsPressed.clear();
    gGamepadInputsJustPressed.clear();
    gGamepadInputsJustReleased.clear();

    gJoystickAxes.clear();
    gJoystickAxesPressed.clear();
    gJoystickAxesJustPressed.clear();
    gJoystickAxesJustReleased.clear();

    gJoystickButtonsPressed.clear();
    gJoystickButtonsJustPressed.clear();
    gJoystickButtonsJustReleased.clear();

    // Close the current game controller, if there is any.
    // Note that closing a game controller closes the associated joystick automatically also.
    if (gpGameController) {
        SDL_GameControllerClose(gpGameController);
        gpGameController = nullptr;
        gpJoystick = nullptr;       // Managed by the game controller object, already closed!
    }

    // Close the current generic joystick, if that's all we have and not the 'game controller' interface
    if (gpJoystick) {
        SDL_JoystickClose(gpJoystick);
        gpJoystick = nullptr;

        if (gJoyHaptic) {
            SDL_HapticClose(gJoyHaptic);
            gpJoystick = nullptr;
        }
    }

    gJoystickId = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Rescans for SDL game controllers and generic joysticks to use: just uses the first available controller or joystick.
// This may choose wrong in a multi-gamepad/joystick situation but the user can always disconnect one to clarify which one is wanted.
// Most computer users would probably only want one gamepad or joystick connected at a time anyway?
//------------------------------------------------------------------------------------------------------------------------------------------
static void rescanGameControllers() noexcept {
    // If we already have a gamepad or generic joystick then just re-check that it is still connected.
    // Note that we can check if a gamepad is connected by checking if the associated joystick is connected.
    if (gpJoystick) {
        if (!SDL_JoystickGetAttached(gpJoystick)) {
            closeCurrentGameController();
        }
    }

    // See if there are any joysticks connected.
    // Note: a return of < 0 means an error, which we will ignore:
    const int numJoysticks = SDL_NumJoysticks();

    for (int joyIdx = 0; joyIdx < numJoysticks; ++joyIdx) {
        // If we find a valid game controller or generic joystick then try to open it.
        // If we succeed then our work is done!
        if (SDL_IsGameController(joyIdx)) {
            printf("IsGameController\n");
            // This is a game controller - try opening that way
            gpGameController = SDL_GameControllerOpen(joyIdx);

            if (gpGameController) {
                gpJoystick = SDL_GameControllerGetJoystick(gpGameController);
                gJoystickId = SDL_JoystickInstanceID(gpJoystick);

                // Check if joystick supports Rumble
                if (!SDL_GameControllerHasRumble(gpGameController)) {
                    printf("Warning: Game controller does not have rumble! SDL Error: %s\n", SDL_GetError());
                }
                break;
            }
        }

        // Fallback to opening the controller as a generic joystick if it's not supported through the game controller interface
        gpJoystick = SDL_JoystickOpen(joyIdx);

        if (gpJoystick) {
            gJoystickId = SDL_JoystickInstanceID(gpJoystick);

            // Check if joystick supports haptic
            if (!SDL_JoystickIsHaptic(gpJoystick)) {
                printf("Warning: Controller does not support haptics! SDL Error: %s\n", SDL_GetError());
            }
            else
            {
                // Get joystick haptic device
                gJoyHaptic = SDL_HapticOpenFromJoystick(gpJoystick);
                if (gJoyHaptic == NULL) {
                    printf("Warning: Unable to get joystick haptics! SDL Error: %s\n", SDL_GetError());
                }
                else
                {
                    // Initialize rumble
                    if (SDL_HapticRumbleInit(gJoyHaptic) < 0) {
                        printf("Warning: Unable to initialize haptic rumble! SDL Error: %s\n", SDL_GetError());
                    }
                }
            }
            break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis value to a -1 to + 1 range float.
//------------------------------------------------------------------------------------------------------------------------------------------
static float sdlAxisValueToFloat(const int16_t axis) noexcept {
    if (axis >= 0) {
        return (float)axis / 32767.0f;
    }
    else {
        return (float)axis / 32768.0f;
    }
}

float getJoystickAxisValue(const uint32_t axis) noexcept {
    for (const JoystickAxis& axisAndValue : gJoystickAxes) {
        if (axisAndValue.axis == axis)
            return axisAndValue.value;
    }

    return 0.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update the value of a joystick axis in the vector of values: removes the value if it has reached '0'
//------------------------------------------------------------------------------------------------------------------------------------------
static void updateJoystickAxisValue(const uint32_t axis, const float value) noexcept {
    // Search for the existing value of this axis: will need to remove or update it if found
    auto iter = std::find_if(gJoystickAxes.begin(), gJoystickAxes.end(), [=](const JoystickAxis& axisValue) noexcept { return (axisValue.axis == axis); });

    if (value == 0.0f) {
        if (iter != gJoystickAxes.end()) {
            gJoystickAxes.erase(iter);
        }
    }
    else {
        if (iter != gJoystickAxes.end()) {
            iter->value = value;
        }
        else {
            gJoystickAxes.emplace_back(JoystickAxis{ axis, value });
        }
    }
}

bool isKeyboardKeyPressed(const uint16_t key) noexcept {
    return vectorContainsValue(gKeyboardKeysPressed, key);
}

bool isKeyboardKeyJustPressed(const uint16_t key) noexcept {
    return vectorContainsValue(gKeyboardKeysJustPressed, key);
}

bool isKeyboardKeyReleased(const uint16_t key) noexcept {
    return (!isKeyboardKeyPressed(key));
}

bool isKeyboardKeyJustReleased(const uint16_t key) noexcept {
    return vectorContainsValue(gKeyboardKeysJustReleased, key);
}

bool isGamepadInputPressed(const GamepadInput input) noexcept {
    return vectorContainsValue(gGamepadInputsPressed, input);
}

bool isGamepadInputJustPressed(const GamepadInput input) noexcept {
    return vectorContainsValue(gGamepadInputsJustPressed, input);
}

bool isGamepadInputJustReleased(const GamepadInput input) noexcept {
    return vectorContainsValue(gGamepadInputsJustReleased, input);
}

bool isJoystickAxisPressed(const uint32_t axis) noexcept {
    return vectorContainsValue(gJoystickAxesPressed, axis);
}

bool isJoystickAxisJustPressed(const uint32_t axis) noexcept {
    return vectorContainsValue(gJoystickAxesJustPressed, axis);
}

bool isJoystickAxisJustReleased(const uint32_t axis) noexcept {
    return vectorContainsValue(gJoystickAxesJustReleased, axis);
}

bool isJoystickButtonPressed(const uint32_t button) noexcept {
    return vectorContainsValue(gJoystickButtonsPressed, button);
}

bool isJoystickButtonJustPressed(const uint32_t button) noexcept {
    return vectorContainsValue(gJoystickButtonsJustPressed, button);
}

bool isJoystickButtonJustReleased(const uint32_t button) noexcept {
    return vectorContainsValue(gJoystickButtonsJustReleased, button);
}

bool isJoystickAxisButtonPressed(uint32_t button) noexcept { // [GEC]
    for (uint32_t i = 0; i <= 5; i++) {
        if (isJoystickAxisPressed(i)) {
            if (sdlJoyAxisToInput(i, getJoystickAxisValue(i)) == (GamepadInput)button) {
                removeValueFromVector(i, gJoystickAxesPressed);
                return true;
            }
        }
    }
    return false;
}

bool isJoystickAxisButtonJustPressed(uint32_t button) noexcept { // [GEC]
    for (uint32_t i = 0; i <= 5; i++) {
        if (isJoystickAxisJustPressed(i)) {
            if (sdlJoyAxisToInput(i, getJoystickAxisValue(i)) == (GamepadInput)button) {
                return true;
            }
        }
    }
    return false;
}

bool isJoystickAxisButtonJustReleased() noexcept { // [GEC]
    for (uint32_t i = 0; i <= 5; i++) {
        if (isJoystickAxisJustReleased(i)) {
            removeValueFromVector(i, gJoystickAxesJustReleased);
            return true;
        }
    }
    return false;
}


bool isGamepadAxisInputPressed(GamepadInput inputIn) noexcept { // [GEC]
    for (int i = (int)GamepadInput::AXIS_LEFT_X; i <= (int)GamepadInput::AXIS_TRIG_RIGHT; i++) {
        const GamepadInput input = (GamepadInput)i;
        const uint8_t inputIdx = (uint8_t)input;
        if (isGamepadInputPressed(input)) {
            if (sdlAxisToInput2(input, gGamepadInputs[inputIdx]) == inputIn) {
                removeValueFromVector(input, gGamepadInputsPressed);
                return true;
            }
        }
    }
    return false;
}

bool isGamepadAxisInputJustPressed(GamepadInput inputIn) noexcept { // [GEC]
    for (int i = (int)GamepadInput::AXIS_LEFT_X; i <= (int)GamepadInput::AXIS_TRIG_RIGHT; i++) {
        const GamepadInput input = (GamepadInput)i;
        const uint8_t inputIdx = (uint8_t)input;
        if (isGamepadInputJustPressed(input)) {
            if (sdlAxisToInput2(input, gGamepadInputs[inputIdx]) == inputIn) {
                return true;
            }
        }
    }
    return false;
}

bool isGamepadAxisInputJustReleased() noexcept { // [GEC]
    for (int i = (int)GamepadInput::AXIS_LEFT_X; i <= (int)GamepadInput::AXIS_TRIG_RIGHT; i++) {
        const GamepadInput input = (GamepadInput)i;
        if (isGamepadInputJustReleased(input)) {
            removeValueFromVector(input, gGamepadInputsJustReleased);
            return true;
        }
    }
    return false;
}
//----------------------

void controllerVibrate(int duration_ms) noexcept {
    float intensity = (float)gVibrationIntensity / 100.f;
    // Use game controller
    if (gpGameController) {
        float frequency = (float)0xFFFF * intensity;
        SDL_GameControllerRumble(gpGameController, 0, 0, 1); // Stop
        SDL_GameControllerRumble(gpGameController, (int)frequency, (int)frequency, duration_ms);
    }
    //Use haptics
    else if (gJoyHaptic) {
        SDL_HapticRumbleStop(gJoyHaptic);
        SDL_HapticRumblePlay(gJoyHaptic, 1.0 * intensity, duration_ms);
    }
}

Input::Input() {
}

Input::~Input() {
    emptyAndShrinkVector(gJoystickButtonsJustReleased);
    emptyAndShrinkVector(gJoystickButtonsJustPressed);
    emptyAndShrinkVector(gJoystickButtonsPressed);

    emptyAndShrinkVector(gJoystickAxesJustReleased);
    emptyAndShrinkVector(gJoystickAxesJustPressed);
    emptyAndShrinkVector(gJoystickAxesPressed);
    emptyAndShrinkVector(gJoystickAxes);

    emptyAndShrinkVector(gGamepadInputsJustReleased);
    emptyAndShrinkVector(gGamepadInputsJustPressed);
    emptyAndShrinkVector(gGamepadInputsPressed);

    emptyAndShrinkVector(gKeyboardKeysJustReleased);
    emptyAndShrinkVector(gKeyboardKeysJustPressed);
    emptyAndShrinkVector(gKeyboardKeysPressed);
}

// Port: set default Binds
void Input::init() {
    std::memcpy(keyMapping, keyMappingDefault, sizeof(keyMapping));
    std::memcpy(keyMappingTemp, keyMappingDefault, sizeof(keyMapping));

    gDeadZone = 50;
    gVibrationIntensity = 80;
    gBegMouseX = -1;
    gBegMouseY = -1;
    gCurMouseX = -1;
    gCurMouseY = -1;

    gKeyboardKeysJustPressed.reserve(32);
    gKeyboardKeysJustReleased.reserve(32);

    gGamepadInputsPressed.reserve(NUM_GAMEPAD_INPUTS);
    gGamepadInputsJustPressed.reserve(NUM_GAMEPAD_INPUTS);
    gGamepadInputsJustReleased.reserve(NUM_GAMEPAD_INPUTS);

    gJoystickAxes.reserve(16);
    gJoystickAxesPressed.reserve(16);
    gJoystickAxesJustPressed.reserve(16);
    gJoystickAxesJustReleased.reserve(16);

    gJoystickButtonsPressed.reserve(32);
    gJoystickButtonsJustPressed.reserve(32);
    gJoystickButtonsJustReleased.reserve(32);

    rescanGameControllers();
}

void Input::unBind(int* keyBinds, int index)
{
    int temp[KEYBINDS_MAX], next, i;

    next = 0;
    keyBinds[index] = -1;

    // Reorder the list
    for (i = 0; i < KEYBINDS_MAX; i++) {
        temp[i] = -1;
        if (keyBinds[i] == -1) {
            continue;
        }
        temp[next++] = keyBinds[i];
    }

    std::memcpy(keyBinds, temp, sizeof(temp));
}

void Input::setBind(int* keyBinds, int keycode) {
    int i;

    // Examina si existe anteriormente, si es así, se desvinculará de la lista
    // Examines whether it exists previously, if so, it will be unbind from the list
    for (i = 0; i < KEYBINDS_MAX; i++) {
        if (keyBinds[i] == keycode) {
            this->unBind(keyBinds, i);
            return;
        }
    }

    // Se guarda el key code en la lista
    // The key code is saved in the list
    for (i = 0; i < KEYBINDS_MAX; i++) {
        if (keyBinds[i] == -1) {
            keyBinds[i] = keycode;
            return;
        }
    }
}

void Input::setInputBind(int scancode) {
    Applet* app = CAppContainer::getInstance()->app;
    int keyMapId = app->menuSystem->items[app->menuSystem->selectedIndex].param;
    this->setBind(keyMappingTemp[keyMapId].keyBinds, scancode);
}

void Input::handleEvents() noexcept {
    Applet* app = CAppContainer::getInstance()->app;
	SDL_Event sdlEvent;
    SDLGL* sdlGL = CAppContainer::getInstance()->sdlGL;
    Canvas* canvas = CAppContainer::getInstance()->app->canvas;

    int winVidWidth = sdlGL->winVidWidth;
    int winVidHeight = sdlGL->winVidHeight;
    Uint8 state;
    int mX, mY;           /* mouse location*/
    int i, j;

    while (SDL_PollEvent(&sdlEvent) != 0) {
        switch (sdlEvent.type) {
            case SDL_QUIT: {
                printf("handleEvents::SDL_QUIT\n");
                CAppContainer::getInstance()->app->shutdown();
            }   break;

            /*case SDL_WINDOWEVENT: {
                if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED) {
                    printf("SDL_WINDOWEVENT_RESIZED\n");
                    SDL_GetWindowSize(sdlGL->window, &winVidWidth, &winVidHeight);
                    sdlGL->updateWinVid(winVidWidth, winVidHeight);
                }
            }   break;*/

            case SDL_KEYDOWN: {
                const uint16_t scancode = (uint16_t)sdlEvent.key.keysym.scancode;
                if ((!sdlEvent.key.repeat) && (scancode < SDL_NUM_SCANCODES)) {
                    //printf("%d\n", scancode);

                    if (app->menuSystem->setBinding) {
                        this->setInputBind(scancode);
                        app->menuSystem->setBinding = !app->menuSystem->setBinding;
                        app->menuSystem->changeValues = !app->menuSystem->changeValues;
                        break;
                    }

                    for (int i = SDL_SCANCODE_1; i <= SDL_SCANCODE_0; ++i) {
                        if (scancode == i) {
                            int num = ((i - SDL_SCANCODE_0) + 10) % 10;
                            if (canvas->numEvents != 4) {
                                canvas->events[canvas->numEvents++] = (AVK_0 + num) | AVK_MENU_NUMBER;
                            }
                            break;
                        }
                    }

                    for (int i = SDL_SCANCODE_KP_1; i <= SDL_SCANCODE_KP_0; ++i) {
                        if (scancode == i) {
                            int num = ((i - SDL_SCANCODE_KP_0) + 10) % 10;
                            if (canvas->numEvents != 4) {
                                canvas->events[canvas->numEvents++] = (AVK_0 + num) | AVK_MENU_NUMBER;
                            }
                            break;
                        }
                    }

                    if (scancode == SDL_SCANCODE_F1) {
                        Canvas* canvas = CAppContainer::getInstance()->app->canvas;
                        TinyGL* tinyGL = CAppContainer::getInstance()->app->tinyGL;
                        _glesObj->isInit = !_glesObj->isInit;

                        if (canvas->state == Canvas::ST_CAMERA) {
                            tinyGL->setViewport(canvas->cinRect[0], canvas->cinRect[1], canvas->cinRect[2], canvas->cinRect[3]);
                        }
                        else {
                            tinyGL->resetViewPort();
                        }
                        CAppContainer::getInstance()->app->game->saveConfig();
                        break;
                    }

                    removeValueFromVector(scancode, gKeyboardKeysJustReleased);
                    gKeyboardKeysPressed.push_back(scancode);
                    gKeyboardKeysJustPressed.push_back(scancode);
                }
            }   break;

            case SDL_KEYUP: {
                const uint16_t scancode = (uint16_t)sdlEvent.key.keysym.scancode;

                if ((!sdlEvent.key.repeat) && (scancode < SDL_NUM_SCANCODES)) {
                    removeValueFromVector(scancode, gKeyboardKeysPressed);
                    removeValueFromVector(scancode, gKeyboardKeysJustPressed);
                    gKeyboardKeysJustReleased.push_back(scancode);
                }
            }   break;

            

            case SDL_MOUSEBUTTONDOWN: {
                state = SDL_GetMouseState(&mX, &mY);
                if (state & SDL_BUTTON_LMASK) {
                    gBegMouseX = (float)((mX) * (1.0f / winVidWidth));
                    gBegMouseY = (float)((mY) * (1.0f / winVidHeight));
                    CAppContainer::getInstance()->userPressed(gBegMouseX, gBegMouseY);
                }
            }   break;

            case SDL_MOUSEBUTTONUP: {
                CAppContainer::getInstance()->userReleased(gCurMouseX, gCurMouseY);
                CAppContainer::getInstance()->unHighlightButtons();
                CAppContainer::getInstance()->TestCheatEntry(gCurMouseX, gCurMouseY);
            }   break;

            case SDL_MOUSEMOTION: {
                state = SDL_GetMouseState(&mX, &mY);
                gCurMouseX = (float)((mX) * (1.0f / winVidWidth));
                gCurMouseY = (float)((mY) * (1.0f / winVidHeight));
                if (state & SDL_BUTTON_LMASK) {
                    CAppContainer::getInstance()->userMoved(gCurMouseX, gCurMouseY);
                }
                CAppContainer::getInstance()->UpdateAccelerometer(gCurMouseX, gCurMouseY, 0.0f, true);
            }   break;

            case SDL_CONTROLLERAXISMOTION: {
                if (gpGameController) {
                    if (sdlEvent.cbutton.which == gJoystickId) {
                        GamepadInput input = sdlAxisToInput(sdlEvent.caxis.axis);

                        if (input != GamepadInput::INVALID) {

                            const float pressedThreshold = (float)gDeadZone / 100.f;//Config::gAnalogToDigitalThreshold;
                            const uint8_t inputIdx = (uint8_t)input;

                            // See if there is a change in the 'pressed' status
                            const bool bPrevPressed = (std::abs(gGamepadInputs[inputIdx]) >= pressedThreshold);
                            const float inputF = sdlAxisValueToFloat(sdlEvent.caxis.value);
                            const float inputFAbs = std::abs(inputF);
                            const bool bNowPressed = (inputFAbs >= pressedThreshold);

                            // Update input value
                            gGamepadInputs[inputIdx] = inputF;

                            // Generate events for the analog input
                            if (bPrevPressed != bNowPressed) {
                                if (bNowPressed) {
                                    if (app->menuSystem->setBinding) {
                                        GamepadInput inputBind = sdlAxisToInput2(input, inputF); // [GEC]
                                        this->setInputBind((int)inputBind | IS_CONTROLLER_BUTTON);
                                        app->menuSystem->setBinding = !app->menuSystem->setBinding;
                                        app->menuSystem->changeValues = !app->menuSystem->changeValues;
                                        break;
                                    }
                                    removeValueFromVector(input, gGamepadInputsJustReleased);
                                    gGamepadInputsPressed.push_back(input);
                                    gGamepadInputsJustPressed.push_back(input);
                                }
                                else {
                                    removeValueFromVector(input, gGamepadInputsPressed);
                                    removeValueFromVector(input, gGamepadInputsJustPressed);
                                    gGamepadInputsJustReleased.push_back(input);
                                }
                            }
                        }
                    }
                }
            }   break;

            case SDL_JOYAXISMOTION: {
                if (!gpGameController) {
                    if (sdlEvent.jaxis.which == gJoystickId) {
                        // See if there is a change in the 'pressed' status
                        const float pressedThreshold = (float)gDeadZone / 100.f;//Config::gAnalogToDigitalThreshold;
                        const uint32_t axis = sdlEvent.jaxis.axis;

                        const bool bPrevPressed = (std::abs(getJoystickAxisValue(axis)) >= pressedThreshold);
                        const float inputF = sdlAxisValueToFloat(sdlEvent.jaxis.value);
                        const float inputFAbs = std::abs(inputF);
                        const bool bNowPressed = (inputFAbs >= pressedThreshold);

                        // Update input value
                        updateJoystickAxisValue(axis, inputF);

                        // Generate events for the analog input
                        if (bPrevPressed != bNowPressed) {
                            if (bNowPressed) {
                                if (app->menuSystem->setBinding) {
                                    const GamepadInput inputBind = sdlJoyAxisToInput(axis, inputF); // [GEC]
                                    this->setInputBind((int)inputBind | IS_CONTROLLER_BUTTON);
                                    app->menuSystem->setBinding = !app->menuSystem->setBinding;
                                    app->menuSystem->changeValues = !app->menuSystem->changeValues;
                                    break;
                                }
                                removeValueFromVector(axis, gJoystickAxesJustReleased);
                                gJoystickAxesPressed.push_back(axis);
                                gJoystickAxesJustPressed.push_back(axis);
                            }
                            else {
                                removeValueFromVector(axis, gJoystickAxesPressed);
                                removeValueFromVector(axis, gJoystickAxesJustPressed);
                                gJoystickAxesJustReleased.push_back(axis);
                            }
                        }
                    }
                }
            }   break;

            case SDL_CONTROLLERBUTTONDOWN: {
                if (gpGameController) {
                    if (sdlEvent.cbutton.which == gJoystickId) {
                        const GamepadInput input = sdlButtonToInput(sdlEvent.cbutton.button);
                        if (input != GamepadInput::INVALID) {
                            if (app->menuSystem->setBinding) {
                                this->setInputBind((int)input | IS_CONTROLLER_BUTTON);
                                app->menuSystem->setBinding = !app->menuSystem->setBinding;
                                app->menuSystem->changeValues = !app->menuSystem->changeValues;
                                break;
                            }
                            removeValueFromVector(input, gGamepadInputsJustReleased);
                            gGamepadInputsPressed.push_back(input);
                            gGamepadInputsJustPressed.push_back(input);
                            gGamepadInputs[(uint8_t)input] = 1.0f;
                        }
                    }
                }
            }   break;

            case SDL_JOYBUTTONDOWN: {
                if (!gpGameController) {
                    if (sdlEvent.jbutton.which == gJoystickId) {
                        const uint32_t button = (uint32_t)sdlJoyButtonToInput(sdlEvent.jbutton.button);
                        if (button != (int)GamepadInput::INVALID) {
                            if (app->menuSystem->setBinding) {
                                this->setInputBind((int)button | IS_CONTROLLER_BUTTON);
                                app->menuSystem->setBinding = !app->menuSystem->setBinding;
                                app->menuSystem->changeValues = !app->menuSystem->changeValues;
                                break;
                            }
                            removeValueFromVector(button, gJoystickButtonsJustReleased);
                            gJoystickButtonsPressed.push_back(button);
                            gJoystickButtonsJustPressed.push_back(button);
                        }
                    }
                }
            }   break;

            case SDL_CONTROLLERBUTTONUP: {
                if (gpGameController) {
                    if (sdlEvent.cbutton.which == gJoystickId) {
                        const GamepadInput input = sdlButtonToInput(sdlEvent.cbutton.button);
                        if (input != GamepadInput::INVALID) {
                            gGamepadInputsJustReleased.push_back(input);
                            removeValueFromVector(input, gGamepadInputsPressed);
                            removeValueFromVector(input, gGamepadInputsJustPressed);
                            gGamepadInputs[(uint8_t)input] = 0.0f;
                        }
                    }
                }
            }   break;

            case SDL_JOYBUTTONUP: {
                if (!gpGameController) {
                    if (sdlEvent.jbutton.which == gJoystickId) {
                        const uint32_t button = sdlEvent.jbutton.button;
                        gJoystickButtonsJustReleased.push_back(button);
                        removeValueFromVector(button, gJoystickButtonsPressed);
                        removeValueFromVector(button, gJoystickButtonsJustPressed);
                    }
                }
            }   break;


            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMAPPED:
                rescanGameControllers();
                break;
            
        }
    }

    canvas->keyDownCausedMove = false;
    if (canvas->state == Canvas::ST_PLAYING && !app->player->inTargetPractice) {
        for (i = 0; i < 2; ++i) {
            for (j = 0; j < KEYBINDS_MAX; j++) {
                // Controller
                if ((keyMapping[i].keyBinds[j] & IS_CONTROLLER_BUTTON)) {
                    const GamepadInput input = (GamepadInput)(keyMapping[i].keyBinds[j] & ~(IS_CONTROLLER_BUTTON | IS_MOUSE_BUTTON));
                    // Joystick
                    if (isJoystickButtonJustReleased((uint32_t)input)) {
                        removeValueFromVector((uint32_t)input, gJoystickButtonsJustReleased);
                        canvas->clearEvents(0);
                        break;
                    }
                    else if (isJoystickButtonJustPressed((uint32_t)input)) {
                        canvas->keyDownCausedMove = true;
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                    // Joystick Axis
                    else if (isJoystickAxisButtonJustReleased()) {
                        canvas->clearEvents(0);
                    }
                    else if(isJoystickAxisButtonJustPressed((uint32_t)input)) {
                        canvas->keyDownCausedMove = true;
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                    // Gamepad
                    else if (isGamepadInputJustReleased(input)) {
                        removeValueFromVector(input, gGamepadInputsJustReleased);
                        canvas->clearEvents(0);
                        break;
                    }
                    else if (isGamepadInputJustPressed(input)) {
                        canvas->keyDownCausedMove = true;
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                    // Gamepad Axis
                    else if (isGamepadAxisInputJustReleased()) {
                        canvas->clearEvents(0);
                    }
                    else if (isGamepadAxisInputJustPressed(input)) {
                        canvas->keyDownCausedMove = true;
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                }
                // Keyboard
                else if (keyMapping[i].keyBinds[j]) {
                    const uint16_t input = (uint16_t)(keyMapping[i].keyBinds[j] & ~(IS_CONTROLLER_BUTTON | IS_MOUSE_BUTTON));

                    if (isKeyboardKeyJustReleased(input)) {
                        removeValueFromVector(input, gKeyboardKeysJustReleased);
                        canvas->clearEvents(0);
                        break;
                    }
                    else if (isKeyboardKeyJustPressed(input)) {
                        canvas->keyDownCausedMove = true;
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                }
            }
        }
    }

    if (!canvas->keyDownCausedMove) {
        for (i = 0; i < (sizeof(keyMapping) / sizeof(keyMapping_t)); ++i) {
            for (j = 0; j < KEYBINDS_MAX; j++) {
                // Controller
                if ((keyMapping[i].keyBinds[j] & IS_CONTROLLER_BUTTON)) {
                    const GamepadInput input = (GamepadInput)(keyMapping[i].keyBinds[j] & ~(IS_CONTROLLER_BUTTON | IS_MOUSE_BUTTON));
                    // Joystick
                    if (isJoystickButtonPressed((uint32_t)input)) {
                        removeValueFromVector((uint32_t)input, gJoystickButtonsPressed);
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                    else if (isJoystickAxisButtonPressed((uint32_t)input)) {
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                    // Gamepad
                    else if (isGamepadInputPressed(input)) {
                        removeValueFromVector(input, gGamepadInputsPressed);
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                    else if (isGamepadAxisInputPressed(input)) {
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                }
                // Keyboard
                else if ((keyMapping[i].keyBinds[j])) {
                    const uint16_t input = (uint16_t)(keyMapping[i].keyBinds[j] & ~(IS_CONTROLLER_BUTTON | IS_MOUSE_BUTTON));
                    if (isKeyboardKeyPressed(input)) {
                        removeValueFromVector(input, gKeyboardKeysPressed);
                        canvas->addEvents(keyMapping[i].avk_action);
                        break;
                    }
                }       
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Discards input events and movements.
// Should be called whenever inputs have been processed for a frame.
//------------------------------------------------------------------------------------------------------------------------------------------
void Input::consumeEvents() noexcept {
    const Canvas* canvas = CAppContainer::getInstance()->app->canvas;
    // Clear all events
    if (!canvas->keyDownCausedMove) {
        gKeyboardKeysJustPressed.clear();
        gKeyboardKeysJustReleased.clear();

        gGamepadInputsJustPressed.clear();
        gGamepadInputsJustReleased.clear();

        gJoystickAxesJustPressed.clear();
        gJoystickAxesJustReleased.clear();
        gJoystickButtonsJustPressed.clear();
        gJoystickButtonsJustReleased.clear();
    }

    // Clear other events
    //gbWindowFocusJustLost = false;
}