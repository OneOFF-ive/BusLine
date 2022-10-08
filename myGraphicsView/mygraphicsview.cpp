#include "mygraphicsview.h"
#include <QDebug>
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include "vexDrtail/vexdetail.h"
#include "lineDetail/linedetail.h"
#include <QQueue>
#include <QStack>
#include "selectEnd/selectend.h"
#include "adjMatrix/adjmatrix.h"

MyGraphicsView::MyGraphicsView(QWidget *parent) : QGraphicsView(parent), currentSelectVex(nullptr), currentSelectLine(nullptr) {
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-330, -330, 660, 660);
    setScene(scene);
    //不加这一行，线条生成会出现残影
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    //激活整个窗体的鼠标追踪
    setMouseTracking(true);

    //默认线条无方向
    isDirected = false;

    AnimationGroup = new QSequentialAnimationGroup(this);

    isVisiting = false;
}

void MyGraphicsView::addVex(VexItem *vex)
{
    //发送日志
    emit buildLOG(QString("V%1(%2) is built").arg(vex->No()).arg(vex->getTag()));

    this->scene->addItem(vex);
    this->vexs.push_back(vex);
    vex->setNo(Vexs().size() - 1);
}

void MyGraphicsView::addLine(LineItem *line)
{
    //发送日志
    emit buildLOG(QString("L%1(%2) is building (%3->?)").arg(line->No()).arg(line->getTag()).arg(line->getStartVex()->No()));

    this->scene->addItem(line);
    this->lines.push_back(line);
    line->setNo(lines.size() - 1);
}

void MyGraphicsView::delVex(VexItem *vex)
{
    if (vex != nullptr) {
        //发送日志
        emit buildLOG(QString("V%1(%2) is deleted").arg(vex->No()).arg(vex->getTag()));

        //清空所有经过该点的所有公交路线
        for (auto &item : vex->getBusNos()) {
            emit delBus(item);
        }
        vex->getBusNos().clear();

        //在视图中移除该点
        this->scene->removeItem(vex);
        //删除所有和该点相连的边
        for (auto &lineItem: vex->Lines()) {
            delLine(lineItem);
        }
        //清空该点用来储存相连边的容器
        vex->Lines().clear();
        //在视图的点容器中删除该点
        MyGraphicsView::deleteVectorItem<VexItem>(vexs, vex);

        //给当前的所有点重新编号
        for (int i = 0; i < vexs.size(); i++) {
           vexs[i]->setNo(i);
        }
        //点的数量减一
        VexItem::number--;
    }
}

void MyGraphicsView::delLine(LineItem *line)
{
    if(line != nullptr) {
        //发送日志
        if (line->getEndVex() != nullptr) {
            emit buildLOG(QString("L%1(%2) is deleted (%3->%4)").arg(line->No()).arg(line->getTag()).arg(line->getStartVex()->No()).arg(line->getEndVex()->No()));
        }

        //在视图中移除该边
        this->scene->removeItem(line);
        //该边的起始点和终点的连接边容器也要做处理
        MyGraphicsView::deleteVectorItem<LineItem>(line->getStartVex()->Lines(), line);
        if (line->getEndVex() != nullptr) {
            MyGraphicsView::deleteVectorItem<LineItem>(line->getEndVex()->Lines(), line);
        }
        //在视图的边容器中删除该边
        MyGraphicsView::deleteVectorItem<LineItem>(lines, line);

        //给当前的所有边重新编号
        for (int i = 0; i < lines.size(); i++) {
           lines[i]->setNo(i);
        }
        //点的数量减一
        LineItem::number--;
    }
}

