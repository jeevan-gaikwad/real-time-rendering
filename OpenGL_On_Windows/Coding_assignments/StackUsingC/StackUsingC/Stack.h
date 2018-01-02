#pragma once
#include"List.h"

#define STACK_EMPTY 5
#define STACK_NOT_EMPTY 6
//typedef int DATA     -Part of List.h
//typedef int RESULT   -Part of List.h

typedef List Stack;

Stack* createStack();
RESULT destroyStack(Stack*);
RESULT push(Stack*,DATA);
RESULT pop(Stack*, DATA*);
RESULT isStackEmpty(Stack*);
RESULT examineStackTop(Stack*, DATA*);
RESULT examineAndDeleteStackTop(Stack*, DATA*);

//Stack dump for debugging purpose
RESULT stackDump(Stack*);
