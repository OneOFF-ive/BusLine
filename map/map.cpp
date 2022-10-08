#include "map.h"
#include <fstream>
#include <iostream>


using std::ofstream;
using std::ifstream;
using std::ios;

Map::Map()
{

}

Map::Map(QVector<VexItem *> v, QVector<LineItem *> l, vector<Bus> b, MyGraphicsView *mv) :
    vexs(v), lines(l), busList(b), mv(mv)
{

}

void Map::save(const string &path)
{
    ofstream os(path, ios::out);
    for (int i = 0; i < vexs.size(); i++) {
        os<< "p" << ":" << vexs[i]->No() <<":" << vexs[i]->getTag().toStdString() << ":" << vexs[i]->pos().x() << ":" << vexs[i]->pos().y() << '\n';
    }
    for (int i = 0; i < lines.size(); i++) {
        os<< "l" << ":" << lines[i]->No() <<":" << lines[i]->getTag().toStdString() << ":" << lines[i]->getWeight() << ":" << lines[i]->getStartVex()->No() << ":" << lines[i]->getEndVex()->No() << '\n';
    }
    for (size_t i = 0; i < busList.size(); i++) {
        os << "b" << ":" << busList[i].getNo() << ":";
        for (size_t j = 0; j < busList[i].getVexs().size(); j++) {
            os << busList[i].getVexs()[j].no << " ";
        }
        os << '\n';
    }
    os.close();


}

void Map::read(const string &path, QVector<VexItem *> &v, QVector<LineItem *> &l, vector<Bus> &b, MainWindow *mw)
{
    ifstream is(path, ios::in);
    char s[100];
    while (is.getline(s, 100)) {
        vector<string> res = mySplit(string(s), ":");
        if(res[0] == "p") {
            int vex_no = stoi(res[1]);
            QString vex_tag = QString::fromStdString(res[2]);
            QPointF point(stod(res[3]), stod(res[4]));
            VexItem *vex = new VexItem(point);
            vex->setNo(vex_no);
            vex->setTag(vex_tag);
            v.emplace_back(vex);
        }
        else if (res[0] == "l") {
            int line_no = stoi(res[1]);
            QString line_tag = QString::fromStdString(res[2]);
            int line_weight = stoi(res[3]);
            int line_start_no = stoi(res[4]);
            int line_end_no = stoi(res[5]);
            LineItem *line = new LineItem(v[line_start_no], v[line_end_no], false);
            line->setWeight(line_weight);
            line->setNo(line_no);
            line->setTag(line_tag);
            v[line_start_no]->addLine(line);
            v[line_end_no]->addLine(line);
            l.emplace_back(line);
        }
        else {
            string bus_no = res[1];
            string bus_vexs_s = res[2];
            b.emplace_back(Bus{stoi(bus_no), bus_vexs_s});
        }
    }
}

vector<std::string> Map::mySplit(const std::string &str, const std::string &pattern)
{
    vector<string> res;
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
            res.push_back(temp);
        }
        //去掉已分割的字符串,在剩下的字符串中进行分割
        strs = strs.substr(pos+1, strs.size());
        pos = strs.find(pattern);
    }

    return res;
}



