/*
 * Implments a generic skip-list, with n sublists. Each sublist has a 
 * head, initialized on list creation. Each sublist shares the payloads/hashes/sizes
 * of its nodes with those of the list below it. 
 * See: https://en.wikipedia.org/wiki/Skip_list
 *
 * Interface notes: 
 * -Relies on the (external) definition of a comparison, called
 *   sl_compare().
 * -Caller's responsibilty to seed the random.h state machine. Easily done
 *   with a single srand(time(NULL)) call before first insert.
 */
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#define P 0.75f // Roll successively to see if node should be promoted [0,1)
#define NUM_LISTS 4 // at least 1

typedef struct ELE {
    void *value;
    size_t payload_size;
    void *hash;
    struct ELE *next;
} list_ele_t;

/*
 * Should return 0 for equality, > 0 for v1 > v2, and < 0 for v1 < v2
 */
extern int sl_compare(void *h1, void *h2);
extern list_ele_t *pack(void *val, size_t size, void *hash);

class skip_list {
  public:
    list_ele_t *heads[NUM_LISTS];
    skip_list() {
      for(int i = 0; i < NUM_LISTS; i++) {
          heads[i] = NULL;
        }
    }

    bool sl_insert(list_ele_t *node);
    bool sl_delete(list_ele_t *node);
    list_ele_t *sl_search(void *payload);
    void sl_print(list_ele_t *start);
  private:
    bool sl_roll();
    void sl_promote(list_ele_t *node, list_ele_t **prev_pts, int list_num);
    list_ele_t *ll_insert(list_ele_t *node, list_ele_t *start);
};

/*
 * Insert immediately at head if list is empty or data will be the smallest
 * node in the list. Else traverse from top left to bottom right, keeping
 * track of the most recently visited node in each list to later do
 * a constant time insert if list splice(s) need to be performed. Try and
 * promote new node after insertion to speed up future accesses.
 */
bool skip_list::sl_insert(list_ele_t *data) {
  list_ele_t *node = (list_ele_t *) malloc(NUM_LISTS * sizeof(list_ele_t));
  if (!node) return false;
  memcpy(node, data, NUM_LISTS * sizeof(list_ele_t));
  if (heads[0]==NULL) { //Empty, init case
    list_ele_t *pt = (list_ele_t *) malloc(NUM_LISTS * sizeof(list_ele_t));
    if (!pt) return false;

    for (int i = 0; i < NUM_LISTS; i++) {
      heads[i] = &pt[i];
      memcpy(&pt[i], node, sizeof(list_ele_t));
      pt[i].next = NULL;
    }
    return true;
  }
  int cmp = sl_compare(node->hash, heads[0]->hash);
  if (0 >= cmp) { //insert at start
    list_ele_t *pt = (list_ele_t *) malloc(NUM_LISTS * sizeof(list_ele_t));
    if (!pt) return false;
    list_ele_t *old_head = heads[0];
    for (int i = 0; i < NUM_LISTS; i++) {
      heads[i] = &pt[i];
      memcpy(&pt[i], node, sizeof(list_ele_t));
      pt[i].next = old_head[i].next;
    }
    heads[0]->next = old_head;
    return true;  
  }
  /*
   * Do a traversal, if a node is found s.t. it compares to strictly less than
   * node, insert node in the bottom list, then roll to potentially promote
   * node into higher list(s).
   */
  int i = NUM_LISTS - 1;
  list_ele_t *prev_pts[NUM_LISTS] = {NULL};
  list_ele_t *pt = heads[i];
  prev_pts[i] = pt;
  for (; i >= 0 ; i--) {
    prev_pts[i] = pt;
    while (pt->next != NULL) {
      if (1 == sl_compare(node->hash, pt->next->hash)) {
        prev_pts[i] = pt;
        pt = pt->next;
      } 
      // found start point for linear search within list 0
      else { 
        prev_pts[0] = prev_pts[i] - i;
        prev_pts[0] = ll_insert(node, prev_pts[0]);
        sl_promote(node, prev_pts, i);
        return true;
      }
    }
    prev_pts[i] = pt;
    pt -= 1;
  }
  prev_pts[0]->next = node;
  node->next = NULL;
  sl_promote(node, prev_pts, 0);
  return true;
}

