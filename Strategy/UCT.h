#ifndef UCT_H_
#define UCT_H_

#include "Node.h"

class UCT {
private:
    enum { PLAYER_WIN = -1, COMPUTER_WIN = 1, TIE = 0, UNDETERMINED = 2 };//获胜、平局、不确定三种棋局状态
    Node* _root;//UCT的根节点
    int** _chessboard;//初始棋盘状态
    int* _top;//初始顶端状态
    int** _newChessboard;//新棋盘状态
    int* _newTop;//新顶端状态
    int _M, _N;//棋盘大小为M(row)*N(column)
    int _noX, _noY;//棋盘不可落子点为(noX,noY)
    
public:
    UCT(int M, int N, int noX, int noY, int** chessboard, int* top);//构造函数，需传入棋盘大小、不可落子点、棋盘状态和顶端状态
    ~UCT();//析构函数
    void Reset();//复原棋盘状态至初始状态
    void Init(Node* node);//初始化节点node
    void Clear(Node* node);//清空以node为根节点的子树
    bool IsExpandable(Node* node);//判断节点node是否可以扩展
    int IsTerminal(Node* node);//判断节点node是否到达终点(WIN或TIE)，同时返回对应enum类型
    Node::Chessman ChangeChessman(Node::Chessman chessman);//返回下一执子者
    Node* TreePolicy(Node* node);//返回按照搜索树策略对节点node扩展的节点
    Node* Expand(Node* node);//返回对节点node扩展的节点
    Node* BestChild(Node* node);//返回按照UCB1规则节点node最优的孩子
    int DefaultPolicy(Node* node);//返回按默认策略随机下到终点(WIN或TIE)节点node的收益值
    void BackUp(Node* node, int deltaProfit);//向上回溯更新收益值
    Node* Search();//返回给定棋盘状态和顶端状态进行搜索的最优结果
};

#endif
