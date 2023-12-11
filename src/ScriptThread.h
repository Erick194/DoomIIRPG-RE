#ifndef __SCRIPTTHREAD_H__
#define __SCRIPTTHREAD_H__

class Text;
class Applet;
class OutputStream;
class InputStream;
class Entity;

class ScriptThread
{
private:

public:
	int unpauseTime;
	int state;
	bool inuse;
	int IP;
	int FP;
	int flags;
	int type;
	bool throwAwayLoot;
	int scriptStack[16];
	int stackPtr;
	Text* debugString;
	Applet* app;

	// Constructor
	ScriptThread();
	// Destructor
	~ScriptThread();

	void saveState(OutputStream* OS);
	void loadState(InputStream* IS);
	uint32_t executeTile(int x, int y, int flags, bool b);
	int queueTile(int x, int y, int flags);
	int queueTile(int x, int y, int flags, bool b);
	int evWait(int time);
	bool evReturn();
	void alloc(int n, int type, bool b);
	void alloc(int ip);
	int peekNextCmd();
	void setupCamera(int n);
	uint32_t run();
	void init();
	void reset();
	int attemptResume(int n);
	int getIndex();
	int pop();
	void push(bool b);
	void push(int n);
	short getUByteArg();
	uint8_t getByteArg();
	int getUShortArg();
	short getShortArg();
	int getIntArg();
	void composeLootDialog();
	void setAIGoal(Entity* entity, int n, int goalParam);
	void corpsifyMonster(int x, int y, Entity* entity, bool b);
};

#endif