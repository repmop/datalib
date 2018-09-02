/*
 * Implments a generic skip-list, with n sublists. Each sublist has a 
 * head, initialized on list creation. Each sublist shares the payloads of 
 * all of its nodes with that of the list below it. Each node in the list 
 * stores the number of nodes skipped by its link pointer (1 for the base list)
 * See: https://en.wikipedia.org/wiki/Skip_list
 *
 * Interface notes: Relies on the definition of a comparison function, called
 * sl_compare().
 */
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#define P .5f // Roll successively to see if node should be promoted [0,1)
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
    void ll_insert(list_ele_t *start, list_ele_t *node);
    void sl_promote(list_ele_t *node, list_ele_t **prev_pts, int list_num);
};

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
      pt[i].next = NULL;
    }
    heads[0]->next = old_head;
    return true;  
  }
  int i = NUM_LISTS - 1;
  list_ele_t *prev_pts[NUM_LISTS] = {NULL};
  list_ele_t *pt = heads[i];
  prev_pts[i] = pt;
  for (; i >= 0 ; i--) {
    prev_pts[i] = pt;
    while (pt->next != NULL) {
      if (0 <= sl_compare(node->hash, pt->next->hash)) {
        prev_pts[i] = pt;
        pt = pt->next;
      } 
      // found insertion point for list 0
      else { 
        prev_pts[0] = prev_pts[i] - i;
        pt = prev_pts[0]->next;
        node->next = pt;
        prev_pts[0]->next = node;
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

list_ele_t *skip_list::sl_search(void *payload) {
  return (list_ele_t *) payload;
}

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

bool skip_list::sl_roll() {
  long r = random() % 1000;
  bool out = ((float) r) > (P * (float) 1000);
  return out;
}
/*
 * Attempt to insert after start. If node preceedes start, change node's
 * next and rely on caller to handle actual insert
 */
void skip_list::ll_insert(list_ele_t *start, list_ele_t *node) {
  if (0 > sl_compare(node->hash, start->hash)) {
    //node->next = start;
    return;
  }
  list_ele_t *prev_pt = start;
  list_ele_t *pt = prev_pt->next;
  while (pt != NULL && 1 == sl_compare(node->hash, pt->hash)) {
    if (pt->next == NULL) {
      node->next = NULL;
      pt->next = node;
      return;
    }
    prev_pt = pt;
    pt = pt->next;
  }
  node->next = pt;
  prev_pt->next = node;
}

void skip_list::sl_promote(list_ele_t *node, list_ele_t **prev_pts, int list_num) {
  int i = 0;
  for (; i <= list_num; i++) {
    prev_pts[i] = prev_pts[0] + i;
  }
  i = 1;
  while (sl_roll() && i < NUM_LISTS) {
    memcpy(node + i, node, sizeof(list_ele_t));
    (&node[i])->next = prev_pts[i]->next;
    prev_pts[i]->next = &node[i];
    i++;
  }
}


void test_sl() {
  srand(time(NULL));
  char val[25] = "Corruption check";
  size_t size = 1 + (size_t) strlen(val);
  list_ele_t *node1 = pack(val, size, (void *) 1);
  list_ele_t *node2 = pack(val, size, (void *) 2);
  list_ele_t *node3 = pack(val, size, (void *) 3);
  skip_list sl;
  sl.sl_insert(node1);
  sl.sl_insert(node3);
  
  sl.sl_insert(node2);
  sl.sl_insert(node2);
  sl.sl_insert(node3);
  sl.sl_print(NULL);
  return;
}

