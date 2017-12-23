#include"SinglyLinkedList.h"

Node* getNode(DATA data) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->data = data;
	node->next = NULL;
	return node;
}

List*   createList() {
	List *list = (List*)malloc(sizeof(List));
	list->head = getNode(0);
	return list;
}

RESULT  addNode(List *list, DATA data) {
	if (list == NULL)
		return FAIL;
	Node* head = list->head;
	Node *node = head;
	while (node->next != NULL) {
		node = node->next;
	}
	node->next = getNode(data);
	return SUCCESS;
}

RESULT  deleteNode(List* list, DATA data) {
	if (list == NULL)
		return FAIL;
	Node* head = list->head;
	Node *node = head->next;
	Node *prev=head;
	while (node != NULL) {
		if (node->data == data) {
			prev->next = node->next;
			free(node);
			return SUCCESS;
		}
		prev = node;
		node = node->next;
	}
	return DATA_NOT_FOUND;
}
RESULT  deleteFirst(List* list) {
	if (list == NULL)
		return FAIL;
	Node* node = list->head->next;
	if (node != NULL) {
		list->head->next = node->next;
		free(node);
		return SUCCESS;
	}
	return LIST_EMPTY;
}
RESULT  deleteLast(List* list) {
	if (list == NULL)
		return FAIL;
	Node* node = list->head;
	Node *prev = NULL;
	
	if (node->next == NULL)
		return LIST_EMPTY;

	while (node ->next != NULL) {
		prev = node;
		node = node->next;
	}
	free(node);
	prev->next = NULL;
	return SUCCESS;
}
RESULT  getLast(List* list, DATA* data_out) {
	if (list == NULL)
		return FAIL;

	Node* node = list->head;

	if (node->next == NULL)
		return LIST_EMPTY;
	while (node->next != NULL) {
		node = node->next;
	}
	*data_out = node->data;
	return SUCCESS;
}
RESULT  getFirst(List* list, DATA* data_out) {
	if (list == NULL)
		return FAIL;

	Node* node = list->head;

	if (node->next == NULL)
		return LIST_EMPTY;

	*data_out = node->next->data;
	return SUCCESS;
}
RESULT  deleteBefore(List* list, DATA data) {
	if (list == NULL)
		return FAIL;

	Node* head = list->head;

	if (head->next == NULL)
		return LIST_EMPTY;
	
	if (head->next->data == data)
		return FAIL;
	Node *node = head->next;
	Node *prev=head;
	
	while (node->next != NULL) {
		if (node->next->data == data)
		{
			prev->next = node->next;
			free(node);
			return SUCCESS;
		}
		prev = node;
		node = node->next;
	}
	return DATA_NOT_FOUND;
}
RESULT  deleteAfter(List* list, DATA data) {
	if (list == NULL)
		return FAIL;

	Node* head = list->head;

	if (head->next == NULL)
		return LIST_EMPTY;

	Node *node = head->next;
	Node *temp;
	while (node->next != NULL) {
		if (node->data == data)
		{
			temp = node->next;
			node->next = node->next->next;
			free(temp);
			return SUCCESS;
		}
		node = node->next;
	}
	return DATA_NOT_FOUND;
}

RESULT displayList(List* list) {
	if (list == NULL)
		return FAIL;

	Node* head = list->head;

	if (head->next == NULL)
		return LIST_EMPTY;
	Node* node = head->next;
	printf("Displaying list: ");
	while (node->next != NULL) {
		printf("%d->", node->data);
		node = node->next;
	}
	printf("%d\n", node->data);
	return SUCCESS;
}

