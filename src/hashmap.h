#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


/// @brief Function that frees the stored value
/// @param value Value to free
typedef void (*free_fn_t)(void* value);

/// @brief Function that generates a hash
/// @param key Key to be hashed
typedef uint32_t (*hash_fn_t)(const char* key);

/// @brief Function to call for each live entry in hmap_iterate
/// @param key Key of the entry
/// @param value Entry that corresponds to the provided key
/// @param ctx Additional caller-supplied context
typedef void (*hmap_iter_fn_t)(const char* key, void* value, void* ctx);


/// @brief State of a slot
typedef enum slot_state_e
{
	SLOT_EMPTY,     // Never used
	SLOT_LIVE,      // Holds a valid value
	SLOT_DELETED,   // Removed value/Tombstone
} slot_state_t;

/// @brief Slot of a hash map
typedef struct hmap_slot_s
{
	slot_state_t    state;      // State of the slot
	char*           key;        // Heap-allocated copy of the key
	void*           value;      // Stored value
} hmap_slot_t;

/// @brief Generic open-addressing hash map
/// @note Uses FNV-1a hashing
typedef struct hash_map_s
{
	hmap_slot_t*    slots;      // Slots of the hash map
	size_t			capacity;   // Total number of slots
	size_t			count;      // Number of used slots
	size_t			tombstones; // Number of deleted slots
    free_fn_t       free_fn;    // Function to use when freeing a value
    hash_fn_t       hash_fn;    // Function to use when hashing a value
} hash_map_t;


/// @brief Allocates a new empty hash map
/// @param capacity Minimum capacity the hash map should have
/// @param free_fn Function to use when freeing a value
/// @param hash_fn Function to use when hashing a value
/// @return The new hash map or NULL if it fails
/// @note HMAP_MIN_CAPACITY is the minimum capacity allowed
/// @note The allocated capacity will be a power of 2 greater than @capacity
/// @note Uses FNV-1a hashing by default
hash_map_t* hmap_new(size_t capacity, free_fn_t free_fn, hash_fn_t hash_fn);

/// @brief Allocates a new empty hash map
/// @param capacity Minimum capacity the hash map should have
/// @return The new hash map or NULL if it fails
hash_map_t* hmap_new_default(size_t capacity);

/// @brief Frees the memory owned by the hash map
/// @param map Hash map to free
void hmap_free(hash_map_t* map);

/// @brief Inserts a new entry into the hash map
/// @param map Hash map to insert the entry in
/// @param key Key of the entry to insert
/// @param value Value to be inserted
/// @return True or false depending on the operation's success
/// @note If a key already exists, it is replaced
/// @note Failure means an allocation failed
bool hmap_put(hash_map_t* map, const char* key, void* value);

/// @brief Returns the entry associated with the provided key
/// @param map Hash map whose entry to get
/// @param key Key of the entry
/// @return The entry needed or NULL if it does not exist
void* hmap_get(const hash_map_t* map, const char* key);

/// @brief Removes an entry from the hashmap
/// @param map Hash map to remove the entry from
/// @param key Key of the entry to remove
/// @return True if an entry was removed, otherwise false
bool hmap_remove(hash_map_t* map, const char* key);

/// @brief Iterates over the hash map, calling a function for every live entry
/// @param map Hash map to iterate over
/// @param fn Function to call
/// @param ctx Context to forward to each function call
/// @note Do not mutate the hashmap from within @fn
void hmap_iterate(const hash_map_t* map, hmap_iter_fn_t fn, void* ctx);

/// @brief Returns the number of live entries in a hash map
/// @param map Hash map whose count to get
/// @return The number of live entries in the hash map
size_t hmap_count(const hash_map_t* map);

#endif