bool skip_list::sl_delete(list_ele_t *node) {
  return node==NULL;
}

/*
 * Traverse list from top left to bottom right, returning first node found
 * this way on a hash match. Returns NULL if no node is found in this way.
 */
list_ele_t *skip_list::sl_search(void *hash) {
  int i = NUM_LISTS - 1;
  list_ele_t *pt = heads[i];
  if (pt == NULL) {
    return NULL;
  }
  for (; i >= 0 ; i--) {
    int cmp;
    if (0 == (cmp = sl_compare(hash, pt->hash))) {
      return pt;
    }
    else if (0 > cmp) {
      return NULL;
    }
    while (pt->next != NULL) {
      if (1 == (cmp = sl_compare(hash, pt->next->hash))) {
        pt = pt->next;
      } 
      else if (0 == cmp){ 
        return pt->next;
      }
      else {
        break;
      }
    }
    pt -= 1;
  }
  return NULL;
}

/*
 * Print out the skip list. If start is non-NULL, print that sublist first.
 */
void skip_list::sl_print(list_ele_t *start) {
  if (start != NULL) {
    while (start->next != NULL) {
      printf("|%ld|  -->  ", (long) start->hash);
      start = start->next;
    }
    printf("|%ld|  --X\n", (long) start->hash);

  }
  for (int i = NUM_LISTS - 1; i >=0; i--) {
    printf("List %d:\t", i);
    list_ele_t *pt = heads[i];
    if (pt==NULL) {
      printf("  --X\n");
      continue;
    }
    while (pt->next != NULL) {
      printf("|%ld|  -->  ", (long) pt->hash);
      pt = pt->next;
    }
    printf("|%ld|  --X\n", (long) pt->hash);
  }
}
/*
 * Gives the output of a coin flip with probability 'P.'
 */
bool skip_list::sl_roll() {
  long r = random() % 1000;
  bool out = ((float) r) < (P * (float) 1000);
  return out;
}

void skip_list::sl_promote(list_ele_t *node, list_ele_t **prev_pts, int list_num) {
  int i = 0;
  for (; i <= list_num; i++) {
    prev_pts[i] = prev_pts[0] + i;
  }
  i = 1;
  while (sl_roll() && i < NUM_LISTS) {
    memcpy(&node[i], node, sizeof(list_ele_t));
    (&node[i])->next = prev_pts[i]->next;
    prev_pts[i]->next = &node[i];
    i++;
  }
}

list_ele_t *skip_list::ll_insert(list_ele_t *node, list_ele_t *start) {
  assert (0 <= sl_compare(node->hash, start->hash));
  if (0 == sl_compare(node->hash, start->hash)) {
    node->next = start->next;
    start->next = node;
    return start;
  }
  list_ele_t *pt = start;
  list_ele_t *prev = pt;
  while (prev->next != NULL) {
    pt = pt->next;
    if (0 < sl_compare(pt->hash, node->hash)) {
      node->next = pt;
      prev->next = node;
      return prev;
    }
    prev = pt; 
  } 
  prev->next = node;
  node->next = NULL;
  return prev;
}


void test_sl() {
  srand(time(NULL));
  char val[25] = "Corruption check";
  size_t size = 1 + (size_t) strlen(val);
  list_ele_t *node1 = pack(val, size, (void *) 1);
  list_ele_t *node2 = pack(val, size, (void *) 2);
  list_ele_t *node3 = pack(val, size, (void *) 3);
  list_ele_t *node4 = pack(val, size, (void *) 10);
  list_ele_t *node5 = pack(val, size, (void *) 15);
  skip_list sl;
  sl.sl_insert(node1);
  sl.sl_insert(node3);
  
  sl.sl_insert(node5);
  sl.sl_insert(node2);
  sl.sl_delete(node3);
  sl.sl_insert(node4);
  sl.sl_insert(node3);
  sl.sl_delete(node1); 
  sl.sl_delete(node5);
  sl.sl_insert(node1);
  sl.sl_insert(node3);
  sl.sl_print(sl.sl_search((void *) 10));
  printf("\n");
  sl.sl_print(sl.sl_search((void *) 15));
  return;
}

