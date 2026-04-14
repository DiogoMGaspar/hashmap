#include "hashmap.h"

#include <stdlib.h>
#include <string.h>


#define HMAP_LOAD_FACTOR        0.65
#define HMAP_TOMBSTONE_RATIO    0.4
#define HMAP_MIN_CAPACITY       16

// 32-bit FNV-1a constants

# define FNV_OFFSET_BASIS  2166136261u
# define FNV_PRIME         16777619u


// Helpers

/// @brief Computes the 32-bit FNV-1a hash
/// @param key Key to be hashed
/// @return The hashed index
static uint32_t	fnv1a(const char* key)
{
	uint32_t hash = FNV_OFFSET_BASIS;
    
    for (const uint8_t* bytes = (const uint8_t*)key; *bytes; bytes++)
    {
        hash ^= (uint32_t)*bytes;
		hash *= FNV_PRIME;
    }

	return hash;
}

/// @brief Get the next power of 2 greater than "num"
/// @param num Number whose next power of 2 to get
/// @return The next power of 2
/// @note The minimum number returned is HMAP_MIN_CAPACITY
static size_t next_pow2(size_t num)
{
	size_t pow2 = HMAP_MIN_CAPACITY;
    
    while (pow2 < num)
    {
        pow2 <<= 1;
    }

	return pow2;
}

/// @brief Finds the index that corresponds to the provided key
/// @param map Hash map to use
/// @param key Key of the entry to find
/// @return The index of the entry or -1 if it does not exist
static ptrdiff_t hmap_find(const hash_map_t* map, const char* key)
{
	uint32_t hash = map->hash_fn(key);
	size_t idx = (size_t)hash & (map->capacity - 1);

	for (size_t i = 0; i < map->capacity; i++)
    {
        hmap_slot_t* slot = &map->slots[idx];

        if (slot->state == SLOT_EMPTY)
        {
            return -1;
        }

        if (slot->state == SLOT_LIVE && strcmp(slot->key, key) == 0)
        {
            return (ptrdiff_t)idx;
        }

        idx = (idx + 1) & (map->capacity - 1);
    }

	return -1;
}

/// @brief Finds the index a value should be inserted in
/// @param map Hash map to use
/// @param key Key of the entry whose slot to find
/// @return The index to insert in
static size_t hmap_find_insert_slot(const hash_map_t* map, const char* key)
{
    uint32_t hash = map->hash_fn(key);
    size_t idx = hash & (map->capacity - 1);
    ptrdiff_t first_tomb = -1;

    for (size_t i = 0; i < map->capacity; i++)
    {
        hmap_slot_t* slot = &map->slots[idx];

        if (slot->state == SLOT_LIVE && strcmp(slot->key, key) == 0)
        {
            return idx;
        }
        else if (slot->state == SLOT_DELETED && first_tomb == -1)
        {
            first_tomb = (ptrdiff_t)idx;
        }
        else if (slot->state == SLOT_EMPTY)
        {
            return (first_tomb != -1) ? (size_t)first_tomb : idx;
        }

        idx = (idx + 1) & (map->capacity - 1);
    }

    return (size_t)first_tomb;
}

/// @brief Rebuilds a hash map, rehashing every value
/// @param map Hash map to be rebuilt
/// @param new_capacity New capacity of the hash map
/// @return True or false depending on the operation's success
/// @note On failure, the hash map is not modified
static bool hmap_rebuild(hash_map_t* map, size_t new_capacity)
{
    hmap_slot_t* new_slots = calloc(new_capacity, sizeof(hmap_slot_t));
    if (!new_slots)
    {
        return false;
    }

    for (size_t i = 0; i < map->capacity; i++)
    {
        hmap_slot_t* slot = &map->slots[i];

        if (slot->state == SLOT_LIVE)
        {
            uint32_t hash = map->hash_fn(slot->key);
            size_t idx = hash & (new_capacity - 1);

            while (new_slots[idx].state == SLOT_LIVE)
            {
                idx = (idx + 1) & (new_capacity - 1);
            }

            new_slots[idx].state = SLOT_LIVE;
            new_slots[idx].key = slot->key;
            new_slots[idx].value = slot->value;
        }
    }

    free(map->slots);

    map->slots = new_slots;
    map->capacity = new_capacity;
    map->tombstones = 0;

    return true;
}

