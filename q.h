/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/************** Data structure declarations ****************/

typedef struct ELE {
    void *value;
    size_t payload_size;
    void *hash;
    struct ELE *next;
} list_ele_t;

/* Queue structure */
typedef struct {
    list_ele_t *head;  /* Linked list of elements */
    /*
      You will need to add more fields to this structure
      to efficiently implement q_size and q_insert_tail
    */
    list_ele_t *tail;
    int nodes;
    size_t size;
} queue_t;

/************** Operations on queue ************************/

list_ele_t *pack(void *val, size_t size, void *hash);

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new();

/*
  Free all storage used by queue.
  No effect if q is NULL
*/
void q_free(queue_t *q);

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_head(queue_t *q, list_ele_t *newHead);

/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_tail(queue_t *q, list_ele_t *newTail);

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  Any unused storage should be freed
*/
bool q_remove_head(queue_t *q, bool free_after);

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_nodes(queue_t *q);

/*
  Reverse elements in queue
  No effect if q is NULL or empty
 */
void q_reverse(queue_t *q);

/*
 * Search the queue for a node with a hash field that matches hash,
 * using hash_compare(). 
 */
list_ele_t *q_search(queue_t *q, void *hash, 
  bool (*hash_compare)(void *h1, void *h2));


/*
 * Scan through q for node's prev pointer, changing its next pointer 
 * to circumvent node. Then, reinsert node into the tail.
 */
void q_shuffle(queue_t *q, list_ele_t *node);

bool hash_compare(void *h1, void *h2);

void q_print(queue_t *q);

