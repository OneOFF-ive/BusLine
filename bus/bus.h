#ifndef BUS_H
#define BUS_H

#include <string>
#include <vector>

using std::string;
using std::vector;

struct BusVex {
    int no;
    int pre;
};

class Bus
{
private:
    vector<BusVex> busVexs;
    int no;
public:
    Bus() = default;
    Bus(int no, const string &vexs);
    Bus(int no);
    Bus(int no, const vector<BusVex> &vexs);
    vector<BusVex> & getVexs();
    void changeVexs(const vector<BusVex> &vexs);
    int getNo();
};

#endif // BUS_H
