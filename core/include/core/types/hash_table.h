#ifndef CORE_HASH_TABLE_H
#define CORE_HASH_TABLE_H

#include <stdint.h>

enum HASH_TABLE_ALGORITHM {
    DJB2 = 0
};

struct hash_bucket {
    /// @brief Hash key used to identify an entry
    unsigned long hash;
    /// @brief Key used to create the hash value
    const char *key;
    /// @brief Value to be associated with the key
    void *value;
    /// @brief Pointer to the next item in the bucket
    struct hash_bucket *next;
};

struct hash_table {
    /// @brief Numnber of entries occupied in the hash table
    uint32_t n;
    /// @brief Number of buckets
    uint32_t k;
    /// @brief Load factor used for resizing the table
    float a;
    /// @brief  Array of buckets to store data
    struct hash_bucket *buckets;
};

/// @note Acceptable load factors include 0.6 and 0.75
static const float hash_table_load_factor_0_75 = 0.75;

void hash_table_initialize(struct hash_table *map, uint32_t size);
void hash_table_deinitialize(struct hash_table *map);

void hash_table_insert(struct hash_table *map, const char *key, void *value);
void *hash_table_get(struct hash_table *map, const char *key);
void hash_table_resize(struct hash_table *map);
unsigned long hash_table_djb2(const char *str);
float hash_table_load_factor(struct hash_table *map);

#endif // CORE_HASH_TABLE_H
