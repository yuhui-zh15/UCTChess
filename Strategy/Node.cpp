#include "Node.h"

//构造函数，需传入上一落子点、上一执子者、父母
Node::Node(int X, int Y, Chessman chessman, Node* parent):
_X(X), _Y(Y), _chessman(chessman), _parent(parent) { }
