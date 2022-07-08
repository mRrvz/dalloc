#ifndef __LIST_H__
#define __LIST_H__

#include "container_of.h"

struct list_head {
    struct list_head *next, *prev;
};

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *newl, struct list_head *next, struct list_head *prev)
{
    next->prev = newl;
    newl->next = next;
    newl->prev = prev;
    prev->next = newl;
}

static inline void list_add(struct list_head *newl, struct list_head *head)
{
    __list_add(newl, head, head->next);
}

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_entry_is_head(pos, head, member) \
	(&pos->member == (head))

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each_entry(pos, head, member)				 \
	for (pos = list_first_entry(head, typeof(*pos), member); \
	     !list_entry_is_head(pos, head, member);			 \
	     pos = list_next_entry(pos, member))

#endif