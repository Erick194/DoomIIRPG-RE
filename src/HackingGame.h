#ifndef __HACKINGGAME_H__
#define __HACKINGGAME_H__

class ScriptThread;
class Image;
class fmButtonContainer;

class HackingGame
{
private:

public:
	static int touchedColumn;

	int touchMe;
	ScriptThread* callingThread;
	Image* imgEnergyCore;
	Image* imgGameColors;
	Image* imgHelpScreenAssets;
	int columnCount;
	short selectedRow;
	short selectedColumn;
	bool selected;
	short turnsLeft;
	int confirmationCursor;
	int columnBlockCameFrom;
	bool gamePlayedFromMainMenu;
	short gameBoard[5][6];
	uint8_t mostRecentSrc[6];
	uint8_t mostRecentDest[6];
	int CORE_WIDTH;
	int CORE_HEIGHT;
	int* stateVars;
	bool touched;
	int currentPress_x;
	int currentPress_y;
	int oldPress_x;
	int oldPress_y;
	fmButtonContainer* m_hackingButtons;

	// Constructor
	HackingGame();
	// Destructor
	~HackingGame();

	void playFromMainMenu();
	void setupGlobalData();
	void initGame(ScriptThread* scriptThread, int i);
	void initGame(ScriptThread* scriptThread, int i, int i2);
	void fillGameBoardRandomly(short array[5][6], int n, int n2, int n3);
	void fillGameBoardRandomly(short array[5][6], int n, int n2, int n3, int n4);
	void handleInput(int action);
	void attemptToMove(short n);
	void updateGame(Graphics* graphics);
	void drawHelpScreen(Graphics* graphics);
	void drawGameScreen(Graphics* graphics);
	void drawGoalTextAndBars(Graphics* graphics, Text* text);
	void drawGamePieces(Graphics* graphics, int x, int y);
	bool gameIsSolved(short array[5][6]);
	void drawPiece(int i, int x, int y, Graphics* graphics);
	void forceWin();
	void endGame(int n);
	void touchStart(int x, int y);
	void touchMove(int x, int y);
	void touchEnd(int x, int y);
};

#endif