#pragma once
#include<Windows.h>
#include<stdio.h>
#define FAIL           0
#define SUCCESS        1
#define DATA_NOT_FOUND 2
#define LIST_EMPTY     3

typedef int RESULT;
typedef int DATA;

typedef struct Node {
	DATA data;
	struct Node *next;
}Node;

typedef struct List {
	Node *head;
}List;

List*   createList();
RESULT  displayList(List*); 
RESULT  addNode(List *, DATA data);
RESULT  getLast(List*, DATA* data_out);
RESULT  getFirst(List*, DATA* data_out);
RESULT  deleteNode(List*, DATA data);
RESULT  deleteFirst(List*);
RESULT  deleteLast(List*);
RESULT  deleteBefore(List*, DATA);
RESULT  deleteAfter(List*, DATA);

