#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

#define CLEAR_LIST( list, var, type, func ) \
{ \
AttachIterator( &Iter, list ); \
while ( ( var = type NextInList( &Iter ) ) ) \
func( var ); \
DetachIterator( &Iter ); \
DeleteList( list ); \
}

#define DESTROY_LIST( list, struct_var, delete_func ) \
{ \
	struct_var *var; \
	ITERATOR Iter; \
\
	AttachIterator( &Iter, list ); \
\
	while ( ( var = ( struct_var * ) NextInList( &Iter ) ) ) \
	{ \
		delete_func( var ); \
	} \
\
	DetachIterator( &Iter ); \
\
	DeleteList( list ); \
}

#define ITERATE_LIST( list, var_type, var, content ) \
{\
	ITERATOR Iter;\
	AttachIterator( &Iter, list ); \
	while ( ( var = ( var_type * ) NextInList( &Iter ) ) ) \
	{ \
		content \
	} \
	DetachIterator( &Iter ); \
}

typedef struct cell_struct		CELL;
typedef struct list_struct		LIST;
typedef struct iterator_struct	ITERATOR;

struct cell_struct
{
	CELL			*next;
	CELL			*prev;
	void			*content;
	bool			valid;
};

struct list_struct
{
	CELL			*head;
	CELL			*tail;
	int				iterators;
	int				size;
	bool			valid;
};

struct iterator_struct
{
	LIST			*list;
	CELL			*cell;
};

extern void AttachIterator( ITERATOR *iter, LIST *list );
extern bool IsInList( void *content, LIST *list );
extern bool AttachToList( void *content, LIST *list );
extern bool DetachFromList( void *content, LIST *list );
extern void DetachIterator	( ITERATOR *iter );
extern LIST *NewList( void );
extern void DeleteList( LIST *list );
extern void *NextInList( ITERATOR *iter );

extern int SizeOfList( LIST *list );
extern void *GetFromList( LIST *list, int num );
extern void *GetFirstFromList( LIST *list );
extern void *GetLastFromList( LIST *list );

#endif
