#ifndef ASTAR_H
#define ASTAR_H
#include <iostream>
#include <queue>
#include <vector>
#include <stack>
#include <algorithm>
#include <string.h>
#include <QPoint>
#include <qDebug>
#include <QtMath>

using namespace std;

class Node
{   public:
    int x,y;
    double g; //起始点到当前点结点最优代价
    double h;//当前节点到目标节点最佳路径的估计代价
    double f;//估计值
    Node* father;

    Node()
    {
    this->father = nullptr;
    }

    Node(int x,int y)
    {
        this->x = x;
        this->y = y ;
        this->g = 0;
        this->h = 0;
        this->f = 0;
        this->father = nullptr;
    }
    Node(int x,int y,Node* father)
    {
        this->x = x;
        this->y = y ;
        this->g = 0;
        this->h = 0;
        this->f = 0;
        this->father = father;
    }
    void giveCordinate(int _x,int _y);
};
class Astar{
    public:
        Astar()
        {
            start=new Node();
            end=new Node();
        }

        ~Astar();
        void search(Node* _startPos,Node* _endPos);
        void checkPoint(int x,int y,Node* father,double g);
        void nextStep(Node* currentPoint);
        int isContain(vector<Node*>* Nodelist ,int x,int y);
        void calculateGHF(Node* sNode,Node* eNode,double g);
        static bool compare(Node* n1,Node* n2);
        bool unWalk(int x,int y);
//        待检查结点，初始时只有起点
        vector<Node*> open;
//        已检查结点
        vector<Node*> closed;
        Node *start;
        Node *end;
        Node *path;
        constexpr static const double D1 = 1;// 正方向消耗
        constexpr static const double D2 = 1.414;//斜方向的消耗
        static const int row =30;
        static const int col =20;
//        constexpr static const double p = 0.03;
        constexpr static const double p = 0.1;
};

#endif // ASTAR_H
