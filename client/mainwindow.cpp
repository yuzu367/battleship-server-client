#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWidget* wid = new QWidget(this);
    this->setCentralWidget(wid);
    QGridLayout* myLayout = new QGridLayout(wid);
    myLayout->setContentsMargins(50, 50, 50, 10);

    QPalette pal = this->palette();
    pal.setColor(QPalette::Background, QColor(36, 36, 74));
    pal.setColor(QPalette::Button, QColor(Qt::lightGray));
    this->setPalette(pal);
    this->update();

    // your field
    QLabel* yourFieldLabel = new QLabel ("Your field", this);
    myLayout->addWidget(yourFieldLabel, 0, 0, Qt::AlignHCenter);

    field = new MyField(this);
    field->setFixedSize(250, 250);
    myLayout->addWidget(field, 1, 0, 2, 1);

    // enemy field
    QLabel* enemyFieldLabel = new QLabel ("Enemy field", this);
    myLayout->addWidget(enemyFieldLabel, 0, 2, Qt::AlignHCenter);


    enemyField = new MyField(this);
    enemyField->setFixedSize(250, 250);
    enemyField->createClickableField();
    myLayout->addWidget(enemyField, 1, 2, 2, 1);

    turnInfo = new QLabel ("You can start a new game", this);
    myLayout->addWidget(turnInfo, 3, 0, 1, 3, Qt::AlignCenter);

    // log
    myLog = new QPlainTextEdit(this);
    myLog->setStyleSheet("background-color : lightGray");
    myLog->setFixedSize(this->width() / 4 * 3, 150);
    myLog->setReadOnly(true);
    appendMessageToLog("Welcome!");
    myLayout->addWidget(myLog, 4, 0, 1, 3, Qt::AlignCenter);

    // buttons
    QPushButton* quitButton = new QPushButton("End Game", this);
    quitButton->setFixedSize(110, 50);
    QObject::connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    myLayout->addWidget(quitButton, 1, 1, Qt::AlignCenter);

    QPushButton* newGameButton = new QPushButton("New Game", this);
    newGameButton->setFixedSize(110, 50);
    QObject::connect(newGameButton, SIGNAL(clicked()), this, SLOT(startNewGame()));
    myLayout->addWidget(newGameButton, 0, 1, Qt::AlignCenter);

    chooseCellButton = new QPushButton("Choose this cell", this);
    chooseCellButton->setFixedSize(150, 50);
    QObject::connect(chooseCellButton, SIGNAL(clicked()), this, SLOT(sendChosenCell()));
    QObject::connect(this, SIGNAL(newTurn()),this, SLOT(changeTurn()));
    QObject::connect(enemyField, SIGNAL(changedChosenCell()), this, SLOT(enableChooseCellButton()));
    chooseCellButton->setEnabled(false);
    myLayout->addWidget(chooseCellButton, 2, 1, Qt::AlignCenter);

    waitBox = new QMessageBox(this);
    waitBox->setWindowTitle("Please wait");
    waitBox->addButton("Close and disconnect", QMessageBox::DestructiveRole);
    connect(waitBox, &QMessageBox::buttonClicked, [=](QAbstractButton* button){
        if(waitBox->buttonRole(button)==QMessageBox::DestructiveRole)
        {
            socket->close();
            qDebug() << "close and disconnect";
        }
    });

    okBox = new QMessageBox(this);
    okBox->setWindowTitle("Notification");

    socket = new QTcpSocket;
    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readFromServer()));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(disconnectedFromServer()));
}


MainWindow::~MainWindow(){
    delete chooseCellButton;
    delete socket;
    delete turnInfo;
    delete waitBox;
    delete myLog;
    delete field;
    delete enemyField;
    delete ui;
}


void MainWindow::startNewGame(){

    if (gameInProgress==true) {
        okBox->setText("You cannot start the new game right now");
        okBox->show();
        return;
    }

    if(socket->state() != QTcpSocket::ConnectedState){// if not connected yet
        qDebug() << "connecting to server";
        socket->connectToHost(QHostAddress::LocalHost, 33333);
        QDataStream os(socket);
        if(socket->waitForConnected(5000) == false) {
            qDebug() << "couldn't connect to host";
            okBox->setText("Couldn't connect to the host :( ");
            okBox->show();
            return;
        }
        appendMessageToLog("Successfully connected to server");
    }

    if (yourTurn == -1) // if no opponent is yet available, waiting for server to send yourTurn
    {
        waitBox->setText("Waiting for second player to connect");
        waitBox->show();
    }
    else {
        if(yourTurn == 1) // if you won last round
        {
            QDataStream os(socket);
            os << yourTurn;
        }

        emit newTurn();
        appendMessageToLog("New game has started!");
    }

    gameInProgress  = true;


    field->destroyField();
    enemyField->destroyField();

    field->fillTheField();
    chooseCellButton->setEnabled(false);
};


