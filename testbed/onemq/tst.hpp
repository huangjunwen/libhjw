#ifndef _TST_HPP_
#define _TST_HPP_

namespace omq {

// simple ternary search tree as a dict

template <typename V>
struct tst_node_t {
    tst_node_t();
    tst_node_t<V> * left;
    tst_node_t<V> * right;
    tst_node_t<V> * middle;
    tst_node_t<V> * next;
    unsigned char c;
    unsigned char occupied;
    V val;
};

template <typename V>
tst_node_t<V>::tst_node_t(): left(NULL), right(NULL), middle(NULL), 
    next(NULL),
    c(0),
    occupied(0),
    val() {}

template <typename V>
class tst_t {
public:
    tst_t(): root(NULL), nodes(NULL), key_count(0) {
    }
    ~tst_t();
    // Return false if already has a key.
    bool insert(V val, const unsigned char * key, size_t klen = 0);
    void replace(V val, const unsigned char * key, size_t klen = 0);
    bool find(V * val, const unsigned char * key, size_t klen = 0);
    uint32_t get_key_count(){ return key_count; }
    void clear();
protected:
    tst_node_t<V> * find_node(const unsigned char * key, size_t klen, 
        bool create_if_not_found);

    tst_node_t<V> * root;
    tst_node_t<V> * nodes;
    uint32_t key_count;
};

template <typename V>
bool tst_t<V>::insert(V val, const unsigned char * key, size_t klen) {
    tst_node_t<V> * n  = find_node(key, klen, true);
    if (n->occupied)
        return false;
    n->val = val;
    n->occupied = 1;
    ++key_count;
    return true;
}

template <typename V>
void tst_t<V>::replace(V val, const unsigned char * key, size_t klen) {
    tst_node_t<V> * n  = find_node(key, klen, true);
    if (n->occupied) {
        n->val = val;
        return;
    }
    n->occupied = 1;
    ++key_count;
}

template <typename V>
bool tst_t<V>::find(V * val, const unsigned char * key, size_t klen) {
    tst_node_t<V> * n  = find_node(key, klen, false);
    if (!n || !n->occupied)
        return false;
    *val = n->val;
    return true;
}

template <typename V>
void tst_t<V>::clear() {
    tst_node_t<V> * n;
    while ((n = nodes)) {
        nodes = nodes->next;
        delete n;
    }
    root = NULL;
    key_count = 0;
}

template <typename V>
tst_t<V>::~tst_t() {
    clear();
}

template <typename V>
tst_node_t<V> * tst_t<V>::find_node(const unsigned char * key, size_t klen, 
        bool create_if_not_found) {

    const unsigned char * end, * s;
    if (klen) {
        end = key + klen;
    }
    else {
        end = key;
        while (*end) ++end;
        // Not allow empty string.
        if (end == key)
            return NULL;
    }

    tst_node_t<V> ** pnext = &root;
    tst_node_t<V> * curr;
    s = key;

    while (s != end) {
        if (!(curr = *pnext))
            break;
        if (*s == curr->c) {
            pnext = &curr->middle;
            ++s;
        }
        else if (*s < curr->c) {
            pnext = &curr->left;
        }
        else {
            pnext = &curr->right;
        }
    }

    if (s == end)
        return curr;
    if (!create_if_not_found)
        return NULL;

    while (s != end) {
        curr = *pnext = new tst_node_t<V>();
        curr->c = *s;
        curr->next = nodes;
        nodes = curr;
        pnext = &curr->middle;
        ++s;
    }
    return curr;
}

}

#endif // _TST_HPP_
