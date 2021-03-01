#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QtNetwork>
#include <QTcpSocket>
#include <QByteArray>
#include <QPushButton>
#include <QMessageBox>
#include <QPaintEvent>
#include <QStatusBar>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTime>

#include "myfield.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;
    MyField* field;
    MyField* enemyField;

    bool gameInProgress = false;
    int yourTurn = -1;// -1 - not defined, 0 - no, 1 - yes
    QTcpSocket* socket;

    QPlainTextEdit* myLog;

    QMessageBox* waitBox;
    QMessageBox* okBox;

    QLabel* turnInfo;
    QPushButton* chooseCellButton;

    void endTheGame(int state = 0);
    void appendMessageToLog(QString);

private slots:
    void startNewGame();
    void enableChooseCellButton();
    void sendChosenCell();
    void readFromServer();
    void changeTurn();
    void disconnectedFromServer();
signals:
    void newTurn();
};

#endif // MAINWINDOW_H
