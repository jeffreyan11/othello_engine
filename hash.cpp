#include "hash.h"

Hash::Hash() {
    table = new HashLL* [1000000];
    size = 1000000;
    for(int i = 0; i < size; i++) {
        table[i] = NULL;
    }
    keys = 0;
}

Hash::Hash(int isize) {
    table = new HashLL* [isize];
    size = isize;
    for(int i = 0; i < size; i++) {
        table[i] = NULL;
    }
}

Hash::~Hash() {
    for(int i = 0; i < size; i++) {
        if(table[i] != NULL)
            delete table[i];
    }
    delete table;
}

/**
 * @brief Adds key b and item move into the hashtable.
 * Assumes that no hash with key is in the table.
*/
void Hash::add(const Board *b, int move) {
    keys++;
    uint32_t h = hash(b);
    unsigned int index = h%size;
    HashLL *node = table[index];
    if(node == NULL) {
        table[index] = new HashLL(b->taken, b->black, move);
        return;
    }

    //TODO std::cerr << "hash collide" << std::endl;

    while(node->next != NULL) {
        node = node->next;
    }
    node->next = new HashLL(b->taken, b->black, move);
}
int Hash::get(const Board *b) {
    uint32_t h = hash(b);
    unsigned int index = h%size;
    HashLL *node = table[index];

    if(node == NULL)
        return -1;

    do {
        if(node->cargo.taken == b->taken && node->cargo.black == b->black)
            return node->cargo.move;
        node = node->next;
    }
    while(node != NULL);

    return -1;
}

/**
 * @brief Hashes a board position using the FNV hashing algorithm.
*/
uint32_t Hash::hash(const Board *b) {
    uint32_t h = 2166136261;
    h ^= b->taken & 0xFFFFFFFF;
    h *= 16777619;
    h ^= (b->taken >> 32);
    h *= 16777619;
    h ^= b->black & 0xFFFFFFFF;
    h *= 16777619;
    h ^= (b->black >> 32);
    h *= 16777619;
    return h;
}

