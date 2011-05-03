
#include <stdlib.h>
#include <stdint.h>

/*
 * a implement of Tree bitmap algorithm
 * ref: 
 *   "Tree Bitmap : Hardware/Software IP Lookups with Incremental Updates"
 *
 */

// how many level in a tbm_node_t
#define STRIDE (8)

#define LEAF_BIT (2 << STRIDE)

#define INNER_BIT (LEAF_BIT - 1)

#define LEAF_BITMASK_SZ ((LEAF_BIT + 7)/8)

#define INNER_BITMASK_SZ ((INNER_BIT + 7)/8)

typedef struct tbm_node_t {
    uint8_t inner_bitmask[INNER_BITMASK_SZ];
    uint8_t leaf_bitmask[LEAF_BITMASK_SZ];
    void * inner;
    struct tbm_node_t * leaf;
} tbm_node_t;

typedef struct tbm_t {
    tbm_node_t root;
} tbm_t;

static tbm_node_t * _malloc_nodes(size_t num) {
    tbm_node_t * nodes = (tbm_node_t *)malloc(sizeof(tbm_node_t) * num);
    if (NULL == nodes)
        return NULL;
    memset(nodes, 0, sizeof(tbm_node_t) * num);
    return nodes;
}

void tbm_init(tbm_t * tree) {
    memset(&tree->root, 0, sizeof(tbm_node_t));
}

static void _free_nodes(tbm_node_t * node) {
}

void tbm_fini(tbm_t * tree) {
    tbm_node_t * n;

}