void MyGraphicsView::mousePressEvent(QMouseEvent *event)
{
    //当前正在遍历图，点击视图则取消
    if (isVisiting) {
        AnimationGroup->stop();

        for (auto &visitedVex : visitedVexs) {
            visitedVex->setUnVisited();
        }
        for (auto &visitedline : visitedLines) {
            visitedline->setUnVisited();
        }
        visitedVexs.clear();
        visitedLines.clear();
    }


    //鼠标点击选中的Item,没选中则指针为空，选中点，则边的指针为空，反之亦然
    QGraphicsItem *clickedSelectedItem = itemAt(event->pos());
    VexItem *clickedSelectedVex = dynamic_cast<VexItem *>(clickedSelectedItem);
    LineItem *clickedSelecteLine = dynamic_cast<LineItem *>(clickedSelectedItem);

    //鼠标右键生成
    if(event->button() == Qt::RightButton){

        //没有点击选择任何图元，则生成点
        if(clickedSelectedItem == nullptr) {
            QPointF point = this->mapToScene(QPoint(event->pos().x(), event->pos().y()));
            VexItem *vex = new VexItem(point);
            addVex(vex);
        }

        //当前点击的是点，并且该点是被选择状态，且当前没有在生成边，则表明是双击，开始生成边
        if (clickedSelectedVex != nullptr and clickedSelectedVex->IsSelected() and creatingLine == nullptr) {
            LineItem *line;
            //有向图
            if (isDirected) {
                line = new LineItem(clickedSelectedVex, nullptr, true);
            }
            //无向图
            else {
                line = new LineItem(clickedSelectedVex, nullptr, false);
            }
            addLine(line);
            creatingLine = line;
            //起点多了一条连接边
            clickedSelectedVex->addLine(creatingLine);
        }
    }

    //鼠标左键选择
    else if (event->button() == Qt::LeftButton) {

        //当前点击什么也没选中，但是正在创建一条边，那么表明用户想要撤销这条边
        if (clickedSelectedItem == nullptr and creatingLine != nullptr) {
            delLine(creatingLine);
            creatingLine = nullptr;
            //发送日志
            emit buildLOG(QString("Cancel building line"));

        }

        //当前点击选中的是点
        if(clickedSelectedVex != nullptr) {
            //正在生成一条边，表明目前选择的点是边的终点图元
            if(creatingLine != nullptr) {
                creatingLine->setEndvex(clickedSelectedVex);

                //发送日志
                emit buildLOG(QString("L%1(%2) is built (%3->%4)").arg(creatingLine->No()).arg(creatingLine->getTag()).arg(creatingLine->getStartVex()->No()).arg(creatingLine->getEndVex()->No()));
                //终点多了一条连接边
                clickedSelectedVex->addLine(creatingLine);
                creatingLine = nullptr;
                clickedSelectedVex = nullptr;
            }
            //当前不是在生成一条边，则判断该点是否已被选择，如果没被选择，则该点进入选择状态
            else if (!clickedSelectedVex->IsSelected()) {
                clickedSelectedVex->setSelected();

                //发送日志
                emit buildLOG(QString("V%1 is selected").arg(clickedSelectedVex->No()));
            }
            //当前不是在生成一条边，则判断该点是否已被选择，如果被选择，则开始查看点的明细
            else {
                emit buildLOG(QString("Check V%1").arg(clickedSelectedVex->No()));
                VexDetail *vexDetail = new VexDetail(clickedSelectedVex, this);
                vexDetail->exec();
            }
        }

        //当前点击选中的是一条边
        else if(clickedSelecteLine != nullptr) {
            //当前被点击的边不是被选择状态，则令其进入选择状态
            if (!clickedSelecteLine->IsSelected()) {
                clickedSelecteLine->setSelected();

                //发送日志
               emit buildLOG(QString("L%1(%2) is built (%3->%4)").arg(clickedSelecteLine->No()).arg(clickedSelecteLine->getTag()).arg(clickedSelecteLine->getStartVex()->No()).arg(clickedSelecteLine->getEndVex()->No()));
            }
            //当前被点击的边是被选择状态，开始查看边的明细
            else {
                //发送日志
                emit buildLOG(QString("Check L%1 (%2->%3)").arg(clickedSelecteLine->No()).arg(clickedSelecteLine->getStartVex()->No()).arg(clickedSelecteLine->getEndVex()->No()));
                LineDetail *lineDetail = new LineDetail(clickedSelecteLine, this);
                lineDetail->exec();
            }

        }
    }

    //点击事件发生后，将上一个被选中的点恢复到未选择状态
    if(currentSelectVex != nullptr and currentSelectVex != clickedSelectedVex) {
        currentSelectVex->setUnSelected();
    }
    //点击事件发生后，将上一个被选中的边恢复到未选择状态
    if(currentSelectLine != nullptr and currentSelectLine != clickedSelecteLine) {
        currentSelectLine->setUnSelected();
    }
    currentSelectVex = clickedSelectedVex;
    currentSelectLine = clickedSelecteLine;

    emit mouseClicked();

    QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    //表明正在生成边
    if (creatingLine != nullptr) {
        QPointF point = this->mapToScene(QPoint(event->pos().x(), event->pos().y()));
        creatingLine->setEndPoint(point);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void MyGraphicsView::removeSelectItem()
{
    if (currentSelectVex != nullptr) {
        delVex(currentSelectVex);
    }
    else if(currentSelectLine != nullptr) {
        delLine(currentSelectLine);
    }
}

void MyGraphicsView::setDirected()
{
    isDirected = true;
    for (auto &lineItem : lines) {
        lineItem->setDirected();
        lineItem->update();
    }
}

void MyGraphicsView::setUnDirected()
{
    isDirected = false;
    for (auto &lineItem : lines) {
        lineItem->setUnDirected();
        lineItem->update();
    }
}

int MyGraphicsView::linesNumber() const
{
    return lines.size();
}

int MyGraphicsView::vexNumber() const
{
    return vexs.size();
}

bool MyGraphicsView::IsDirected() const
{
    return isDirected;
}

QVector<VexItem *> &MyGraphicsView::Vexs()
{
    return vexs;
}

QVector<LineItem *> &MyGraphicsView::Lines()
{
    return lines;
}

QQueue<DijkVex> MyGraphicsView::Dijkstra()
{
    QQueue<DijkVex> visitedQueue;
    if(currentSelectVex != nullptr) {
        //重置view上的每一个点
        for (auto &vexItem : vexs) {
            vexItem->setPre(-1);
            vexItem->setDist(-1);
            vexItem->setStart(-1);
        }

        //选择终点
        int end;
        SelectEnd selectEnd(this, this);
        selectEnd.exec();
        end = selectEnd.end();

        AdjMatrix adjMatrix(this);
        //选择终点，打印一条最短路径
        if (end != -1) {
            visitedQueue = adjMatrix.DijkstraPath(currentSelectVex->No(), end);
            playAnimation<DijkVex>(visitedQueue);
            for (auto &vex : visitedQueue) {
                vexs[vex.no]->setPre(vex.pre);
                vexs[vex.no]->setDist(vex.dist);
                vexs[vex.no]->setStart(vex.dijkstraStart);
            }
        }
    }
    return visitedQueue;
}

QQueue<DijkVex> MyGraphicsView::Floyd()
{
    QQueue<DijkVex> visitedQueue;
    if(currentSelectVex != nullptr) {
        //重置view上的每一个点
        for (auto &vexItem : vexs) {
            vexItem->setPre(-1);
            vexItem->setDist(-1);
            vexItem->setStart(-1);
        }

        //选择终点
        int end;
        SelectEnd selectEnd(this, this);
        selectEnd.exec();
        end = selectEnd.end();

        AdjMatrix adjMatrix(this);
        //选择终点，打印一条最短路径
        if (end != -1) {
            visitedQueue = adjMatrix.FloydPath(currentSelectVex->No(), end);
            if (!visitedQueue.empty()) {
                playAnimation<DijkVex>(visitedQueue);
            }
            for (auto &vex : visitedQueue) {
                vexs[vex.no]->setPre(vex.pre);
                vexs[vex.no]->setDist(vex.dist);
                vexs[vex.no]->setStart(vex.dijkstraStart);
            }
        }
    }
    return visitedQueue;
}

void MyGraphicsView::clearView()
{
    for (auto &vexItem : vexs) {
        delVex(vexItem);
    }
    for (auto &lineItem : lines) {
        delLine(lineItem);
    }
}

void MyGraphicsView::getVexFromMap(QVector<VexItem *> &vs)
{
    for (int i = 0; i < vs.size(); i++) {
        addVex(vs[i]);
    }
}

void MyGraphicsView::getLineFromMap(QVector<LineItem *> &ls)
{
    for (int i = 0; i < ls.size(); i++) {
        addLine(ls[i]);
    }
}

void MyGraphicsView::delBusVex(int vexNo, int busNo)
{
    for (auto & vex : vexs) {
        if (vex->No() == vexNo) {
            vector<int> &busNos = vex->getBusNos();
            for(auto iter = busNos.begin(); iter != busNos.end(); ) {
                if (*iter == busNo) {
                    busNos.erase(iter);
                    break;
                }
                else {
                    iter++;
                }
            }
            break;
        }
    }
}

void MyGraphicsView::getMainWindow(MainWindow *mw)
{
    this->mw = mw;
}

VexItem *MyGraphicsView::getCurrentVexItem()
{
    return currentSelectVex;
}






