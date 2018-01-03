#include<Windows.h>
#include"Queue.h"
void displayChoiceMenu() {
	printf(" \
			1) Create empty queue \n \
			2) Enqueue \n \
			3) Dequeue \n \
			4) Print Q front and tail \n \
			5) Print Q elements\n \
			6) Exit\n \
			Enter your choice:");
}
int main(void) {
	Queue* queue = NULL;
	int choice=0;
	DATA data;
	while (choice != 6) {
		displayChoiceMenu();
		scanf_s("%d", &choice);
		switch (choice)
		{
		case 1: queue = createQueue();
			printf("Empty Q is created\n");
			break;
		case 2: 
			if (queue != NULL) {
				printf("Enter element to be enqueue:");
				scanf_s("%d", &data);
				enqueue(queue, data);
				queueDump(queue);
			}
			else
				printf("Create queue first.\n");
			break;
		case 3:
			if (queue != NULL) {				
				dequeue(queue, &data);
				printf("Dequeued:%d\n", data);
			}
			else
				printf("Create queue first.\n");
			//break; //let it fall through to display new front
		case 4:
			if (queue != NULL) {
				int front, tail;
				examineQueueFront(queue,&front);
				printf("Queue Front=%d\n", front);				
				data = examineQueueTail(queue,&tail);
				printf("Queue Tail=%d\n", tail);
			}
			else
				printf("Create queue first.\n");
			break;
		case 5:
			if (queue != NULL) {
				queueDump(queue);
			}
			else
				printf("Create queue first.\n");
			break;
		case 6: //exit
			break;
		default:
			printf("Wrong choice. Try again.\n");
			break;
		}
	}
	printf("Thank you.\n Press any key to exit.");
	getchar();
	getchar();
}