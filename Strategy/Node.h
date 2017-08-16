#ifndef NODE_H_
#define NODE_H_

struct Node {
    //变量
    enum Chessman { EMPTY = 0, PLAYER = 1, COMPUTER = 2 } _chessman;//上一执子者
    int _X, _Y;//上一落子点
    int _visitedNum;//节点访问次数
    int _expandableNum;//可扩展节点数
    int _profit;//节点收益
    int* _expandableNode;//可扩展列编号，一维数组
    Node** _children;//孩子，一维指针数组
    Node* _parent;//父母
    
    //构造函数，需传入上一落子点、上一执子者、父母
    Node(int X, int Y, Chessman chessman, Node* parent);
};

#endif
