/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "q.h"

list_ele_t *pack(void *val, size_t size, void *hash) {
  list_ele_t *out = (list_ele_t *) malloc(sizeof(list_ele_t));
  if (out==NULL) return NULL;
  out->value = malloc(size);
  if (out->value==NULL) return NULL;
  out->hash = hash;
  out->payload_size = size;
  memcpy(out->value, val, size);
  out->next = NULL;
  return out;
}

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    /* Remember to handle the case if malloc returned NULL */
    queue_t *out = (queue_t *) malloc(sizeof(queue_t));
    if (out==NULL) return NULL;
    out->size = 0;
    out->nodes = 0;
    out->tail = NULL;
    out->head = NULL;
    return out;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
  /* Remember to free the queue structue and list elements */
  if (q==NULL) return;
  list_ele_t *pt = q->head;
  list_ele_t *next = pt;
  while(next!=NULL)
  {
    next = pt->next;
    free(pt->value);
    free(pt);
    pt = next;
  }
  free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_head(queue_t *q, list_ele_t *newHead)
{
    if (q==NULL) return false;
    if (newHead==NULL) return false;
    list_ele_t *oldHead = q->head;
    newHead->next = oldHead;
    q->head = newHead;
    if (q->tail==NULL) q->tail = newHead;
    q->nodes++;
    q->size+=newHead->payload_size;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_tail(queue_t *q, list_ele_t *newTail)
{
    if (q==NULL) return false;
    if (newTail==NULL) return false;
    newTail->next = NULL;
    if (q->tail!=NULL) q->tail->next = newTail;
    if (q->head==NULL) q->head = newTail;
    q->tail = newTail;
    q->nodes++;
    q->size+=newTail->payload_size;
    return true;
}
/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  Any unused storage should be freed
*/
bool q_remove_head(queue_t *q, bool free_after)
{
    if (q==NULL) return false;
    if (q->head==NULL || q->tail==NULL) return false;
    //Or here to protect against a breach in the data structure invariant(s)
    list_ele_t *oldHead = q->head;
    q->head = oldHead->next;
    if (q->tail==oldHead) 
    {
      q->head = NULL;
      q->tail=NULL;
    }
    q->nodes--;
    q->size-=oldHead->payload_size;
    if (free_after)
    {
      free(oldHead->value);
      free(oldHead);
    }
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_nodes(queue_t *q)
{
    if (q==NULL) return 0;
    if (q->head==NULL || q->tail==NULL) return 0;
    return q->nodes;
}

size_t q_size(queue_t *q)
{
  if (q==NULL) return 0;
  if (q->head==NULL || q->tail==NULL) return 0;
  return q->size;
}

/*
  Reverse elements in queue.

  Your implementation must not allocate or free any elements (e.g., by
  calling q_insert_head or q_remove_head).  Instead, it should modify
  the pointers in the existing data structure.
 */
void q_reverse(queue_t *q)
{
    if (q==NULL) return;
    if (q->head==NULL || q->tail==NULL) return;
    list_ele_t *pt = q->head;
    list_ele_t *next = pt->next;
    list_ele_t *prev = pt;
    list_ele_t *oldTail = q->tail;
    while (next!=NULL)
    {
      pt=next;
      next = next->next;
      pt->next = prev;
      prev = pt;
    }
    q->tail = q->head;
    q->head = oldTail;
    q->tail->next = NULL;

}

/*
 * Search the queue for a node with a hash field that matches hash,
 * using hash_compare(). 
 */
list_ele_t *q_search(queue_t *q, void *hash, 
  bool (*hash_compare)(void *h1, void *h2))
{
  if (q==NULL) return NULL;
  if (hash==NULL) return NULL;
  if (q->head==NULL || q->tail==NULL) return NULL;
  list_ele_t *pt = q->head;
  do 
  { 
    if (hash_compare(hash, pt->hash)) return pt;
    pt = pt->next;
    if (pt==NULL) return NULL;
  } while (pt->next != NULL);
  if (hash_compare(hash, pt->hash)) return pt;
  return NULL;
}

/*
 * Scan through q for node's prev pointer, changing its next to circumvent node.
 * Then, reinsert node into the tail
 */
void q_shuffle(queue_t *q, list_ele_t *node)
{
  if (q==NULL) return;
  if (node==q->head) q_remove_head(q, false);
  list_ele_t *pt = q->head;
  while (pt != NULL)
  {
    if (pt->next==node) pt->next = node->next;
    pt = pt->next;
  }
  q_insert_tail(q, node);
}

void q_print(queue_t *q) {
  int i = 0;
  for (list_ele_t *pt = q->head; pt != NULL; pt = pt->next) {
    printf("Node %d with hash at %p has size %#zx\n", 
          i, &pt->hash, pt->payload_size);
    i++;
  }
}

void test_q() {
  char val[25] = "Corruption check";
  size_t size = 1 + (size_t) strlen(val);
  queue_t *q = q_new();
  list_ele_t *node1 = pack(val, size, (void *) 1);
  list_ele_t *node2 = pack(val, size, (void *) 2);
  q_insert_tail(q, node1);
  q_insert_head(q, node2);
  assert(q_nodes(q) == 2);
  assert(q_size(q) == 2 * size);
  assert(q_search(q, (void *) 1, &hash_compare));
  assert(q_search(q, (void *) 2, &hash_compare));
  assert(!q_search(q, (void *) 3, &hash_compare));
  q_shuffle(q, node2);
  assert(q->head == node1);
  q_print(q);
  q_reverse(q);
  assert(q->head == node2);
  assert(q->tail == node1);
  assert(q_nodes(q) == 2);
  assert(q_size(q) == 2 * size);
  q_remove_head(q, 0);
  assert(q->tail == node1);
  assert(q->head == node1);
  assert(q_nodes(q) == 1);
  assert(q_size(q) == 1 * size);
  q_remove_head(q, 0);
  assert(q_nodes(q) == 0);
  assert(q_size(q) == 0 * size);
}

bool hash_compare(void *h1, void *h2) {
  return (int64_t) (intptr_t) h1 == (int64_t) (intptr_t) h2;
}

