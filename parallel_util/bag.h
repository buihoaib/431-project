#pragma once
#include "pennant.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#define NORMAL 20

using namespace std;

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
	this->init(NORMAL);
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
			bag2->backbone[i - 1] = this->backbone[i]->p_split();
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
		this->backbone[i]->p_union(vertices);
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