/// @brief Rehashes a hash map
/// @param map Hash map to rehash
/// @return True or false depending on the operation's success
/// @see hmap_rebuild
static bool hmap_rehash(hash_map_t* map)
{
    return hmap_rebuild(map, map->capacity);
}

/// @brief Resizes a hash map, rehashing every value
/// @param map Hash map to resize
/// @param new_capacity Capacity to resize the hash map to
/// @return True or false depending on the operation's success
/// @see hmap_rebuild
static bool hmap_resize(hash_map_t* map, size_t new_capacity)
{
    return hmap_rebuild(map, new_capacity);
}

// Implementation

hash_map_t* hmap_new(size_t capacity, free_fn_t free_fn, hash_fn_t hash_fn)
{
    hash_map_t* map = malloc(sizeof(hash_map_t));
    if (!map)
    {
        return NULL;
    }

    map->capacity = next_pow2(capacity);

	map->slots = calloc(map->capacity, sizeof(hmap_slot_t));
	if (!map->slots)
	{
		free(map);
		return NULL;
	}
	
	map->count = 0;
	map->tombstones = 0;
	map->free_fn = free_fn;
    map->hash_fn = hash_fn ? hash_fn : fnv1a;

	return map;
}

hash_map_t* hmap_new_default(size_t capacity)
{
    return hmap_new(capacity, NULL, NULL);
}

void hmap_free(hash_map_t* map)
{
    if (!map)
    {
        return;
    }

    for (size_t i = 0; i < map->capacity; i++)
    {
        hmap_slot_t* slot = &map->slots[i];

        if (slot->state == SLOT_LIVE)
        {
            free(slot->key);
            if (map->free_fn)
            {
                map->free_fn(slot->value);
            }
        }
    }

    free(map->slots);
    free(map);
}

bool hmap_put(hash_map_t* map, const char* key, void* value)
{
    if (!map || !key)
    {
        return false;
    }

    if (map->tombstones > map->capacity * HMAP_TOMBSTONE_RATIO)
    {
        if (!hmap_rehash(map))
        {
            return false;
        }
    }

    size_t total_count = map->count + map->tombstones;
	double load = (double)(total_count) / (double)map->capacity;
	if (load >= HMAP_LOAD_FACTOR)
	{
		if (!hmap_resize(map, map->capacity << 1))
        {
			return false;
        }
	}

    size_t idx = hmap_find_insert_slot(map, key);
    hmap_slot_t* slot = &map->slots[idx];

    // Replace duplicate
	if (slot->state == SLOT_LIVE)
	{
        if (map->free_fn)
        {
            map->free_fn(slot->value);
        }
		slot->value = value;
		return true;
	}

    // Insert new
    size_t len = strlen(key) + 1;
    slot->key = malloc(len);
    if (!slot->key)
    {
        return false;
    }
    memcpy(slot->key, key, len);

    slot->value = value;
    if (slot->state == SLOT_DELETED)
    {
        map->tombstones--;
    }
    
    slot->state = SLOT_LIVE;
    map->count++;

    return true;
}

void* hmap_get(const hash_map_t* map, const char* key)
{
    if (!map || !key)
    {
        return NULL;
    }
	
    ptrdiff_t idx = hmap_find(map, key);
    if (idx == -1)
    {
        return NULL;
    }

    return map->slots[idx].value;
}

bool hmap_remove(hash_map_t* map, const char* key)
{
    if (!map || !key)
    {
        return false;
    }

    ptrdiff_t idx = hmap_find(map, key);
    if (idx == -1)
    {
        return false;
    }

	free(map->slots[idx].key);
	map->slots[idx].key = NULL;

	if (map->free_fn)
    {
		map->free_fn(map->slots[idx].value);
    }

	map->slots[idx].value = NULL;
	map->slots[idx].state = SLOT_DELETED;

	map->count--;
	map->tombstones++;
	return true;
}

void hmap_iterate(const hash_map_t* map, hmap_iter_fn_t fn, void* ctx)
{
    if (!map || !fn)
    {
        return;
    }

	for (size_t i = 0; i < map->capacity; i++)
    {
        hmap_slot_t* slot = &map->slots[i];

        if (slot->state == SLOT_LIVE)
        {
            fn(slot->key, slot->value, ctx);
        }
    }
}

size_t hmap_count(const hash_map_t* map)
{
	if (!map)
    {
		return 0;
    }

	return map->count;
}