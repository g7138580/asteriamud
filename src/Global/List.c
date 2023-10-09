#include <stdio.h>
#include <stdlib.h>

#include "Global/List.h"

CELL *NewCell()
{
	CELL *cell = malloc( sizeof( *cell ) );

	cell->next = NULL;
	cell->prev = NULL;
	cell->content = NULL;
	cell->valid = true;

	return cell;
}

LIST *NewList( void )
{
	LIST *list = malloc( sizeof( *list ) );

	list->head = NULL;
	list->tail = NULL;
	list->iterators = 0;
	list->size = 0;
	list->valid = true;

	return list;
}

void DeleteCell( CELL *cell, LIST *list )
{
	if ( list->head == cell ) list->head = cell->next;
	if ( list->tail == cell ) list->tail = cell->prev;
	if ( cell->prev ) cell->prev->next = cell->next;
	if ( cell->next ) cell->next->prev = cell->prev;

	free( cell );

	return;
}

void AttachIterator( ITERATOR *iter, LIST *list )
{
	iter->list = list;

	if ( list )
	{
		list->iterators++;
		iter->cell = list->head;
	}
	else
		iter->cell = NULL;

	return;
}

bool IsInList( void *content, LIST *list )
{
	if ( !content )
		return false;

	CELL *cell = NULL;

	for ( cell = list->head; cell; cell = cell->next )
	{
		if ( !cell->valid )
			continue;

		if ( cell->content == content )
			return true;
	}

	return false;
}

bool AttachToList( void *content, LIST *list )
{
	if ( !content )
		return false;

	CELL *cell = NULL;

	for ( cell = list->head; cell; cell = cell->next )
	{
		if ( !cell->valid )
			continue;

		if ( cell->content == content )
			return false;
	}

	cell = NewCell();
	cell->content = content;

	if ( !list->tail )
	{
		list->head = cell;
		list->tail = cell;
	}
	else
	{
		list->tail->next = cell;
		cell->prev = list->tail;
		list->tail = cell;
	}	

	list->size++;

	return true;
}

bool DetachFromList( void *content, LIST *list )
{
	if ( !content )
		return false;

	CELL *cell = NULL;

	for ( cell = list->head; cell; cell = cell->next )
	{
		if ( !cell->valid )
			continue;

		if ( cell->content == content )
		{
			if ( list->iterators > 0 )
				cell->valid = false;
			else
				DeleteCell( cell, list );

			list->size--;

			return true;
		}
	}

	return false;
}

void DetachIterator( ITERATOR *iter )
{
	LIST *list = iter->list;

	if ( !list )
		return;

	list->iterators--;

	if ( list->iterators == 0 )
	{
		CELL *cell = NULL, *cell_next = NULL;

		for ( cell = list->head; cell; cell = cell_next )
		{
			cell_next = cell->next;

			if ( !cell->valid )
				DeleteCell( cell, list );
		}

		if ( !list->valid )
			DeleteList( list );
	}

	return;
}

void DeleteList( LIST *list )
{
	CELL *cell = NULL, *cell_next = NULL;

	if ( list->iterators > 0 )
	{
		list->valid = false;
		return;
	}

	for ( cell = list->head; cell; cell = cell_next )
	{
		cell_next = cell->next;
		DeleteCell( cell, list );
	}

	free( list );

	return;
}

void *NextInList( ITERATOR *iter )
{
	void *content = NULL;

	while ( iter->cell && !iter->cell->valid )
		iter->cell = iter->cell->next;

	if ( iter->cell )
	{
		content = iter->cell->content;
		iter->cell = iter->cell->next;
	}

	return content;
}

int SizeOfList( LIST *list )
{
	if ( !list )
		return 0;

	return list->size;
}

void *GetFromList( LIST *list, int num )
{
	CELL *cell = NULL;
	int cnt = 0;

	if ( num < 1 || num > list->size )
		return NULL;

	for ( cell = list->head; cell; cell = cell->next )
	{
		if ( !cell->valid )
			continue;

		if ( ++cnt == num )
			return cell->content;
	}

	return NULL;
}

void *GetFirstFromList( LIST *list )
{
	if ( !list )
		return NULL;

	return ( list->head ? list->head->content : NULL );
}

void *GetLastFromList( LIST *list )
{
	if ( !list )
		return NULL;

	return ( list->tail ? list->tail->content : NULL );
}
