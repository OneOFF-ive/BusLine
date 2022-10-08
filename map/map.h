#ifndef MAP_H
#define MAP_H

#include <QVector>
#include <vector>
#include "vexItem/vexitem.h"
#include "lineItem/lineitem.h"
#include "bus/bus.h"
#include "myGraphicsView/mygraphicsview.h"
#include "mainWindow/mainwindow.h"

class Map : public QObject
{
private:
    QVector<VexItem *> vexs;
    QVector<LineItem *> lines;
    vector<Bus> busList;
    MyGraphicsView *mv;
public:
    Map();
    Map(QVector<VexItem *> v, QVector<LineItem *> l, vector<Bus> b, MyGraphicsView *mv);
    void save(const string &path);
    void read(const string &path, QVector<VexItem *> &v, QVector<LineItem *> &l, vector<Bus> &b, MainWindow *mw);
    static vector<string> mySplit(const string &str, const string &pattern);

};

#endif // MAP_H
