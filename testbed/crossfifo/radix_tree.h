#ifndef _CFF_RADIX_TREE_H_
#define _CFF_RADIX_TREE_H_

struct radix_tree_t;
typedef struct radix_tree_node_t rt_node_t;

struct radix_tree_node_t {
    int bit_idx;
    rt_node_t * left;
    rt_node_t * right;
    const char * key;       // NULL-terminated string
    void * val;
};

struct radix_tree_t {
    rt_node_t * root;
    unsigned int nelem;
};

void radix_tree_init(struct radix_tree_t * tree);

void radix_tree_destory(struct radix_tree_t * tree);

rt_node_t * radix_tree_find(struct radix_tree_t * tree, const char * key);

rt_node_t * radix_tree_insert(struct radix_tree_t * tree, 
        const char * key, 
        void * val);

void radix_tree_remove(struct radix_tree_t * tree, const char * key);


#endif // _CFF_RADIX_TREE_H_
