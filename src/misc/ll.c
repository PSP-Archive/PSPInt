/* ======================================================================== */
/*  LL   -- Singly linked list routines.                                    */
/*                                                                          */
/*  LL_INSERT     -- Inserts an element at the head of a linked list.       */
/*  LL_REVERSE    -- Reverses a BC_LLIST_T linked list.                     */
/*  LL_CONCAT     -- Concatenate one list onto another.                     */
/*  LL_ACTON      -- Performs an action on each element of a linked list.   */
/*  LL_FREE       -- Frees a linked list.                                   */
/* ======================================================================== */

#include "config.h"
#include "misc/ll.h"

/* ======================================================================== */
/*  LL_INSERT     -- Inserts an element at the head of a linked list.       */
/* ======================================================================== */
ll_t *ll_insert
(
    ll_t *const RESTRICT head, 
    ll_t *const RESTRICT elem
)
{
    if (!elem)
        return head;

    elem->next = head;
    return elem;
}


/* ======================================================================== */
/*  LL_REVERSE    -- Reverses a LLIST_T linked list.                        */
/* ======================================================================== */
void ll_reverse
(
    ll_t *RESTRICT *RESTRICT head
)
{
    ll_t *p, *q, *r;

    p = NULL;
    q = *head;
    
    if (!q || !q->next)
        return;

    while (q)
    {
        r       = q->next;
        q->next = p;
        p       = q;
        q       = r;
    }

    *head = p;
}

/* ======================================================================== */
/*  LL_CONCAT     -- Concatenate one list onto another.                     */
/* ======================================================================== */
void ll_concat
(
    ll_t *RESTRICT *RESTRICT head, 
    ll_t *RESTRICT const     list
)
{
    if (!head)
        return;

    while (*head)
        head = &((*head)->next);

    *head = list;
}


/* ======================================================================== */
/*  LL_ACTON      -- Performs an action on each element of a linked list.   */
/* ======================================================================== */
void  ll_acton  (ll_t *RESTRICT list, void (act)(ll_t *, void *), void *opq)
{
    ll_t *prev;

    while (list)
    {
        prev = list;
        list = list->next;
        act(prev, opq);
    }
}

/* ======================================================================== */
/*  LL_FREE       -- Frees a linked list.                                   */
/* ======================================================================== */
void  ll_free   (ll_t *list)
{
    ll_t *prev;

    while (list)
    {
        prev = list;
        list = list->next;
        free(prev);
    }
}


/* ======================================================================== */
/*  This program is free software; you can redistribute it and/or modify    */
/*  it under the terms of the GNU General Public License as published by    */
/*  the Free Software Foundation; either version 2 of the License, or       */
/*  (at your option) any later version.                                     */
/*                                                                          */
/*  This program is distributed in the hope that it will be useful,         */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       */
/*  General Public License for more details.                                */
/*                                                                          */
/*  You should have received a copy of the GNU General Public License       */
/*  along with this program; if not, write to the Free Software             */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               */
/* ======================================================================== */
/*                 Copyright (c) 2003-+Inf, Joseph Zbiciak                  */
/* ======================================================================== */
