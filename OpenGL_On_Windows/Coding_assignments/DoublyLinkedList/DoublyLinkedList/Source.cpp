#include<Windows.h>
#include"List.h"

int main(void) {
	DATA data;
	List *list1 = createList();
	//test add
	for (int i = 10;i < 100;i = i + 10) {
		addNode(list1, i);
	}
	displayList(list1);
	printf("List in reverse order:");
	displayListInReverseOrder(list1);
	if (getFirst(list1, &data) == SUCCESS) {
		printf("First:%d \n", data);
	}
	if (getLast(list1, &data) == SUCCESS) {
		printf("Last:%d \n", data);
	}

	//delete desired node
	printf("Deleting 30\n");
	deleteNode(list1, 30);
	displayList(list1);
	printf("List in reverse order:");
	displayListInReverseOrder(list1);

	//delete before
	if (deleteBefore(list1, 60) == SUCCESS) {
		printf("Element before 60 deleted\n");
		displayList(list1);
		printf("List in reverse order:");
		displayListInReverseOrder(list1);
	}

	//delete after
	if (deleteAfter(list1, 70) == SUCCESS) {
		printf("Element after 70 deleted\n");
		displayList(list1);
		printf("List in reverse order:");
		displayListInReverseOrder(list1);
	}
	if (destroyList(list1) == SUCCESS) {
		printf("List destroyed successfully\n");
	}
	else
		printf("Failed to destory the list\n");
	getchar();
}