void MainWindow::sendChosenCell(){

    QDataStream os(socket);
    int chosenCell = enemyField->getChosenCell();

    if(chosenCell == -1)// just in case?
        return;

    os << gameInProgress << chosenCell;
    qDebug() << "sent" << chosenCell;
    appendMessageToLog((((QString)"You chose cell %1%2..").
                        arg((char)('A' + (int) chosenCell / 10))).arg(chosenCell % 10));

    chooseCellButton->setEnabled(false);
}


void MainWindow::readFromServer(){

    QDataStream os(socket);
    if(yourTurn == -1) // server sends you your turn, when playing for the first time
    {
        os >> yourTurn;
        emit newTurn();
        appendMessageToLog("New game has started!");

        return;
    }

    int chosenCell = enemyField->getChosenCell();;
    bool success = false;

    if (yourTurn)
    {
        os >> gameInProgress >> success;
        if(!gameInProgress) // checking if another player has disconnected
        {
            endTheGame(2);
            return;
        }
        qDebug() << "success" << success;
        appendMessageToLog(((QString)"..and it's a %1").arg(success? "hit": "miss"));

        enemyField->hasBeenHit(chosenCell, success);
        enemyField->setChosenCell(-1);
    } else
    {
        if(!gameInProgress){ // when the next game has not yet started, but server sends a message
            os >> gameInProgress;
            if(!gameInProgress) // either second player has disconnected
            {
                endTheGame(2);
                return;
            }
            gameInProgress = false;
            QMessageBox::StandardButton reply; // or has started a new game
              reply = QMessageBox::question(this, "Opponent has started a new game", "Do you wish to join?",
                                           QMessageBox::Yes|QMessageBox::No);
              if (reply == QMessageBox::Yes)
                  startNewGame();
              else{
                  socket->close();
                  return;
              }
              os >> chosenCell;

        } else
            os >> gameInProgress >> chosenCell;

        if(!gameInProgress)
        {
            endTheGame(2);
            return;
        }

        qDebug() << "read" << chosenCell;
        appendMessageToLog((((QString)"Opponent chose cell %1%2..").
                            arg((char)('A' + (int) chosenCell / 10))).arg(chosenCell % 10));

        success = field->hasBeenHit(chosenCell);
        os << gameInProgress << success;
        appendMessageToLog(((QString)"..and it's a %1").arg(success? "hit": "miss"));
        qDebug() << "success" << success;
    }

    if(field->checkShips() == true) { // check if someone won
        endTheGame(1);
        return;
    }else if(enemyField->checkShips() == true){
        endTheGame(0);
        return;
    }

    yourTurn = !yourTurn; // change of turn
    emit newTurn();
}

void MainWindow::enableChooseCellButton(){//cannot click chooseButton if game has not started or cell is not chosen

    if (gameInProgress == false)
            return;

    if (enemyField->getChosenCell() == -1)
        chooseCellButton->setEnabled(false);
    else
        chooseCellButton->setEnabled(true);
}

void MainWindow::changeTurn()
{
    if (yourTurn){
        waitBox->hide();
        turnInfo->setText("You Turn!");
    } else {
        turnInfo->setText("Waiting for your turn");
        waitBox->setText("Waiting for opponent to end his turn");
        waitBox->show();
    }
}


void MainWindow::endTheGame(int state){//0 - win, 1 - lose, 2 - disconnected
    waitBox->hide();

    switch (state)
    {
    case 0 : {
        yourTurn = 1;
        okBox->setText("Congrats!");
        appendMessageToLog("You won!");
        QDataStream os(socket);
        os << false; // telling server that game has ended
        break;}
    case 1 : {
        okBox->setText("You lost :(");
        appendMessageToLog("You lost..");
        yourTurn = 0;
        break; }
    case 2 : {
        okBox->setText("Opponent disconnected :/");
        appendMessageToLog("Opponent disconnected.. ");
        yourTurn = -1;
        break;
    }
    }
    turnInfo->setText("You can start a new game");
    okBox->show();
    gameInProgress = false;
}

void MainWindow::disconnectedFromServer() {
    okBox->setText("disconnected from server");
    appendMessageToLog("You were disconnected from server");
    okBox->show();
    waitBox->hide();
    gameInProgress = false;
    yourTurn = -1;
}

void MainWindow::appendMessageToLog(QString message){
    myLog->appendPlainText((QString("%1 ")).arg((QTime::currentTime()).toString("hh:mm:ss")) + message);
    myLog->verticalScrollBar()->setValue(myLog->verticalScrollBar()->maximum());
}
