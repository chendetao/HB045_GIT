#ifndef __LIST_H__
#define __LIST_H__

struct List
{
	struct List *prev;
	struct List *next;
	int win_id;
};

extern struct List *pc;

extern void List_Init(void);
extern int List_Add(struct List *n);
extern struct List * List_Get(void);

#endif
