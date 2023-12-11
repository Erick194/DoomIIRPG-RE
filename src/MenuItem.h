#ifndef __MENUITEM_H__
#define __MENUITEM_H__

class Text;

class MenuItem
{
private:

public:
	int touchMe;
	int textField;
	int textField2;
	int helpField;
	int flags;
	int param;
	int action;

	// Constructor
	MenuItem();
	// Destructor
	~MenuItem();

	void Set(int textField, int textField2, int flags);
	void Set(int textField, int textField2, int flags, int action, int param, int helpField);
	void WrapHelpText(Text* text);
};

#endif