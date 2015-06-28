/* llist.h - Header file for llist.c
   Eugene Kim, <eekim@eekim.com>
   $Id: cgi-llist.h,v 1.2 1997/10/06 08:58:21 eekim Exp $

   Copyright (C) 1995 Eugene Eric Kim
   All Rights Reserved
*/

#ifndef _CGI_LLIST
#define _CGI_LLIST 1

#define FREE(p)  do { if (p != NULL) { free(p); p= NULL; } } while (0)

typedef struct {
  char *name;
  char *value;
} entrytype;

typedef struct _node {
  entrytype entry;
  struct _node* next;
} node;

typedef struct {
  node* head;
} llist;

void list_create(llist *l);
node* list_next(node* w);
short on_list(llist *l, node* w);
short on_list_debug(llist *l, node* w);
void list_traverse(llist *l, void (*visit)(entrytype item));
node* list_insafter(llist* l, node* w, entrytype item);
void list_clear(llist* l);

#endif

