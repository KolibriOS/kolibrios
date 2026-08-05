#include <stdlib.h>
#include "ulist.h"

Ulist* ulist_create() {
    Ulist* list = (Ulist*)malloc(sizeof(Ulist));
    if (list == NULL) {
        exit(-1);
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

void ulist_destroy(Ulist* list) {
    Node* current = list->head;
    Node* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

void ulist_push_front(Ulist* list, void* data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        exit(-1);
    }
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = list->head;

    if (list->head != NULL) {
        list->head->prev = new_node;
    }

    list->head = new_node;

    if (list->tail == NULL) {
        list->tail = new_node;
    }

    list->size++;
}

void ulist_push_back(Ulist* list, void* data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        exit(-1);
    }
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = list->tail;

    if (list->tail != NULL) {
        list->tail->next = new_node;
    }

    list->tail = new_node;

    if (list->head == NULL) {
        list->head = new_node;
    }

    list->size++;
}

void ulist_remove(Ulist* list, Node* node) {
    if (list == NULL || node == NULL) {
        return;
    }
    // Update previous node's next pointer
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    else {
        // If the node is the head, update the head pointer
        list->head = node->next;
    }
    // Update next node's previous pointer
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    else {
        // If the node is the tail, update the tail pointer
        list->tail = node->prev;
    }
    // Free the memory occupied by the node
    free(node);
    list->size--;
}

void ulist_remove_front(Ulist* list) {
    if (list->head == NULL) {
        return;
    }

    Node* node_to_remove = list->head;
    list->head = list->head->next;

    if (list->head != NULL) {
        list->head->prev = NULL;
    }
    else {
        list->tail = NULL;
    }

    free(node_to_remove);
    list->size--;
}

void ulist_splice(Ulist* list, int n) {
    if (list->size <= n) {
        return;  // No need to splice if the list size is less than or equal to n
    }
    int count = list->size - n;
    while (count > 0) {
        ulist_remove_back(list);
        count--;
    }
}

void ulist_remove_back(Ulist* list) {
    if (list->tail == NULL) {
        return;
    }

    Node* node_to_remove = list->tail;
    list->tail = list->tail->prev;

    if (list->tail != NULL) {
        list->tail->next = NULL;
    }
    else {
        list->head = NULL;
    }

    free(node_to_remove);
    list->size--;
}

int ulist_size(Ulist* list) {
    return list->size;
}
