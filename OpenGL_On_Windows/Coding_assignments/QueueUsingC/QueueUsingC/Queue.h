#pragma once
#include"List.h"

#define QUEUE_EMPTY 7
#define QUEUE_NOT_EMPTY 8
//typedef int DATA     -Part of List.h
//typedef int RESULT   -Part of List.h

typedef List Queue;

Queue* createQueue();
RESULT destroyQueue(Queue*);
RESULT enqueue(Queue*,DATA);
RESULT dequeue(Queue*, DATA*);
RESULT isQueueEmpty(Queue*);
RESULT examineQueueFront(Queue*, DATA*);
RESULT examineQueueTail(Queue*, DATA*);

//Queue dump for debugging purpose
RESULT queueDump(Queue*);
