#include "bus.h"
#include <QDebug>
#include <QString>

vector<int> split(const string &str, const string &pattern)
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

Bus::Bus(int no, const std::string &vexs) : no(no)
{
    vector<int> res = split(vexs, " ");
    busVexs.reserve(res.size());
    for (size_t i = 0; i < res.size(); i++) {
        if (i == 0) {
            busVexs.emplace_back(BusVex{res[i], res[i]});
        }
        else {
            busVexs.emplace_back(BusVex{res[i], res[i - 1]});
        }
    }
}

Bus::Bus(int no) : no(no)
{

}

Bus::Bus(int no, const vector<BusVex> &vexs) : no(no), busVexs(vexs)
{

}

vector<BusVex> &Bus::getVexs()
{
    return busVexs;
}

void Bus::changeVexs(const vector<BusVex> &vexs)
{
    busVexs.clear();
    busVexs = vexs;
}

int Bus::getNo()
{
    return no;
}
