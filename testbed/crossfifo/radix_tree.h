#ifndef _CFF_RADIX_TREE_H_
#define _CFF_RADIX_TREE_H_

/*  kv_list_t stands for a group of key-value pairs */

typedef struct kv_t {
    const char * key;           // NULL-term referenced string
    void * val;                 // referenced object
    struct kv_t * next;
    struct kv_t * prev;
} kv_t;

typedef struct kv_list_t {
    kv_t head;
    unsigned int length;
    unsigned int elem_size;
} kv_list_t;

/*  radix_tree_t use radix tree to boost the operations
 *  on the group of key-value pairs */

typedef struct radix_tree_node_t {
    kv_t kv;
    int bit_idx;
    struct radix_tree_node_t * left;
    struct radix_tree_node_t * right;
} rt_node_t;

typedef struct radix_tree_t {
    kv_list_t node_list;
    struct radix_tree_node_t * root;
} radix_tree_t;

/* operations on kv */

kv_list_t * kv_list_create(unsigned int elem_size);

void kv_list_destory(kv_list_t * list);

kv_t * kv_list_insert(kv_list_t * list, const char * key, void * val);

void kv_list_remove(kv_list_t * list, kv_t * kv);

/* operations on radix tree */

#if 0
void radix_tree_init(struct radix_tree_t * tree);

void radix_tree_destory(struct radix_tree_t * tree);

rt_node_t * radix_tree_find(struct radix_tree_t * tree, const char * key);

rt_node_t * radix_tree_insert(struct radix_tree_t * tree, 
        const char * key, 
        void * val);

void radix_tree_remove(struct radix_tree_t * tree, const char * key);
#endif

#endif // _CFF_RADIX_TREE_H_
