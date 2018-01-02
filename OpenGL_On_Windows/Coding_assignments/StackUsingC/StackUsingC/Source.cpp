#include<Windows.h>
#include"Stack.h"

void testIsStackEmpty(Stack* stack) {
	if (isStackEmpty(stack) == STACK_EMPTY) {
		printf("testIsStackEmpty:Stack is empty\n");
	}
	else {
		printf("testIsStackEmpty:Stack is NOT empty\n");
	}
}
void testPop(Stack *stack, DATA* data) {
	RESULT result;
	if ((result = pop(stack, data)) == SUCCESS) {
		fprintf(stdout, "testPop:Popped %d\n", *data);
	}
	else if (result == STACK_EMPTY) {
		fprintf(stdout, "testPop:Stack is empty. Failed to popped\n");
	}
	else
		fprintf(stderr, "testPop:Failed to pop from the stack\n");
}
void printStackElements(Stack* stack) {
	fprintf(stdout, "Stack dump:");
	stackDump(stack);
}
int main(void) {
	DATA data;

	Stack* stack1 = createStack();
	//test stack empty
	testIsStackEmpty(stack1);

	//Push some elements into the stack
	for (int i = 10;i < 100;i = i + 10) {
		if (push(stack1, i) != SUCCESS) {
			fprintf(stderr,"Push:Failed to push %d on to the stack. Exiting\n", i);
			destroyStack(stack1);
			exit(FAIL);
		}
	}
	//Print stack element (debug info)
	

	//examine stack top
	if (examineStackTop(stack1, &data) == SUCCESS) {
		fprintf(stdout, "examineStackTop: stack top is:%d\n", data);
	}
	else
		fprintf(stderr, "examineStackTop: Failed to examine stack top\n");

	//examine and delete stack top
	if (examineAndDeleteStackTop(stack1, &data) == SUCCESS) {
		fprintf(stdout, "examineAndDeleteStackTop: stack top is:%d\n", data);
	}
	else
		fprintf(stderr, "examineAndDeleteStackTop: Failed to examine and delete stack top\n");
	
	//Print stack element (debug info)
	printStackElements(stack1);

	//pop stack top
	fprintf(stdout, "Popping 2 elements from stack..\n");
	testPop(stack1, &data);
	testPop(stack1, &data);

	//Print stack elements 
	printStackElements(stack1);

	//Pop all elements from stack
	while (pop(stack1, &data) == SUCCESS);
	
	fprintf(stdout, "Popped all elements from stack\n");

	//test stack empty
	testIsStackEmpty(stack1);

	//Add some elements again
	push(stack1, 11);
	push(stack1, 22);
	push(stack1, 33);
	
	//Print stack elements
	printStackElements(stack1);

	//destroy stack
	destroyStack(stack1);
	getchar();
}
