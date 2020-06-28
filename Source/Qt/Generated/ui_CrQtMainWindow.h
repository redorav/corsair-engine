/********************************************************************************
** Form generated from reading UI file 'CrQtMainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CRQTMAINWINDOW_H
#define UI_CRQTMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CrQtMainWindow
{
public:
    QAction *actionNew_Scene;
    QAction *actionOpen_Scene;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *CrQtMainWindow)
    {
        if (CrQtMainWindow->objectName().isEmpty())
            CrQtMainWindow->setObjectName(QStringLiteral("CrQtMainWindow"));
        CrQtMainWindow->resize(1280, 720);
        actionNew_Scene = new QAction(CrQtMainWindow);
        actionNew_Scene->setObjectName(QStringLiteral("actionNew_Scene"));
        actionOpen_Scene = new QAction(CrQtMainWindow);
        actionOpen_Scene->setObjectName(QStringLiteral("actionOpen_Scene"));
        centralWidget = new QWidget(CrQtMainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        CrQtMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(CrQtMainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1280, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        CrQtMainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(CrQtMainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        CrQtMainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(CrQtMainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        CrQtMainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionNew_Scene);
        menuFile->addAction(actionOpen_Scene);
        menuFile->addSeparator();

        retranslateUi(CrQtMainWindow);

        QMetaObject::connectSlotsByName(CrQtMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *CrQtMainWindow)
    {
        CrQtMainWindow->setWindowTitle(QApplication::translate("CrQtMainWindow", "Corsair Engine 0.01", nullptr));
        actionNew_Scene->setText(QApplication::translate("CrQtMainWindow", "New Scene", nullptr));
        actionOpen_Scene->setText(QApplication::translate("CrQtMainWindow", "Open Scene", nullptr));
        menuFile->setTitle(QApplication::translate("CrQtMainWindow", "File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CrQtMainWindow: public Ui_CrQtMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CRQTMAINWINDOW_H
