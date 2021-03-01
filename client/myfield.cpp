#include "myfield.h"

ClickableCell::ClickableCell(QWidget *parent, int i, int j)
    :QLabel(parent) {
    tag = i * 10 + j;
}


void ClickableCell::mousePressEvent (QMouseEvent *event) {
 emit clicked(event);
}


int ClickableCell::getTag(void) {
 return this->tag;
}


MyField::~MyField(){
    qDeleteAll(labels);
    labels.clear();
}


void MyField::paintEvent(QPaintEvent *){// painting field
    QPainter painter(this);

    painter.setBrush(Qt::lightGray);
    int x = cSize, y = cSize;

    for(int i = 1; i < 11; ++i){
        painter.drawRect(QRect(x, 0, cSize, cSize));
        painter.drawRect(QRect(0, x, cSize, cSize));
        painter.drawText(QRect(x, 0, cSize, cSize),Qt::AlignCenter,((QString)"%1").arg(i - 1));//horiz
        painter.drawText(QRect(0, x, cSize, cSize),Qt::AlignCenter,((QString)"%1").arg((char)('A' + i - 1)) );//vert
        x += cSize;
    }

    x = cSize;

    for(int i = 0; i < 10; ++i){
        for(int j = 0; j < 10; ++j)
        {
            switch (arr[i][j]){
            case 0 : painter.setBrush(Qt::blue); break; // sea
            case 1 : painter.setBrush(Qt::gray); break; // ship
            case 2 : painter.setBrush(Qt::darkBlue); break; // miss
            case 3 : painter.setBrush(Qt::red); break; // hit
            case 4 : painter.setBrush(Qt::black); break; // choosing cell
            }
           QRect r(x, y, cSize, cSize);
           painter.drawRect(r);
           x += cSize;
        }
        x = cSize;
        y += cSize;
    }
}


void MyField::createClickableField(){//created once when app starts

    for (int i = 0; i < 10; ++i){
        for (int j = 0; j < 10; ++j){
            labels << new ClickableCell (this, i, j);
        }
    }

    QGridLayout * myLayout = new QGridLayout(this);
    myLayout->setSpacing(0);
    myLayout->setContentsMargins(cSize, cSize, this->width() - 11 * cSize, this->height() - 11 * cSize);

     for (int i = 0; i < 10; ++i){
           for (int j = 0; j < 10; ++j)
           {
              labels.at(i * 10 + j)->setFixedSize(QSize(cSize, cSize));

              QSignalMapper* signalMapper = new QSignalMapper (this) ;
              connect (labels.at( i * 10 + j), SIGNAL(clicked(QMouseEvent*)), signalMapper, SLOT(map())) ;
              signalMapper -> setMapping (labels.at(i * 10 + j), labels.at(i * 10 + j)->getTag()) ;
              connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(labelClicked(int))) ;

              myLayout->addWidget(labels.at(i * 10 + j), i + 1, j + 1);
           }
     }
}

void MyField::fillTheField(){ // fill with ships at the start of each new game
    QTime midnight(0, 0, 0);
    qsrand(midnight.secsTo(QTime::currentTime()));

    int i = 0;
    int j = 0;
    bool horiz = 0;

    //4
    int len = 4;
    do {horiz = qrand() % 2;// 1 = horiz
        i = qrand() % (10 - (len - 1) * (!horiz));
        j = qrand() % (10 - (len - 1) * horiz);
    } while (!addShip(i, j, horiz, 4));

    //3 3
    len = 3;
    for (int num = 1; num < 3 ; ++num){
       do { horiz = qrand() % 2;
           i = qrand() % (10 - (len-1) * (!horiz));
           j = qrand() % (10 - (len - 1) * horiz);
       } while (!addShip(i, j, horiz, 3));
    }

   //2 2 2
   len = 2;
   for (int num = 1; num < 4; ++num){
       do { horiz = qrand() % 2;
            i = qrand() % (10 - (len - 1) * (!horiz));
            j = qrand() % (10 - (len - 1) * horiz);
       } while (!addShip(i, j, horiz, 2));
    }

   // 1 1 1 1
   for (int num = 1; num < 5; ++num){
        do { i = qrand() % 10;
             j = qrand() % 10;
       } while (!addShip(i, j, true, 1));
    }

   update();
}


bool MyField::addShip(int i, int j, bool horiz, int len){
    int k = 0;
    int l = 0;

    if (len == 4){
        if(horiz == true){
            for (k = j; k < j + len; ++k)
                arr[i][k] = 1;
        } else {
            for (k = i; k < i + len; ++k)
                arr[k][j] = 1;
        }
        return true;
    }

    if(horiz == true){
        for (k = std::max(0, i - 1); k <= std::min(10, i + 1); ++k){
            for (l = std::max(0, j - 1); l <= std::min(10, j + len); ++l)
            {
                if (arr[k][l] == 1)// if another ship is too close
                    return false;
            }
        }
        for (k = j; k < j + len; ++k)
            arr[i][k] = 1;
    } else {
        for (k = std::max(0,i - 1); k <= std::min(10, i + len); ++k)
        {
            for (l = std::max(0,j - 1); l <= std::min(10, j + 1 ); ++l)
            {
                if (arr[k][l] == 1)
                    return false;
            }
        }
        for (k = i; k < i + len; ++k)
            arr[k][j] = 1;
        }
    return true;
}


bool MyField::checkShips(){
    if (hitCount == 1)//22)
        return true;
    return false;
}


bool MyField::hasBeenHit(int n, bool success) // mark cell as hit, return true if it was a ship
{
    int i = (int) n / 10;
    int j = n % 10;
    if (i < 0 || i > 9){
        qDebug() << "Incorrect value sent to hasBeenHit()";
        return false;
    }

    if (arr[i][j] == 1 || success)//if ship
        ++hitCount;

    if (arr[i][j] == 4)//if black on enemyfield
        arr[i][j] = 2 + success;
    else
        arr[i][j] += 2 + success;//mark as hit

    update();

    return arr[i][j] == 3? true: false;
}


void MyField::destroyField(){

    for (int i = 0; i < 10; ++i)
    {
        for(int j = 0; j < 10; ++j)
        {
            arr[i][j] = 0;
        }
    }
    hitCount = 0;
    chosenCell = -1;
    update();
}


void MyField::labelClicked(int t){

    int i = (int) t / 10 ;
    int j = t % 10;

    if (i < 0 || i > 9){
        qDebug() << "Incorrect value sent to labelClicked()";
        return;
    }

    if (arr[i][j] != 0 && arr[i][j] != 4)//you can only click sea or previously chosen  cells
        return;

    if (chosenCell == t) // same cell clicked twice
    {
        chosenCell = - 1;
        arr[i][j] = 0;
        emit changedChosenCell();
        update();
        return;
    } else
        if(chosenCell != -1)// change status of previous chosen cell (chosen->sea)
        {
            int tmpi = (int) chosenCell / 10 ;
            int tmpj = chosenCell % 10;
            arr[tmpi][tmpj] = 0;
        }

    chosenCell = t;
    arr[i][j] = 4;

    update();
    emit changedChosenCell();
}


int MyField::getChosenCell(){
    return chosenCell;
}


void MyField::setChosenCell(int n){
    chosenCell = n;
}
