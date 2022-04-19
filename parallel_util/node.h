#pragma once
#include "../core/graph.h"


class Node
{
public:

	uintV vertex;
	Node* left;
	Node* right;
    Node(int vertex){
        this->vertex = vertex;
	    this->left = NULL;
	    this->right = NULL;
    };
	~Node(){};
};