#ifndef MYFIELD_H
#define MYFIELD_H
#include <QMainWindow>
#include <QPainter>
#include <QDebug>
#include <QLabel>
#include <QSignalMapper>
#include <QGridLayout>
#include <QMouseEvent>
#include "QTime"

class ClickableCell : public QLabel {
    Q_OBJECT

    int tag;
public:
    explicit ClickableCell(QWidget* parent = nullptr, int i = 0, int j = 0);
    ~ClickableCell(){};
    int getTag();
signals:
    void clicked(QMouseEvent *);
protected:
    void mousePressEvent (QMouseEvent* event);
};


class MyField :public QWidget{
    Q_OBJECT

public:
    MyField(QWidget* parent=nullptr){};
    ~MyField();
    void fillTheField();
    bool hasBeenHit(int, bool success = false);
    bool checkShips();
    void createClickableField();
    int getChosenCell();
    void setChosenCell(int);
    void destroyField();

signals:
    void changedChosenCell();

protected:
    void paintEvent(QPaintEvent* event);

private:
    int arr[10][10] = {{0}};
    int cSize = 22; // size of a rect cell
    int hitCount = 0;// how many hits were successful
    int chosenCell = -1;// id of a chosen cell or -1 if none are chosen

    bool addShip(int i, int j, bool horiz, int len);

    QList <ClickableCell *> labels;

private slots:
    void labelClicked(int t);
};


#endif // MYFIELD_H
