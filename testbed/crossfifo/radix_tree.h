#ifndef _CFF_RADIX_TREE_H_
#define _CFF_RADIX_TREE_H_

/* kv_list_t stands for a group of key-value pairs */

typedef struct kv_t {
    const char * key;                   // NULL-term string
    void * val
    struct kv_t * next;
    struct kv_t * prev;
} kv_t;

typedef struct kv_list_t {
    kv_t head;
    unsigned int length;
    unsigned int elem_size;
} kv_list_t;

/* radix tree */

typedef struct radix_tree_node_t {
    kv_t kv;
    int bit_idx;
    struct radix_tree_node_t * left;    // zero branch
    struct radix_tree_node_t * right;   // none zero branch
} rt_node_t;

typedef struct radix_tree_t {
    kv_list_t * node_list;
    struct radix_tree_node_t * root;
} radix_tree_t;

/* operations */

// kv ops
kv_list_t * kv_list_create(unsigned int elem_size);

void kv_list_destory(kv_list_t * list);

kv_t * kv_list_insert(kv_list_t * list, const char * key, void * val);

void kv_list_remove(kv_list_t * list, kv_t * kv);

// radix tree ops
radix_tree_t * radix_tree_create();

void radix_tree_destory(radix_tree_t * tree);

void * radix_tree_lookup(radix_tree_t * tree, const char * key);

int radix_tree_insert(struct radix_tree_t * tree, 
        const char * key, 
        void * val);

int radix_tree_replace(struct radix_tree_t * tree, 
        const char * key, 
        void * val);
#if 0
void radix_tree_remove(struct radix_tree_t * tree, const char * key);
#endif

#endif // _CFF_RADIX_TREE_H_
