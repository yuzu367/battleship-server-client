#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Server");

    QWidget* wid = new QWidget(this);
    this->setCentralWidget(wid);
    QVBoxLayout* myLayout = new QVBoxLayout(wid);
    wid->setLayout(myLayout);

    QLabel* label = new QLabel("Server log", this);
    myLayout->addWidget(label, 0);
    myLayout->setAlignment(label, Qt::AlignCenter);

    myLog = new QPlainTextEdit(this);
    myLog->setReadOnly(true);
    myLayout->addWidget(myLog, 1);

    QPushButton* stopServerButton = new QPushButton("Stop server", this);
    myLayout->addWidget(stopServerButton, 2);
    QObject::connect(stopServerButton, SIGNAL(clicked()), this, SLOT(stopServer()));

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(addNewUser()));

    if (!tcpServer->listen(QHostAddress::Any, 33333) && server_status == 0)
    {
        qDebug() <<  QObject::tr("Unable to start the server: %1.").arg(tcpServer->errorString());
        appendMessage("Unable to start the server");
    } else
    {
        server_status = 1;
        qDebug() << tcpServer->isListening() << "Server is ready and listening";
        appendMessage("Server is ready");
    }    
}


MainWindow::~MainWindow()
{
    if(server_status == 1)
        stopServer();
    delete ui;
    delete myLog;
    delete tcpServer;
}


void MainWindow::stopServer()
{
    if(server_status == 1)
    {
        for(int i = 0; i < SClients.length(); ++i)
        {
            SClients[i]->close();
        }

        tcpServer->close();
        qDebug() << "Server is stopped";
        server_status = 0;
    }
    qApp->exit();
}


void MainWindow::addNewUser()
{
    if(server_status == 1){
        QTcpSocket* clientSocket=tcpServer->nextPendingConnection();
        SClients.push_back(clientSocket);

        qDebug() << "New connection!";
        appendMessage(((QString)"New connection #%1").arg(clientSocket->socketDescriptor()));

        connect(SClients[numOfClients], SIGNAL(readyRead()), this, SLOT(readFromClient()));
        connect(SClients[numOfClients++], SIGNAL(disconnected()), this, SLOT(deleteClient()));

        if (numOfClients == 2)
        {
            tcpServer->pauseAccepting();
            QDataStream ds (SClients[playerTurn]);//first to connect
            ds << 1;//first player
            QDataStream os (SClients[!playerTurn]);
            os << 0;//second player

            appendMessage("Both players are connected");
            gameInProgress = true;
        } else
            appendMessage("Waiting for second player");
    }
}


void MainWindow::readFromClient(){
    QDataStream ds((QTcpSocket*)sender());
    int index = SClients.indexOf((QTcpSocket*)sender());

    // if several games in a row, winner plays first
    // winner sends a message to indicate that it is his turn now
    if(!gameInProgress) {
        int a;
        ds >> a;
        playerTurn = index;
        gameInProgress = true;
        appendMessage("New game started");
        return;
    }

    bool success = false;
    int chosenCell = -1;

    if (index == playerTurn) // player 1 sends
    {
        ds >> gameInProgress >> chosenCell;

        if (!gameInProgress){
            appendMessage(((QString)"Player #%1 won!").
                          arg(((QTcpSocket*)sender())->socketDescriptor()));
            gameEnded(true);
            return;
        }

        qDebug() << "next move" << chosenCell;
        appendMessage(((QString)"Player #%1 chooses cell %2:%3..").
                      arg((((QTcpSocket*)sender())->socketDescriptor())).
                      arg((int)chosenCell / 10).arg (chosenCell % 10 ));

        QDataStream os(SClients[!playerTurn]);
        os << gameInProgress << chosenCell;

    } else// player 2 sends
    {
        ds >> gameInProgress >> success;

        if (!gameInProgress){
            appendMessage(((QString)"Player #%1 won!").
                          arg(((QTcpSocket*)sender())->socketDescriptor()));
            gameEnded(true);
            return;
        }

        qDebug() << "success" << success;
        appendMessage(((QString)"..and it's a %1").arg(success? "hit": "miss"));

        QDataStream os(SClients[playerTurn]);
        os << gameInProgress << success;
        playerTurn = !playerTurn;

    }
}

void MainWindow::deleteClient(){

    --numOfClients;
    playerTurn = false;

    int index = SClients.indexOf((QTcpSocket*)sender());
    SClients[index]->deleteLater();
    SClients.removeAt(index);

    gameEnded(false);

    qDebug() << "deleted client";
    appendMessage("Client disconnected");

    tcpServer->resumeAccepting();
}

void MainWindow::gameEnded(bool ending) {

    qDebug() << "game over";
    appendMessage("Game over");
    gameInProgress = false;
    playerTurn = false;

    if(!ending && SClients.length()){ // if one client disconected, notify another client (if he exists)
        QDataStream os(SClients[0]);
        os<<gameInProgress;
    }
}

void MainWindow::appendMessage(QString message){
    myLog->appendPlainText((QString("%1 ")).arg((QTime::currentTime()).toString("hh:mm:ss"))+message);
    myLog->verticalScrollBar()->setValue(myLog->verticalScrollBar()->maximum());
}
