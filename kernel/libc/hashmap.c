#include "hashmap.h"

typedef struct hashmap_item_t hashmap_item_t;
struct hashmap_item_t {
    void *key;
    void *value;
    hashmap_item_t *prev_item;
};

typedef struct {

} hashmap_t;