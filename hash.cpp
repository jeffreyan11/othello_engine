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
    keys = 0;
}

Hash::~Hash() {
    for(int i = 0; i < size; i++) {
        HashLL* temp = table[i];
        while(temp != NULL) {
            HashLL *temp2 = temp->next;
            delete temp;
            temp = temp2;
        }
    }
    delete[] table;
}

/**
 * @brief Adds key (b,ptm) and item move into the hashtable.
 * Assumes that this key has been checked with get and is not in the table.
*/
void Hash::add(const Board &b, int score, int move, int ptm, int turn,
        int depth, uint8_t nodeType) {
    keys++;
    #if USE_HASH64
    bitbrd h = hash(b);
    #else
    uint32_t h = hash(b);
    #endif
    unsigned int index = (unsigned int)(h % size);
    HashLL *node = table[index];
    if(node == NULL) {
        table[index] = new HashLL(b.taken, b.black, score, move, ptm, turn,
            depth, nodeType);
        return;
    }

    //TODO std::cerr << "hash collide" << std::endl;

    while(node->next != NULL) {
        if(node->cargo.taken == b.taken && node->cargo.black == b.black
                    && node->cargo.ptm == (uint8_t) ptm)
            return;
        node = node->next;
    }

    if(node->cargo.taken == b.taken && node->cargo.black == b.black
                && node->cargo.ptm == (uint8_t) ptm)
        return;
    node->next = new HashLL(b.taken, b.black, score, move, ptm, turn,
        depth, nodeType);
}

/**
 * @brief Get the move, if any, associated with a board b and player to move.
*/
BoardData *Hash::get(const Board &b, int ptm) {
    #if USE_HASH64
    bitbrd h = hash(b);
    #else
    uint32_t h = hash(b);
    #endif
    unsigned int index = (unsigned int)(h % size);
    HashLL *node = table[index];

    if(node == NULL)
        return NULL;

    do {
        if(node->cargo.taken == b.taken && node->cargo.black == b.black
                    && node->cargo.ptm == (uint8_t) ptm)
            return &(node->cargo);
        node = node->next;
    }
    while(node != NULL);

    return NULL;
}

void Hash::clean(int turn) {
    for(int i = 0; i < size; i++) {
        HashLL *node = table[i];
        if(node == NULL)
            continue;
        while(node->cargo.turn <= turn) {
            keys--;
            table[i] = node->next;
            delete node;
            node = table[i];
            if(node == NULL)
                break;
        }
    }
}

/**
 * @brief Hashes a board position using the FNV hashing algorithm.
*/
#if USE_HASH64
bitbrd Hash::hash(const Board &b) {
    bitbrd h = 14695981039346656037ULL;
    h ^= b.taken;
    h *= 1099511628211;
    h ^= b.black;
    h *= 1099511628211;
    return h;
}
#else
uint32_t Hash::hash(const Board &b) {
    uint32_t h = 2166136261UL;
    h ^= b.taken & 0xFFFFFFFF;
    h *= 16777619;
    h ^= (b.taken >> 32);
    h *= 16777619;
    h ^= b.black & 0xFFFFFFFF;
    h *= 16777619;
    h ^= (b.black >> 32);
    h *= 16777619;
    return h;
}
#endif

void Hash::test() {
    int zeros = 0;
    int threes = 0;

    for(int i = 0; i < size; i++) {
        int linked = 0;
        HashLL* node = table[i];
        if(node == NULL)
            zeros++;
        else {
            linked++;
            while(node->next != NULL) node = node->next;
            if(linked >= 3) threes++;
        }
    }

    cout << "zeros: " << zeros << endl;
    cout << "threes: " << threes << endl;
}
