/* Circular doubly-linked lists/queues

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Reuben Thomas.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#ifndef LIST_H
#define LIST_H

#include <stddef.h>


typedef struct list_s *list;
struct list_s
{
  list prev;
  list next;
  void *item;
};

list list_new (void);
void list_delete (list l);
list list_prepend (list l, void *i);
void *list_behead (list l);

#define list_first(l) ((l)->next)
#define list_next(l)  ((l)->next)
#define list_prev(l)  ((l)->prev)

#endif
