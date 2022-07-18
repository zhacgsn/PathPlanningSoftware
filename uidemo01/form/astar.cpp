#include "astar.h"

int _MAP[30][20]={{0}};

Astar::~Astar()
{

}

void Astar::search( Node* _startPos, Node* _endPos )
{
//    int count = 0;

    if (_startPos->x < 0 || _startPos->x > row || _startPos->y < 0 || _startPos->y >col ||
        _endPos->x < 0 || _endPos->x > row || _endPos->y < 0 || _endPos->y > col)
        return ;

    Node* current;
    start->giveCordinate(_startPos->x,_startPos->y);
    end->giveCordinate(_endPos->x,_endPos->y);

    open.push_back(start);

    while(open.size() > 0)
    {
        current = open[0];
//        找到终点
        if (current->x == end->x && current->y == end->y)
        {
//            qDebug() << "路径长度：" << current->f;
            //printMap();
            path=current;
            open.clear();
            closed.clear();
            break;
        }
        nextStep(current);
        closed.push_back(current);

//         标记扩展结点
//        _MAP[current->x][current->y] = 2;
//        count++;

        open.erase(open.begin());
//        按f升序排序
        sort(open.begin(),open.end(),compare);
    }
//    qDebug() << "扩展结点数量：" << count;

//    qDebug() << "对角距离";
//    qDebug() << "切比雪夫";
//    qDebug() << "曼哈顿";
//    qDebug() << "欧几里得";
}

void Astar::checkPoint( int x,int y,Node* father,double g)
{
    if (x < 0 || x > row || y < 0 || y > col)
        return;
//    结点为障碍
    if (this->unWalk(x,y))
        return;
    if (isContain(&closed,x,y) != -1)
        return;
    int index;

    if ((index = isContain(&open,x,y)) != -1)
    {
        Node *point = open[index];

        if (point->g > father->g + g)
        {
            point->father = father;
            point->g = father->g + g;
            point->f = point->g + point->h;
        }
    }
    else
    {
        Node * point = new Node(x,y,father);
        calculateGHF(point,end,g);
        open.push_back(point);
    }
}

void Astar::nextStep( Node* current )
{
    checkPoint(current->x - 1, current->y, current, D1);//左
    checkPoint(current->x + 1, current->y, current, D1);//右
    checkPoint(current->x, current->y + 1, current, D1);//上
    checkPoint(current->x, current->y - 1, current, D1);//下
//    对角方向
    checkPoint(current->x - 1, current->y + 1, current, D2);//左上
    checkPoint(current->x - 1, current->y - 1, current, D2);//左下
    checkPoint(current->x + 1, current->y - 1, current, D2);//右下
    checkPoint(current->x + 1, current->y + 1, current, D2);//右上
}

int Astar::isContain(vector<Node*>* Nodelist, int x,int y )
{
    for (int i = 0;i < Nodelist->size();i++)
    {
        if (Nodelist->at(i)->x  == x && Nodelist->at(i)->y == y)
        {
            return i;
        }
    }
    return -1;
}

void Astar::calculateGHF( Node* sNode,Node* eNode,double g)
{
    int dx = abs(sNode->x - eNode->x);
    int dy = abs(sNode->y - eNode->y);
//    Octile distance
    double h = D1 * (dx + dy) + (D2 - 2 * D1) * min(dx, dy);

//    切比雪夫距离
//    double h = dx + dy - min(dx, dy);

//    曼哈顿距离
//    double h = dx + dy;

//    欧几里得距离
//    double h = qSqrt(dx * dx + dy * dy);

    double currentg = sNode->father->g + g;
    double f = currentg + (1 + p) * h;

    sNode->f = f;
    sNode->h = h;
    sNode->g = currentg;
}

bool Astar::compare( Node* n1,Node* n2 )
{
    //printf("%d,%d",n1->f,n2->f);
    return n1->f < n2->f;
}

bool Astar::unWalk( int x,int y)
{
    if (_MAP[x][y] == 1)
        return true;
    return false;
}
/*
void Astar::printPath( Node* current )
{
    if (current->father != NULL)
        printPath(current->father);
    printf("(%d,%d)",current->x,current->y);
}

void Astar::printMap()
{
    for(int i=0;i<=row;i++){
        for(int j=0;j<=col;j++){
            printf("%d ",map[i][j]);
        }
        printf("\n");
    }
}
*/
void Node::giveCordinate(int _x, int _y)
{
    this->x=_x;
    this->y=_y;
}
