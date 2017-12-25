#include"List.h"

Node* getNode(DATA data) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->data = data;
	node->next = NULL;
	node->prev = NULL;
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
	Node *newNode = getNode(data);
	node->next = newNode;
	newNode->prev = node;
	return SUCCESS;
}

RESULT  deleteNode(List* list, DATA data) {
	if (list == NULL)
		return FAIL;
	Node* head = list->head;
	Node *node = head->next;
	Node *prev = head;
	while (node != NULL) {
		if (node->data == data) {
			node->next->prev = prev;
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
		node->next->prev = list->head;
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

	while (node->next != NULL) {
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
	Node *prev = head;

	while (node->next != NULL) {
		if (node->next->data == data)
		{
			node->next->prev = prev;
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
			node->next->next->prev = node;
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

RESULT displayListInReverseOrder(List* list) {
	if (list == NULL)
		return FAIL;

	Node* head = list->head;

	if (head->next == NULL) {
		printf("empty\n");
		return LIST_EMPTY;
	}
	Node* node = head->next;
	while (node->next != NULL) {
		node = node->next;
	}
	while (node->prev != head) {
		printf("%d->", node->data);
		node = node->prev;
	}
	printf("%d\n", node->data);
	return SUCCESS;
}
RESULT destroyList(List* list) {
	Node* head = list->head;
	if (list == NULL) {
		return FAIL;
	}
	if (head->next == NULL) {
		printf("List is empty\n");
		return LIST_EMPTY;
	}
	Node* node = head->next;
	while (node->next != NULL) {
		Node *temp = node;
		node = node->next;
		free(temp);
	}
	return SUCCESS;
}

