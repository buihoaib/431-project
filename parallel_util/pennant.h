#pragma once
#include "node.h"
#include <iostream>
#include <stack>


class Pennant
{
public:


	Node *root;
	Pennant(Node* root);
	~Pennant();
    Pennant* p_split();
	void p_union(Pennant* &y);
	static void FA(Pennant* &x, Pennant* &y, Pennant* &carry);

};

Pennant::Pennant(Node* root)
{
	this->root = root;
}


Pennant::~Pennant()
{
	if (this->root != NULL)
	{
		std::stack<Node *> nodes;
		nodes.push(this->root);
		while (nodes.size()>0)
		{
			Node *current = nodes.top();
			nodes.pop();
			if (current->left != NULL)
			{
				nodes.push(current->left);
			}
			if (current->right != NULL)
			{
				nodes.push(current->right);
			}
			delete current;
		}
	}
}

void Pennant::p_union(Pennant* &y)
{
	if (y == NULL) 
    {
        return;
    }
	y->root->right = this->root->left;
	this->root->left = y->root;
	y->root = NULL;
	delete y;
	y = NULL;
}

Pennant* Pennant::p_split()
{
	if (this->root->left == NULL)
	{
		return NULL;
	}
	Pennant *y = new Pennant(this->root->left);
	this->root->left = y->root->right;
	y->root->right = NULL;
	return y;
}

//Based on illustration table on page 5 of A Work-Efficient Parallel Breadth-First Search Algorithm (or How to Cope with the Nondeterminism of Reducers) by Charles E. Leiserson & Tao B. Schardl
void Pennant::FA(Pennant* &x, Pennant* &y, Pennant* &z)
{
	if (x == NULL && y == NULL && z == NULL) {
		return;
	}
	if (x != NULL && y == NULL && z == NULL) {
		return;
	}
	if (x == NULL && y != NULL && z == NULL) {
		x = y;
		y = NULL;
		return;
	}
	if (x == NULL && y == NULL && z != NULL) {
		x = z;
		z = NULL;
		return;
	}
	if (x != NULL && y != NULL && z == NULL) {
		x->p_union(y);
		z = x;
		x = NULL;
		return;
	}
	if (x != NULL && y == NULL && z != NULL) {
		x->p_union(z);
		z = x;
		x = NULL;
		return;
	}
	if (x == NULL && y != NULL && z != NULL) {
		y->p_union(z);
		z = y;
		y = NULL;
		return;
	}
	if (x != NULL && y != NULL && z != NULL) {
		z->p_union(y);
		return;
	}
}