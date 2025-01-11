#ifndef ULIST_H_
#define ULIST_H_

typedef struct Node {
    void* data;
    struct Node* prev;
    struct Node* next;
} Node;

typedef struct Ulist {
    Node* head;
    Node* tail;
    int size;
} Ulist;

Ulist* ulist_create();
void ulist_destroy(Ulist* list);
void ulist_push_front(Ulist* list, void* data);
void ulist_push_back(Ulist* list, void* data);
void ulist_remove(Ulist *list, Node *node);
void ulist_remove_front(Ulist* list);
void ulist_splice(Ulist* list, int n);
void ulist_remove_back(Ulist* list);
int ulist_search(Ulist* list, void* data);
void* ulist_get_front(Ulist* list);
void* ulist_get_back(Ulist* list);
int ulist_size(Ulist* list);
void ulist_print(Ulist* list);
void ulist_test();

#endif /* ULIST_H_ */