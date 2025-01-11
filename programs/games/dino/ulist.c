#include <stdio.h>
#include <stdlib.h>
#include "ulist.h"

Ulist* ulist_create() {
    Ulist* list = (Ulist*)malloc(sizeof(Ulist));
    if (list == NULL) {
        // abort();
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
        // abort();
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
        // abort();
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

int ulist_search(Ulist* list, void* data) {
    Node* current = list->head;
    int index = 0;

    while (current != NULL) {
        if (current->data == data) {
            return index;
        }

        current = current->next;
        index++;
    }

    return -1;
}

void* ulist_get_front(Ulist* list) {
    if (list->head == NULL) {
        return NULL;
    }

    return list->head->data;
}

void* ulist_get_back(Ulist* list) {
    if (list->tail == NULL) {
        return NULL;
    }

    return list->tail->data;
}

int ulist_size(Ulist* list) {
    return list->size;
}

void ulist_print(Ulist* list) {
    Node* current = list->head;

    while (current != NULL) {
        printf("%p ", current->data);
        current = current->next;
    }

    printf("\n");
}

void ulist_test() {
    // Create a new Ulist
    Ulist* list = ulist_create();

    // Test insertFront
    int data1 = 10;
    ulist_push_front(list, &data1);
    printf("List after inserting 10 at the front: ");
    ulist_print(list); // Expected output: 10

    // Test insertBack
    int data2 = 20;
    ulist_push_back(list, &data2);
    printf("List after inserting 20 at the back: ");
    ulist_print(list); // Expected output: 10 20

    // Test removeFront
    ulist_remove_front(list);
    printf("List after removing front element: ");
    ulist_print(list); // Expected output: 20

    // Test removeBack
    ulist_remove_back(list);
    printf("List after removing back element: ");
    ulist_print(list); // Expected output: 

    // Test search
    int data3 = 30;
    ulist_push_front(list, &data3);
    printf("Index of 30 in the list: %d\n", ulist_search(list, &data3)); // Expected output: 0

    // Test getFront
    int* front = (int*)ulist_get_front(list);
    printf("Front element of the list: %d\n", *front); // Expected output: 30

    // Test getBack
    int* back = (int*)ulist_get_back(list);
    printf("Back element of the list: %d\n", *back); // Expected output: 30

    // Test getSize
    printf("Size of the list: %d\n", ulist_size(list)); // Expected output: 1

    // Destroy the list
    ulist_destroy(list);
}
