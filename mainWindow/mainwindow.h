#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qevent.h>
#include <bus/bus.h>
#include <qlistwidget.h>
#include "vexItem/vexitem.h"
#include <QMenuBar>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void infoUpdate();

    void mousePressEvent(QMouseEvent *event);

    void clearBusView();

    void calLine(vector<VexItem *> visitVexs, size_t k, int curBusNo, string s);

    void add_bus(QString busNo_s, QString busStop_s);
    void change_bus(QString busNo_s, QString busStop_s);
    void del_bus(QString busNo_s);

    void getBusFromMap(vector<Bus> bs);
    void del_bus_before_vex(int busNo);

private slots:
    void on_DelButton_clicked();

    void on_setClearButton_clicked();

    void on_DijkstraButton_clicked();

    void on_FloydButton_clicked();

    void on_btn_addBus_clicked();

    void on_busList_itemClicked(QListWidgetItem *item);

    void on_btn_changeBus_clicked();

    void on_busList_itemDoubleClicked(QListWidgetItem *item);

    void on_btn_delBus_clicked();

    void on_btn_clearBus_clicked();

    void on_btn_save_clicked();

    void on_btn_open_clicked();

private:
    Ui::MainWindow *ui;
    vector<Bus> busList;

    QMenuBar *menuBar;
    QMenu *menu;
signals:
    void go_to_del_bus(int vexNo, int busNo);
};
#endif // MAINWINDOW_H
