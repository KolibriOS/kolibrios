
typedef struct link
{
  struct link *prev;
  struct link *next;
}link_t;

#define LIST_INITIALIZE(name) \
	link_t name = { .prev = &name, .next = &name }

#define list_get_instance(link, type, member) \
  ((type *)(((u8_t *)(link)) - ((u8_t *)&(((type *)NULL)->member))))

static inline void link_initialize(link_t *link)
{
	link->prev = NULL;
	link->next = NULL;
}

static inline void list_initialize(link_t *head)
{
	head->prev = head;
	head->next = head;
}

static inline void list_append(link_t *link, link_t *head)
{
	link->prev = head->prev;
	link->next = head;
	head->prev->next = link;
	head->prev = link;
}

static inline void list_remove(link_t *link)
{
	link->next->prev = link->prev;
	link->prev->next = link->next;
	link_initialize(link);
}

static inline bool list_empty(link_t *head)
{
	return head->next == head ? true : false;
}

static inline void list_prepend(link_t *link, link_t *head)
{
	link->next = head->next;
	link->prev = head;
	head->next->prev = link;
	head->next = link;
}

static inline list_insert(link_t *old, link_t *new)
{
   new->prev = old->prev;
   new->next = old;
   new->prev->next = new;
   old->prev = new;
}
