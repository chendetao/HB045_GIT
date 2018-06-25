#include "list.h" 
#include "Window.h"
#include <stdio.h>
#include <string.h>

#define SIZEOF(x) (sizeof(x)/sizeof(x[0]))

struct List list_heap[10];
struct List *pc;
unsigned long head_used = 0;

void List_Init(void)
{
	memset(&list_heap, 0, sizeof(list_heap));
	head_used = 0;
	pc = NULL;
}

int List_Add(struct List *n)
{
	if ( n == NULL )
	{
		return (-1);
	}
	
	if ( pc == NULL )
	{
		n->next = n;
		n->prev = n;
		pc = n;
	} else {
	
		n->next  = pc->next;
		n->prev  = pc;
		pc->next = n;
	}
	
	return 0;
}

struct List * List_Get(void)
{
	for ( int i = 0; i < SIZEOF(list_heap); i++ )
	{
		if ( !(head_used & (0x1<<i)) )
		{
			head_used |= (0x1<<i);
			return &list_heap[i];
		}
	}
	
	return NULL;
}
