#include"Queue.h"

Queue* createQueue() {
	Queue* queue =(Queue*) createList();
	return queue;
}
RESULT destroyQueue(Queue* queue) {
	return destroyList(queue);
}
RESULT enqueue(Queue* queue, DATA data) {	
	return addNode(queue, data);
}
RESULT dequeue(Queue* queue, DATA* data) {
	RESULT result;
	if ((result=examineQueueFront(queue,data)) == SUCCESS)
		return deleteFirst(queue);
	else 
		return result;
}
RESULT isQueueEmpty(Queue* queue) {
	RESULT result;
	DATA data;
	result = getFirst(queue, &data);
	if (result == LIST_EMPTY) {
		return QUEUE_EMPTY;
	}
	return QUEUE_NOT_EMPTY;
}
RESULT examineQueueFront(Queue* queue, DATA* data) {
	RESULT result;
	if ((result = getFirst(queue, data)) == SUCCESS)
		return result;
	else if (result == LIST_EMPTY)
		return QUEUE_EMPTY;
}
RESULT examineQueueTail(Queue* queue, DATA* data) {
	RESULT result;
	if ((result = getLast(queue, data)) == SUCCESS)
		return result;
	else if (result == LIST_EMPTY)
		return QUEUE_EMPTY;
}

//Queue dump for debugging purpose
RESULT queueDump(Queue* queue) {
	Node* node = queue->head;
	if (isQueueEmpty(queue) == QUEUE_EMPTY) {
		return QUEUE_EMPTY;
	}
	printf("Q data : ");
	return displayList(queue);
}

