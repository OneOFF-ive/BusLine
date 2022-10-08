#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myGraphicsView/mygraphicsview.h"
#include <QFile>
#include <QTextStream>
#include <QQueue>
#include "adjMatrix/adjmatrix.h"
#include <thread>
#include <map/map.h>
#include <QFileDialog>

vector<int> mySplit(const string &str, const string &pattern);
const vector<BusVex> getBusLine(string v);


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //鼠标点击时更新信息
    connect(ui->graphicsView, &MyGraphicsView::mouseClicked, [=](){
        infoUpdate();
    });

    connect(ui->graphicsView, &MyGraphicsView::buildLOG, [=](QString log){
        ui->LogList->addItem(log);
        ui->LogList->setCurrentRow(ui->LogList->count()-1);
    });

    connect(ui->graphicsView, &MyGraphicsView::delBus, [=](int busNo){
        del_bus_before_vex(busNo);
    });

    connect(this, &MainWindow::go_to_del_bus, ui->graphicsView, &MyGraphicsView::delBusVex);

    ui->graphicsView->getMainWindow(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::infoUpdate()
{
    ui->vexsNum->setText(QString::number(ui->graphicsView->vexNumber()));
    ui->LinesNum->setText(QString::number(ui->graphicsView->linesNumber()));
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    infoUpdate();
    QMainWindow::mousePressEvent(event);
}

void MainWindow::on_DelButton_clicked()
{
    ui->graphicsView->removeSelectItem();
    infoUpdate();
}

void MainWindow::on_setClearButton_clicked()
{
    ui->graphicsView->clearView();
}

void MainWindow::on_DijkstraButton_clicked()
{
    if (ui->graphicsView->getCurrentVexItem() != nullptr) {
        QQueue<DijkVex> visitQueue = ui->graphicsView->Dijkstra();
        vector<VexItem *> visitVexs;
        visitVexs.reserve(visitQueue.size());
        while (!visitQueue.empty()) {
            visitVexs.emplace_back(ui->graphicsView->Vexs()[visitQueue.front().no]);
            visitQueue.dequeue();
        }

        if(!visitVexs.empty()) {
            for (size_t i = 0; i < visitVexs[0]->getBusNos().size(); i++) {
                string s = "";
                calLine(visitVexs, 0, visitVexs[0]->getBusNos()[i], s);
            }
        }
    }
}

void MainWindow::calLine(vector<VexItem *> visitVexs, size_t k, int curBusNo, std::string s)
{
    if (k == visitVexs.size() - 1) {
        ui->LogList->addItem("乘车顺序：" + QString::fromStdString(s));
        ui->LogList->setCurrentRow(ui->LogList->count()-1);
        return;
    }

    //如果当前乘坐的公交车路过下一个目标站点，则不换车
    for (size_t j = 0; j < visitVexs[k + 1]->getBusNos().size(); j++) {
        int t1 = visitVexs[k + 1]->getBusNos()[j];
        if (curBusNo == t1) {
            s += " ";
            s += std::to_string(t1);
            calLine(visitVexs, k + 1, curBusNo, s);
            return;
        }
    }

    //如果当前乘坐的公交车不路过下一个目标站点，则换车
    if (k != 0) {
        for (size_t i = 0; i < visitVexs[k]->getBusNos().size(); i++) {
            int t1 = visitVexs[k]->getBusNos()[i];
            bool hasSameBus = false;
            for (size_t j = 0; j < visitVexs[k + 1]->getBusNos().size(); j++) {
                int t2 = visitVexs[k + 1]->getBusNos()[j];
                if (t1 == t2) {
                    hasSameBus = true;
                    break;
                }
            }
            if (hasSameBus) {
                string tmp = s + " " + std::to_string(t1);
                calLine(visitVexs, k + 1, t1, tmp);
            }
        }
    }
}

void MainWindow::add_bus(QString busNo_s, QString busStop_s)
{
    if (busNo_s != "" and busStop_s != "") {
        for (auto item : busList) {
            //已经存在该公交路线
            if ( item.getNo() == stoi(busNo_s.toStdString()) ) {
                ui->LogList->addItem("该路线已存在");
                ui->LogList->setCurrentRow(ui->LogList->count()-1);
                return;
            }
        }
        //创建该公交路线并加入到集合中
        vector<BusVex> busVexs = getBusLine(busStop_s.toStdString());
        AdjMatrix matrix(ui->graphicsView);

        if (matrix.is_a_line(busVexs)) {
            busList.emplace_back(Bus(stoi(busNo_s.toStdString()), busVexs));
            //将该公交路线名加入界面列表中展示
            ui->busList->addItem(busNo_s);

            //将该公交车路线名写入经过的站点中
            for (BusVex &busVex : busVexs) {
                ui->graphicsView->Vexs()[busVex.no]->getBusNos().emplace_back(stoi(busNo_s.toStdString()));
            }

        }
        else {
            ui->LogList->addItem("路线输入有误，请重新输入");
            ui->LogList->setCurrentRow(ui->LogList->count()-1);
        }
    }
}

void MainWindow::change_bus(QString busNo_s, QString busStop_s)
{
    if (busNo_s != "" and busStop_s != "") {

        for (auto &item : busList) {
            //找到要修改的公交线路编号
            if ( item.getNo() == stoi(busNo_s.toStdString()) ) {
                AdjMatrix matrix(ui->graphicsView);
                vector<BusVex> newBusVexs = getBusLine(busStop_s.toStdString());
                if (matrix.is_a_line(newBusVexs)) {

                    //删除地图上该公交编号所有的旧停站点
                    vector<BusVex> oldbusVexs = item.getVexs();
                    for (BusVex &oldbusvex : oldbusVexs) {
                        vector<int> &busNos = ui->graphicsView->Vexs()[oldbusvex.no]->getBusNos();
                        for(auto iter = busNos.begin(); iter != busNos.end(); ) {
                            if (*iter == item.getNo()) {
                                busNos.erase(iter);
                                break;
                            }
                            else {
                                iter++;
                            }
                        }
                    }

                    //更新公交停站点
                    item.changeVexs(newBusVexs);
                    //将该公交车路线名写入新的经过的站点中
                    for (BusVex &busVex : newBusVexs) {
                        ui->graphicsView->Vexs()[busVex.no]->getBusNos().emplace_back(stoi(busNo_s.toStdString()));
                    }
                }
                else {
                    ui->LogList->addItem("路线输入有误，请重新输入");
                    ui->LogList->setCurrentRow(ui->LogList->count()-1);
                }
                return;
            }
        }
        ui->LogList->addItem("该路线不存在");
        ui->LogList->setCurrentRow(ui->LogList->count()-1);
    }
}

void MainWindow::del_bus(QString busNo_s)
{
    if (busNo_s != "") {


        for (auto item = busList.begin(); item != busList.end();) {
            //找到要删除的公交线路编号
            if ( item->getNo() == stoi(busNo_s.toStdString()) ) {
                //删除地图上该公交编号所有的旧停站点
                vector<BusVex> oldbusVexs = item->getVexs();
                for (BusVex &oldbusvex : oldbusVexs) {
                    emit go_to_del_bus(oldbusvex.no, item->getNo());
                }

                //遍历ui->busList，寻找要删除的公交线路编号，并删除
                for (int i = 0; i < ui->busList->count(); ++i) {
                    QListWidgetItem* pItem = ui->busList->item(i);
                    if (pItem->text() == busNo_s)
                    {
                        ui->busList->setCurrentItem(pItem);
                        break;
                    }
                }
                delete ui->busList->currentItem();

                //删除busLis中的公交编号，删除完成
                busList.erase(item);
                return;
            }
            else {
                item++;
            }
        }
        ui->LogList->addItem("该路线不存在");
        ui->LogList->setCurrentRow(ui->LogList->count()-1);
    }
}

void MainWindow::getBusFromMap(vector<Bus> bs)
{
    for (auto &b : bs) {
        busList.emplace_back(b);
        //将该公交路线名加入界面列表中展示
        ui->busList->addItem(QString::number(b.getNo()));

        //将该公交车路线名写入经过的站点中
        for (BusVex &busVex : b.getVexs()) {
            ui->graphicsView->Vexs()[busVex.no]->getBusNos().emplace_back(b.getNo());
        }
    }
}

void MainWindow::del_bus_before_vex(int busNo)
{
    for (auto item = busList.begin(); item != busList.end();) {
        //找到要删除的公交线路编号
        if ( item->getNo() == busNo ) {
            //遍历ui->busList，寻找要删除的公交线路编号，并删除
            for (int i = 0; i < ui->busList->count(); ++i) {
                QListWidgetItem* pItem = ui->busList->item(i);
                if (pItem->text() == QString::number(busNo))
                {
                    ui->busList->setCurrentItem(pItem);
                    break;
                }
            }
            delete ui->busList->currentItem();

            //删除busLis中的公交编号，删除完成
            busList.erase(item);
            return;
        }
        else {
            item++;
        }
    }
}

void MainWindow::on_FloydButton_clicked()
{
    if (ui->graphicsView->getCurrentVexItem() != nullptr) {
        QQueue<DijkVex> visitQueue = ui->graphicsView->Floyd();
        vector<VexItem *> visitVexs;
        while (!visitQueue.empty()) {
            visitVexs.emplace_back(ui->graphicsView->Vexs()[visitQueue.front().no]);
            visitQueue.dequeue();
        }

        if(!visitVexs.empty()) {
            for (size_t i = 0; i < visitVexs[0]->getBusNos().size(); i++) {
                string s = "";
                calLine(visitVexs, 0, visitVexs[0]->getBusNos()[i], s);
            }
        }
    }
}

vector<int> mySplit(const string &str, const string &pattern)
{
    vector<int> res;
    res.reserve(10);
    if(str == "")
        return res;
    //在字符串末尾也加入分隔符，方便截取最后一段
    string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while(pos != strs.npos)
    {
        string temp = strs.substr(0, pos);
        if (temp != "") {
            res.push_back(stoi(temp));
        }
        //去掉已分割的字符串,在剩下的字符串中进行分割
        strs = strs.substr(pos+1, strs.size());
        pos = strs.find(pattern);
    }

    return res;
}

const vector<BusVex> getBusLine(string v) {
    vector<int> res = mySplit(v, " ");
    vector<BusVex> vexs;
    vexs.reserve(res.size());
    for (size_t i = 0; i < res.size(); i++) {
        if (i == 0) {
            vexs.emplace_back(BusVex{res[i], res[i]});
        }
        else {
            vexs.emplace_back(BusVex{res[i], res[i - 1]});
        }
    }
    return vexs;
}

void MainWindow::on_btn_addBus_clicked()
{
    QString busNo_s = ui->busNoEdit->text();
    QString busStop_s = ui->busStopEdit->text();
    add_bus(busNo_s, busStop_s);
    ui->busNoEdit->setText("");
    ui->busStopEdit->setText("");
}


void MainWindow::on_busList_itemClicked(QListWidgetItem *item)
{
    ui->busNoEdit->setText(item->text());
    Bus bus;
    for (auto &b : busList) {
        if(b.getNo() == stoi(item->text().toStdString())) {
            bus = b;
        }
    }
    QString stopText = "";
    for (auto &i : bus.getVexs()) {
        stopText += QString::number(i.no);
        stopText += " ";
    }
    ui->busStopEdit->setText(stopText);
}


void MainWindow::on_btn_changeBus_clicked()
{
    QString busNo_s = ui->busNoEdit->text();
    QString busStop_s = ui->busStopEdit->text();
    change_bus(busNo_s, busStop_s);
    ui->busNoEdit->setText("");
    ui->busStopEdit->setText("");
}

void MainWindow::on_busList_itemDoubleClicked(QListWidgetItem *item)
{
    ui->busNoEdit->setText(item->text());
    Bus bus;
    for (auto &b : busList) {
        if(b.getNo() == stoi(item->text().toStdString())) {
            bus = b;
        }
    }
    QQueue<BusVex> aniQueue;
    vector<BusVex> vexs = bus.getVexs();
    for (BusVex item : vexs) {
        aniQueue.enqueue(item);
    }
    ui->graphicsView->playAnimation(aniQueue);
}


void MainWindow::on_btn_delBus_clicked()
{
    QString busNo_s = ui->busNoEdit->text();
    QString busStop_s = ui->busStopEdit->text();
    del_bus(busNo_s);
    ui->busNoEdit->setText("");
    ui->busStopEdit->setText("");
}

void MainWindow::clearBusView()
{
    while (!busList.empty()) {
        del_bus(QString::number(busList[0].getNo()));
    }
    busList.clear();
}

void MainWindow::on_btn_clearBus_clicked()
{
    clearBusView();
}

void MainWindow::on_btn_save_clicked()
{
    QDir cur_path(QCoreApplication::applicationDirPath());
    //当前目录的上一层
    cur_path.cdUp();
    //转成字符串
    QString filePath = cur_path.absolutePath();
    //添加后续具体路径
    filePath = filePath + tr("/maps");


    QString file_name = QFileDialog::getSaveFileName(NULL, "保存", filePath, "*.map");
    qDebug() << file_name;
    Map map(ui->graphicsView->Vexs(), ui->graphicsView->Lines(), busList, ui->graphicsView);
    map.save(file_name.toStdString());
}

void MainWindow::on_btn_open_clicked()
{
    QDir cur_path(QCoreApplication::applicationDirPath());
    //当前目录的上一层
    cur_path.cdUp();
    //转成字符串
    QString filePath = cur_path.absolutePath();
    //添加后续具体路径
    filePath = filePath + tr("/maps");

    QString file_name = QFileDialog::getOpenFileName(NULL, "打开", filePath, "*.map");
    Map map;
    clearBusView();
    ui->graphicsView->clearView();
    QVector<VexItem *> vs;
    QVector<LineItem *> ls;
    vector<Bus> bs;
    map.read(file_name.toStdString(), vs, ls, bs, this);
    ui->graphicsView->getVexFromMap(vs);
    ui->graphicsView->getLineFromMap(ls);
    getBusFromMap(bs);
}
