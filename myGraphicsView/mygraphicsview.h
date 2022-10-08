#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVector>
#include <QList>
#include <lineItem/lineitem.h>
#include <vexItem/vexitem.h>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include "mainWindow/mainwindow.h"

struct VexNode;
struct DijkVex;

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    MyGraphicsView(QWidget *parent = nullptr);
private:
    //打包一个scene，在构造函数中完成初始化
    QGraphicsScene *scene;
   //储存视图中的边和点
    QVector<VexItem *> vexs;
    QVector<LineItem *> lines;

    //当前鼠标点击选择的点,也就是可以操作的点,没选择则为空指针
    VexItem *currentSelectVex = nullptr;
    //当前鼠标点击选择的边,也就是可以操作的边,没选择则为空指针
    LineItem *currentSelectLine = nullptr;
    //正在生成的边
    LineItem *creatingLine = nullptr;

    //当前图是否有方向, 默认为无方向
    bool isDirected;

    //串口动画播放容器
    QSequentialAnimationGroup *AnimationGroup;

    //当前是否正在遍历图
    bool isVisiting;

    QVector<VexItem *> visitedVexs;
    QVector<LineItem *> visitedLines;

    MainWindow *mw;
public:
    //在容器和scene中添加边和点，显示在view中
    void addVex(VexItem *vex);
    void addLine(LineItem *line);

    //在容器和scene中移除边和点
    void delVex(VexItem *vex);
    void delLine(LineItem *line);

    //鼠标右键点击的位置创建一个点或边，左键选择一个点或边
    void mousePressEvent(QMouseEvent *event) override;
    //根据鼠标移动生成边
    void mouseMoveEvent(QMouseEvent *event) override;

    //删除选中的点或线
    void removeSelectItem();
    //设置是否有向
    void setDirected();
    void setUnDirected();

    int linesNumber() const;
    int vexNumber() const;

    bool IsDirected() const;

    template<typename T> static void deleteVectorItem(QVector<T *> &vector, T *delItem);

    QVector<VexItem *> &Vexs();
    QVector<LineItem *> &Lines();

    QQueue<DijkVex> Dijkstra();
    QQueue<DijkVex> Floyd();

    //清空视图
    void clearView();

    //播放遍历动画
    template<typename T>
    void playAnimation(QQueue<T> visitedQueue);

    void getVexFromMap(QVector<VexItem *> &vs);
    void getLineFromMap(QVector<LineItem *> &ls);

    void delBusVex(int vexNo, int busNo);

    void getMainWindow(MainWindow *mw);

    VexItem * getCurrentVexItem();

signals:
    void mouseClicked();
    void buildLOG(QString log);
    void delBus(int busNo);
};


template<typename T> void MyGraphicsView::deleteVectorItem(QVector<T *> &vector, T *delItem)
{
    if(vector.empty()) return;
    for (auto iter = vector.begin(); iter != vector.end(); ) {
        if (*iter == delItem) {
            vector.erase(iter);
            break;
        }
        else {
            iter++;
        }
    }
}

template<typename T>
void MyGraphicsView::playAnimation(QQueue<T> visitedQueue)
{
    //清空播放器
    AnimationGroup->clear();

    //开始遍历
    isVisiting = true;

    //将遍历第一个点的动画放入容器中
    T firstVex = visitedQueue.dequeue();
    QPropertyAnimation *animation = new QPropertyAnimation(vexs[firstVex.no], "isVisited", this);
    animation->setEndValue(true);
    AnimationGroup->addAnimation(animation);
    AnimationGroup->addPause(500);
    visitedVexs.push_back(vexs[firstVex.no]);


    while (!visitedQueue.empty()) {
        T vex = visitedQueue.dequeue();

        //将边遍历动画放入容器中
        for (auto &line : vexs[vex.no]->Lines()) {
            if ((line->getStartVex()->No() == vex.pre and line->getEndVex()->No() == vex.no)
                    or (line->getStartVex()->No() == vex.no and line->getEndVex()->No() == vex.pre)) {
                 QPropertyAnimation *animationl = new QPropertyAnimation(line, "isVisited", this);
                 animationl->setEndValue(true);
                 AnimationGroup->addAnimation(animationl);
                 AnimationGroup->addPause(500);
                 visitedLines.push_back(line);
            }
        }

        //将点遍历动画放入容器中
        QPropertyAnimation *animation = new QPropertyAnimation(vexs[vex.no], "isVisited", this);
        animation->setEndValue(true);
        AnimationGroup->addAnimation(animation);
        AnimationGroup->addPause(500);
        visitedVexs.push_back(vexs[vex.no]);
    }
    AnimationGroup->start();
}
#endif // MYGRAPHICSVIEW_H
