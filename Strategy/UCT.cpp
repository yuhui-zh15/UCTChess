#include "UCT.h"
#include "Judge.h"
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <cstdio>

const double COEFFICIENT = 0.8;//调整信心模型的参数
const double TIME_LIMIT = 2.0;//调整每步时间限制，单位为秒

//构造函数，需传入棋盘大小、不可落子点、棋盘状态和顶端状态
UCT::UCT(int M, int N, int noX, int noY, int** chessboard, int* top): _M(M), _N(N), _noX(noX), _noY(noY), _chessboard(chessboard), _top(top) {
    _newChessboard = new int*[_M];
    for (int i = 0; i < _M; i++) {
        _newChessboard[i] = new int[_N];
        for (int j = 0; j < _N; j++) {
            _newChessboard[i][j] = _chessboard[i][j];
        }
    }
    _newTop = new int[_N];
    for (int i = 0; i < _N; i++) {
        _newTop[i] = _top[i];
    }
}

//析构函数
UCT::~UCT() {
    Clear(_root);
    delete _root;
    for (int i = 0; i < _M; i++) {
        delete []_newChessboard[i];
    }
    delete []_newChessboard;
    delete []_newTop;
}

//复原棋盘状态至初始状态
void UCT::Reset() {
    for (int i = 0; i < _M; i++) {
        for (int j = 0; j < _N; j++) {
            _newChessboard[i][j] = _chessboard[i][j];
        }
    }
    for (int i = 0; i < _N; i++) {
        _newTop[i] = _top[i];
    }
}

//初始化节点node
void UCT::Init(Node* node) {
    node->_profit = 0;
    node->_visitedNum = 0;
    node->_expandableNum = 0;
    node->_expandableNode = new int[_N];
    node->_children = new Node*[_N];
    for (int i = 0; i < _N; i++) {
        node->_children[i] = NULL;
        if (_newTop[i] != 0) node->_expandableNode[node->_expandableNum++] = i;
    }
}

//清空以node为根节点的子树
void UCT::Clear(Node* node) {
    delete []node->_expandableNode;
    for (int i = 0; i < _N; i++) {
        if (node->_children[i]) {
            Clear(node->_children[i]);//递归清空孩子节点
            delete []node->_children[i];
        }
    }
    delete []node->_children;
}

//判断节点node是否可以扩展
bool UCT::IsExpandable(Node* node) {
    return node->_expandableNum > 0;
}

//判断节点node是否到达终点(WIN或TIE)，同时返回对应enum类型
int UCT::IsTerminal(Node* node) {
    if (node->_X == -1 && node->_Y == -1) return UNDETERMINED;//根节点单独处理
    if (node->_chessman == Node::COMPUTER && machineWin(node->_X, node->_Y, _M, _N, _newChessboard)) return COMPUTER_WIN;
    if (node->_chessman == Node::PLAYER && userWin(node->_X, node->_Y, _M, _N, _newChessboard)) return PLAYER_WIN;
    if (isTie(_N, _newTop)) return TIE;
    return UNDETERMINED;
}

//返回下一执子者
Node::Chessman UCT::ChangeChessman(Node::Chessman chessman) {
    if (chessman == Node::PLAYER) return Node::COMPUTER;
    if (chessman == Node::COMPUTER) return Node::PLAYER;
    return Node::EMPTY;
}

//返回按照搜索树策略对节点node扩展的节点
Node* UCT::TreePolicy(Node* node) {
    while (IsTerminal(node) == UNDETERMINED) {
        if (IsExpandable(node)) {
            return Expand(node);
        }
        else
            node = BestChild(node);
    }
    return node;
}

//返回对节点node扩展的节点
Node* UCT::Expand(Node* node) {
    int r = rand() % node->_expandableNum;//随机一列
    //向随机出的方向落子
    int newY = node->_expandableNode[r];
    int newX = --_newTop[newY];
    Node::Chessman chessman = ChangeChessman(node->_chessman);
    _newChessboard[newX][newY] = chessman;
    if (newY == _noY && newX == _noX + 1) --_newTop[newY];
    //建立孩子节点
    node->_children[newY] = new Node(newX, newY, chessman, node);
    Init(node->_children[newY]);
    std::swap(node->_expandableNode[r], node->_expandableNode[--node->_expandableNum]);
    return node->_children[newY];
}

//返回按照UCB1规则节点node最优的孩子
Node* UCT::BestChild(Node* node) {
    //初始化，注意把maxProfit初始化为最小值
    Node* best = NULL;
    int bestIndex = 0;
    double maxProfit = -RAND_MAX;
    for (int i = 0; i < _N; i++) {
        if (node->_children[i] == NULL) continue;
        //根据节点执子者判断正负
        int childProfit = (node->_chessman == Node::PLAYER? 1: -1) * node->_children[i]->_profit;
        int childVisitedNum = node->_children[i]->_visitedNum;
        //UCT计算方法
        double tempProfit = (double)childProfit / (double)childVisitedNum + sqrt(2.0 * log((double)node->_visitedNum) / (double)childVisitedNum) * COEFFICIENT;
        //更新最优孩子
        if (tempProfit > maxProfit) {
            maxProfit = tempProfit;
            best = node->_children[i];
            bestIndex = i;
        }
    }
    //向最优节点落子
    int newY = bestIndex;
    int newX = --_newTop[newY];
    Node::Chessman chessman = ChangeChessman(node->_chessman);
    _newChessboard[newX][newY] = chessman;
    if (newY == _noY && newX == _noX + 1) --_newTop[newY];
    return best;
}

//返回按默认策略随机下到终点(WIN或TIE)节点node的收益值
int UCT::DefaultPolicy(Node* node) {
    //复制原节点状态，新建节点
    Node* newNode = new Node(node->_X, node->_Y, node->_chessman, NULL);
    Init(newNode);
    //对该节点进行随机模拟至结束，计算收益
    int profit;
    while ((profit = IsTerminal(newNode)) == UNDETERMINED) {
        newNode->_chessman = ChangeChessman(newNode->_chessman);
        do {
            newNode->_Y = rand() % _N;
        } while (_newTop[newNode->_Y] == 0);
        newNode->_X = --_newTop[newNode->_Y];
        _newChessboard[newNode->_X][newNode->_Y] = newNode->_chessman;
        if (newNode->_Y == _noY && newNode->_X == _noX + 1) _newTop[newNode->_Y]--;
    }
    Clear(newNode);
    delete newNode;
    return profit;
}

//向上回溯更新收益值
void UCT::BackUp(Node* node, int deltaProfit) {
    while (node) {
        node->_visitedNum++;
        node->_profit += deltaProfit;
        node = node->_parent;
    }
}

//返回给定棋盘状态和顶端状态进行搜索的最优结果
Node* UCT::Search() {
    int startTime = clock(), endTime;//起止时间
    int searchNum = 0;//搜索次数
    //创建根节点
    _root = new Node(-1, -1, Node::PLAYER, NULL);
    Init(_root);
    //当时间没有结束，进行搜索并更新
    while (true) {
        if (searchNum % 5000 == 0 && ((endTime = clock()) - startTime) / (double)CLOCKS_PER_SEC > TIME_LIMIT) break;//每5000次搜索计一次时间
        Reset();//初始化棋盘为传入状态
        Node* node = TreePolicy(_root);
        int deltaProfit = DefaultPolicy(node);
        BackUp(node, deltaProfit);
        searchNum++;
    }
    //返回最优解
    Reset();
    return BestChild(_root);
}
