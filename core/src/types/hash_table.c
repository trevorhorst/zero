#include <malloc.h>

#include "core/logger.h"
#include "core/types/hash_table.h"

/**
 * @brief Initializes a hash map dat structure
 *
 * @param map Desired map
 * @param size Starting size of the hash map
 */
void hash_table_initialize(struct hash_table *map, uint32_t size)
{
    if(map) {
        map->n = 0;
        map->k = size;
        map->a = hash_table_load_factor_0_75;
        map->buckets = (struct hash_bucket*)malloc(sizeof(struct hash_bucket) * size);
        for(unsigned int i = 0; i < size; i++) {
            map->buckets[i].hash = 0;
            map->buckets[i].key = NULL;
            map->buckets[i].value = NULL;
            map->buckets[i].next = NULL;
        }
    }
}

void hash_table_deinitialize(struct hash_table *map)
{
    for(unsigned int i = 0; i < map->k; i++) {
        struct hash_bucket *it = map->buckets[i].next;
        while(it != NULL) {
            // Hold the current iteration, to be freed later
            struct hash_bucket *current = it;

            // Set the iterator for the next value
            it = it->next;

            // Free the current iteration and continue
            free(current);
        }
    }

    // Free the maps buckets
    free(map->buckets);
    map->buckets = NULL;
    map->n = 0;
}

bool hash_table_insert(struct hash_table *map, const char *key, void *value)
{
    bool success = false;
    unsigned long hash = hash_table_djb2(key);
    LOG_DEBUG("insert key %s (%lu)\n", key, hash);
    unsigned int bucket = hash % map->k;
    if(map->buckets[bucket].value == NULL) {
        // This bucket is unoccupied so we can safely add the hash value
        map->buckets[bucket].key = key;
        map->buckets[bucket].hash = hash;
        map->buckets[bucket].value = value;
        map->n++;
        success = true;
    } else {
        LOG_DEBUG("Hash map collision detected\n");
        struct hash_bucket *it = &map->buckets[bucket];

        if(it->hash == hash) {
            LOG_DEBUG("Duplicate 1 detected\n");
            it->key = key;
            it->hash = hash;
            it->value = value;
            success = true;
        } else {
            char duplicate = 0;

            while(it->next) {
                it = it->next;
                if(it->hash == hash) {
                    LOG_DEBUG("Duplicate 2 detected\n");
                    duplicate = 1;
                    break;
                }
            }

            if(duplicate) {
                // Item is duplicate, no need to create new memory for it
                it->key = key;
                it->hash = hash;
                it->value = value;
                success = true;
            } else {
                // Item is new, create new entry for the linked list
                struct hash_bucket *new_bucket = (struct hash_bucket*)malloc(sizeof(struct hash_bucket));
                new_bucket->key = key;
                new_bucket->hash = hash;
                new_bucket->value = value;
                new_bucket->next = NULL;
                it->next = new_bucket;
                map->n++;
                success = true;
            }
        }
    }

    float a = hash_table_load_factor(map);
    LOG_DEBUG("Load factor is %f\n", a);
    if(a > map->a /*|| (a < (map->a / 4))*/) {
        LOG_DEBUG("Resizing map...\n");
        // hash_table_resize(map);
        LOG_DEBUG("Resizing complete.\n");
    }

    return success;
}

/**
 * @brief Retrieves a value stored in the map
 *
 * @param map Desired map
 * @param key Desired key for hashing
 * @return void* Pointer to the stored data
 */
void *hash_table_get(struct hash_table *map, const char *key)
{
    void *value = NULL;
    unsigned long hash = hash_table_djb2(key);
    LOG_DEBUG("%s hash returns %lu\n", key, hash);
    unsigned int bucket = hash % map->k;

    struct hash_bucket *it = &map->buckets[bucket];
    while((it != NULL) && (it->hash != hash)) {
        it = it->next;
    }

    if(it) {
        value = it->value;
    }

    return value;
}

void hash_table_resize(struct hash_table *map)
{
    struct hash_table new_map;
    hash_table_initialize(&new_map, (map->k * 2));

    for(unsigned int i = 0; i < map->k; i++) {
        LOG_DEBUG("Bucket %d\n", i);
        struct hash_bucket *it = &map->buckets[i];
        while(it != NULL && (it->value != NULL)) {
            LOG_DEBUG("  %lu\n", it->hash);
            hash_table_insert(&new_map, it->key, it->value);
            it = it->next;
        }
    }

    hash_table_deinitialize(map);
    map->buckets = new_map.buckets;
    map->n       = new_map.n;
    map->k       = new_map.k;
}

void hash_table_traverse(struct hash_table *map)
{
    for(unsigned int i = 0; i < map->k; i++) {
        LOG_DEBUG("Bucket %d\n", i); 
        struct hash_bucket *it = &map->buckets[i];
        while(it != NULL) {
            printf("  %s: %lu\n", it->key, it->hash);
            it = it->next;
        }
    }   
}

unsigned long hash_table_djb2(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

float hash_table_load_factor(struct hash_table *map)
{
    return ((float)map->n / (float)map->k);
}
