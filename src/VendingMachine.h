#ifndef __VENDINGMACHINE_H__
#define __VENDINGMACHINE_H__

class ScriptThread;
class Image;
class fmButtonContainer;
class Graphics;

class VendingMachine
{
private:

public:
	int touchMe;
	ScriptThread* callingThread;
	int16_t* energyDrinkData;
	Image* imgVendingBG;
	Image* imgVendingGame;
	Image* imgHelpScreenAssets;
	Image* imgVending_BG;
	Image* imgVending_button_small;
	int mainTerminalCursor;
	int gameCursor;
	int solution[4];
	int sliderPositions[4];
	int playersGuess[4];
	int minimums[4];
	int maximums[4];
	int chevronAnimationOffset[4];
	int correctSum;
	int triesLeft;
	int currentMapNumber;
	int currentMachineNumber;
	bool machineHasBeenHacked;
	bool machineJustHacked;
	int currentItemQuantity;
	int currentItemPrice;
	int* stateVars;
	int widthOfContainingBox;
	int iqBonusAwarded;
	bool gamePlayedFromMainMenu;
	fmButtonContainer* m_vendingButtons;
	Image* imgSubmitButton;
	Image* imgVending_submit_pressed;
	Image* imgVending_arrow_up;
	Image* imgVending_arrow_down;
	Image* imgVending_arrow_up_pressed;
	Image* imgVending_arrow_down_pressed;
	bool touched;
	int currentPress_x;
	int currentPress_y;
	int gameCursor2; //[GEC]

	// Constructor
	VendingMachine();
	// Destructor
	~VendingMachine();

	bool startup();

	void playFromMainMenu();
	void initGame(ScriptThread* callingThread, int a, int a2);
	void returnFromBuying();
	bool machineCanBeHacked();
	void randomizeGame();
	void handleInput(int action);
	void handleInputForBasicVendingMachine(int action);
	void handleInputForHelpScreen(int action);
	void handleInputForGame(int action);
	void updateHighLowState();
	void updateGame(Graphics* graphics);
	void drawGameResults(Graphics* graphics);
	void drawMainScreen(Graphics* graphics);
	void drawVendingMachineBackground(Graphics* graphics, bool b);
	void drawHelpScreen(Graphics* graphics);
	void drawGameScreen(Graphics* graphics);
	void drawGameTopBar(Graphics* graphics, Text* text);
	void drawGameMiddleBar(Graphics* graphics, Text* text);
	bool drinkInThisVendingMachine(int n);
	short getDrinkPrice(int n);
	bool buyDrink(int n);
	int getSnackPrice();
	int numbersCorrect();
	bool playerHasWon();
	void forceWin();
	void endGame(int n);
	void touchStart(int pressX, int pressY);
	void touchMove(int pressX, int pressY);
	void touchEnd(int pressX, int pressY);
	void handleTouchForHelpScreen(int pressX, int pressY);
	void handleTouchForGame(int pressX, int pressY);
	void handleTouchForBasicVendingMachine(int pressX, int pressY);
};

#endif