#ifndef CR_QT_MAIN_WINDOW
#define CR_QT_MAIN_WINDOW

#include <QtWidgets/QMainWindow>
#include "ui_CrQtMainWindow.h"

extern bool g_appWasClosed;

class CrQtMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CrQtMainWindow(QWidget *parent = 0);
	~CrQtMainWindow();

	void closeEvent(QCloseEvent* /*event*/) override
	{
		g_appWasClosed = true;
	}

private:
	Ui::CrQtMainWindow ui;
};

#endif // CR_QT_MAIN_WINDOW
