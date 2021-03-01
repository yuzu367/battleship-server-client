#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTcpSocket>
#include <QObject>
#include <QByteArray>
#include <QDebug>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void stopServer();
    void addNewUser();
    void readFromClient();
    void deleteClient();

private:
    Ui::MainWindow* ui;

    int numOfClients = 0;
    int server_status = 0;
    bool playerTurn = false;
    bool gameInProgress = false;

    QPlainTextEdit* myLog;
    QTcpServer* tcpServer;
    QList<QTcpSocket *> SClients;

    void gameEnded(bool = true);// true = game correctly ended, false = someone disconnected
    void appendMessage(QString);

};
#endif // MAINWINDOW_H
