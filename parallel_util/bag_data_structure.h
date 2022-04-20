//Reference and idea for Bag data structure: https://github.com/nducthang/BFSParallel
#pragma once
#include <math.h>
#include <algorithm>
#include <iostream>
#include <stack>
#include "../core/graph.h"


#define DEFAULT_SIZE 20

using namespace std;


class Node
{
public:

	uintV vertex;
	Node* left_node;
	Node* right_node;
    Node(int vertex){
        this->vertex = vertex;
		this->right_node = NULL;
	    this->left_node = NULL;
    };
	~Node(){};
};


class Pennant
{
public:

	Node *root;
	Pennant(Node* root);
	~Pennant();
    Pennant* split_pennant();
	void union_pennant(Pennant* y);
	static void FA(Pennant* x, Pennant* y, Pennant* carry);

};

Pennant::Pennant(Node* root){
	this->root = root;
}


Pennant::~Pennant(){
	if (this->root != NULL)
	{
		std::stack<Node *> vertices_tree;
		vertices_tree.push(this->root);
		while (!vertices_tree.empty())
		{
			Node *curr_vertext = vertices_tree.top();
			vertices_tree.pop();
			if (curr_vertext->right_node != NULL){
				vertices_tree.push(curr_vertext->right_node);
			}
			if (curr_vertext->left_node != NULL){
				vertices_tree.push(curr_vertext->left_node);
			}
			delete curr_vertext;
		}
	}
}

void Pennant::union_pennant(Pennant* y){
	if (y == NULL) {
        return;
    }
	y->root->right_node= this->root->left_node;
	this->root->left_node = y->root;
	//Set to null to prevent deleting this->root
	y->root = NULL;
	delete y;
	y = NULL;
}

//Split pennant into two same size pennants
Pennant* Pennant::split_pennant(){
	if (this->root->left_node == NULL){
		return NULL;
	}
	Pennant *y = new Pennant(this->root->left_node);
	this->root->left_node = y->root->right_node;
	y->root->right_node= NULL;
	return y;
}

//Based on illustration table on page 5 of A Work-Efficient Parallel Breadth-First Search Algorithm (or How to Cope with the Nondeterminism of Reducers) by Charles E. Leiserson & Tao B. Schardl
void Pennant::FA(Pennant* x, Pennant* y, Pennant* z){
	if (x == NULL && y == NULL && z == NULL) {
		return;
	} else if (x != NULL && y == NULL && z == NULL) {
		return;
	} else if (x == NULL && y != NULL && z == NULL) {
		x = y;
		y = NULL;
		return;
	} else if (x == NULL && y == NULL && z != NULL) {
		x = z;
		z = NULL;
		return;
	} else if (x != NULL && y != NULL && z == NULL) {
		x->union_pennant(y);
		z = x;
		x = NULL;
		return;
	} else if (x != NULL && y == NULL && z != NULL) {
		x->union_pennant(z);
		z = x;
		x = NULL;
		return;
	} else if (x == NULL && y != NULL && z != NULL) {
		y->union_pennant(z);
		z = y;
		y = NULL;
		return;
	} else if (x != NULL && y != NULL && z != NULL) {
		z->union_pennant(y);
		return;
	}
}
class Bag
{
public:
	Pennant** backbone; 
	int max_index; 
	int r;

	Bag();
	Bag(int r);
	~Bag();
	void clear();
	void init(int r);
	void insert_vertex(int x);
	bool empty();
	int size();
	Bag* split();
	void insert(Pennant *&vertices);
	void print();
	void merge(Bag* y);

};

Bag::Bag()
{
	this->init(DEFAULT_SIZE);
}
Bag::Bag(int r)
{
	this->init(r);
}
void Bag::init(int r)
{
	this->backbone = new Pennant*[r]();
	this->r = r;
	this->max_index = -1;
}
//void Bag::copy(Bag* y)
//{
//	this->backbone = y->backbone;
//	this->r = y->r;
//	this->largest_nonempty_index = y->largest_nonempty_index;
//}
void Bag::clear() {
	for (int i = 0; i <= this->max_index; i++) {
		delete this->backbone[i];
		this->backbone[i] = NULL;
	}

	this->max_index = -1;
}
Bag::~Bag()
{
	this->clear();
	delete[] this->backbone;
}

void Bag::insert_vertex(int x)
{
	Node* vertex = new Node(x);
	Pennant *vertices = new Pennant(vertex);
	this->insert(vertices);
}

bool Bag::empty()
{
	return (this->max_index < 0);
}

int Bag::size()
{
	int sum = 0;
	for (int i = 0; i <= this->max_index; i++)
	{
		if (this->backbone[i] != NULL)
		{
			sum += pow(2, i);
		}
	}
	return sum;
}
Bag* Bag::split()
{
	Bag* bag2 = new Bag(this->r);
	Pennant* first_element = this->backbone[0];
	this->backbone[0] = NULL;
	for (int i = 1; i <= this->max_index; i++)
	{
		if (this->backbone[i] != NULL)
		{
			bag2->backbone[i - 1] = this->backbone[i]->split_pennant();
			this->backbone[i - 1] = this->backbone[i];
			this->backbone[i] = NULL;
		}
	}
	if (this->max_index != 0)
	{
		this->max_index = this->max_index - 1;
		bag2->max_index = this->max_index;
	}
	if (first_element != NULL)
	{
		this->insert(first_element);
	}
	return bag2;
}

void Bag::insert(Pennant *&vertices)
{
	int i = 0;
	while (this->backbone[i]!=NULL)
	{
		this->backbone[i]->union_pennant(vertices);
		vertices = this->backbone[i];
		this->backbone[i] = NULL;
		i++;
	}
	this->backbone[i] = vertices;
	if (i > max_index)
	{
		this->max_index = i;
	}
}

void Bag::print()
{
	cout << "Bag: { ";
	for (int i = 0; i < this->r; i++)
	{
		cout << (this->backbone[i] != NULL)*pow(2, i) << " ";
	}
	cout << "}" << endl;
}

void Bag::merge(Bag* y)
{
	int max_temp_index = max(y->max_index, this->max_index);
	Pennant *carry = NULL;

	for (int i = 0; i <= max_temp_index; i++) {
		Pennant::FA(this->backbone[i], y->backbone[i], carry);
	}

	if (carry != NULL) {
		max_temp_index += 1;
		this->backbone[max_temp_index] = carry;
	}

	this->max_index = max_temp_index;
}

