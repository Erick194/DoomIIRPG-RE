#ifndef __SENTRYBOTGAME_H__
#define __SENTRYBOTGAME_H__

class ScriptThread;
class Image;
class fmButtonContainer;
class Graphics;

class SentryBotGame
{
private:
	static bool wasTouched;

public:
	int touchMe;
	ScriptThread* callingThread;
	Image* imgHelpScreenAssets;
	Image* imgGameAssets;
	Image* imgMatrixSkip_BG;
	Image* imgButton;
	Image* imgButton2;
	Image* imgButton3;
	uint32_t skipCode;
	int gameBoard[16];
	int solution[4];
	int usersGuess[4];
	int player_cursor;
	int answer_cursor;
	int bot_selection_cursor;
	int* stateVars;
	short botType;
	int timeSinceLastCursorMove;
	bool failedEarly;
	bool gamePlayedFromMainMenu;
	fmButtonContainer* m_sentryBotButtons;
	Image* imgSubmit;
	Image* imgUnk1;
	Image* imgDelete;
	bool touched;
	int currentPress_x;
	int currentPress_y;

	// Constructor
	SentryBotGame();
	// Destructor
	~SentryBotGame();

	void playFromMainMenu();
	void setupGlobalData();
	void initGame(ScriptThread* scriptThread, short botType);
	void handleInput(int action);
	void updateGame(Graphics* graphics);
	void drawFailureScreen(Graphics* graphics);
	void drawSuccessScreen(Graphics* graphics);
	void drawHelpScreen(Graphics* graphics);
	void drawGameScreen(Graphics* graphics);

	void drawPlayersGuess(int n, int n2, bool b, Text* text, Graphics* graphics);
	void drawCursor(int n, int n2, bool b, Graphics* graphics);
	bool playerHasWon();
	bool playerCouldStillWin();
	void forceWin();
	void awardSentryBot(int n);
	void endGame(int n);
	void touchStart(int pressX, int pressY);
	void touchMove(int pressX, int pressY);
	void touchEnd(int pressX, int pressY);
	void handleTouchForHelpScreen(int pressX, int pressY);
	void handleTouchForGame(int pressX, int pressY);
};

#endif