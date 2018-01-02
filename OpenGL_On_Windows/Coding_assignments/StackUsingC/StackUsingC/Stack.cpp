#include"Stack.h"

Stack* createStack() {
	List *list = createList();
	return list;
}
RESULT destroyStack(Stack* stack) {
	if (stack == NULL)
		return FAIL;
	//stack is list so call destroyList()
	return destroyList(stack);
}
RESULT push(Stack* stack, DATA data) {
	if (stack == NULL)
		return FAIL;
	return addNode(stack, data);
}
RESULT pop(Stack* stack, DATA* data) {
	if (stack == NULL)
		return FAIL;
	getLast(stack, data);
	return deleteLast(stack);
}
RESULT isStackEmpty(Stack* stack) {
	DATA temp;
	
	if (stack == NULL)
		return FAIL;
	if (getFirst(stack, &temp) == LIST_EMPTY) {
		return STACK_EMPTY;
	}else
		return STACK_NOT_EMPTY;
}
RESULT examineStackTop(Stack* stack, DATA* data) {
	RESULT result;
	if (stack == NULL)
		return FAIL;

	if ((result=getLast(stack, data)) == SUCCESS) {
		return SUCCESS;
	}
	else
		return result;

}
RESULT examineAndDeleteStackTop(Stack* stack, DATA* data) {
	RESULT result=FAIL;
	result=examineStackTop(stack, data);
	if (result == SUCCESS) {
		deleteLast(stack);
	}

	return result;
}
RESULT stackDump(Stack* stack) {
	RESULT result;
	result= displayListInReverseOrder(stack);
	if (result != SUCCESS)
		return FAIL;
	else
		return SUCCESS;
}