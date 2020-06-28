#pragma once

#include "Qt/CrQtMainWindow.h"
#include <QtWidgets/QApplication>
#include <QtCore/QAbstractNativeEventFilter>
#include "CrInputManager.h"

class NativeEventFilter : public QAbstractNativeEventFilter
{
	public:
	NativeEventFilter() : QAbstractNativeEventFilter() {};
	~NativeEventFilter() {};

	virtual bool nativeEventFilter(const QByteArray& /*eventType*/, void* message, long* /*result*/) override
	{
		CrInput.HandleMessage(message);
		return false;
	}
};


class CrEventFilter : public QObject
{
	bool eventFilter(QObject* /*receiver*/, QEvent* /*event*/) override
	{
		return false;
	}
};

class CrGUI
{
	public:
	CrGUI(int& argc, char** argv);
	~CrGUI();

	HWND GetMainWindowHandle();

	private:
	QApplication* m_app;
	CrQtMainWindow* m_mainWindow;
};

inline CrGUI::CrGUI(int& argc, char** argv)
{
	// Note: argc and argv might be changed as Qt removes command line arguments that it recognizes.
	// http://doc.qt.io/qt-5/qapplication.html#QApplication
	m_app = new QApplication(argc, argv);

	m_mainWindow = new CrQtMainWindow();

	CrEventFilter* crEventFilter = new CrEventFilter();
	m_app->installEventFilter(crEventFilter);

	NativeEventFilter* winEventFilter = new NativeEventFilter();
	m_app->installNativeEventFilter(winEventFilter);	

	m_mainWindow->show();
}

inline CrGUI::~CrGUI()
{
	delete m_app;
	// Note: DO NOT try to delete Qt objects manually, therefore also DO NOT allocate them on the stack. Qt manages its objects and tries to delete them itself, 
	// at which point the program may crash. MSVC doesn't crash (on a debug build) but Clang does.
}

// TODO Windows-specific
inline HWND CrGUI::GetMainWindowHandle()
{
	return (HWND)m_mainWindow->centralWidget()->winId();
}