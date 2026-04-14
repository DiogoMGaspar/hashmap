#include "unity.h"
#include "hashmap.h"
#include <string.h>
#include <stdlib.h>

static int free_counter = 0;

static void counting_free(void* ptr)
{
    free_counter++;
    free(ptr);
}

static void iter(const char* key, void* value, void* ctx)
{
    (void)key;
    (void)value;
    int* c = (int*)ctx;
    (*c)++;
}

void setUp(void) {}
void tearDown(void) {}


void test_put_and_get(void)
{
    hash_map_t* map = hmap_new_default(16);

    TEST_ASSERT_TRUE(hmap_put(map, "key", "value"));
    TEST_ASSERT_EQUAL_PTR("value", hmap_get(map, "key"));

    hmap_free(map);
}

void test_get_nonexistent(void)
{
    hash_map_t* map = hmap_new_default(16);

    TEST_ASSERT_NULL(hmap_get(map, "nonexistent"));

    hmap_free(map);
}

void test_overwrite_value(void)
{
    hash_map_t* map = hmap_new_default(16);

    TEST_ASSERT_TRUE(hmap_put(map, "key", "value1"));
    TEST_ASSERT_TRUE(hmap_put(map, "key", "value2"));

    TEST_ASSERT_EQUAL_PTR("value2", hmap_get(map, "key"));
    TEST_ASSERT_EQUAL_UINT64(1, hmap_count(map));

    hmap_free(map);
}

void test_remove(void)
{
    hash_map_t* map = hmap_new_default(16);

    hmap_put(map, "key", "value");

    TEST_ASSERT_TRUE(hmap_remove(map, "key"));
    TEST_ASSERT_NULL(hmap_get(map, "key"));
    TEST_ASSERT_EQUAL_UINT64(0, hmap_count(map));

    hmap_free(map);
}

void test_remove_nonexistent(void)
{
    hash_map_t* map = hmap_new_default(16);

    TEST_ASSERT_FALSE(hmap_remove(map, "nonexistent"));

    hmap_free(map);
}

void test_count_multiple(void)
{
    hash_map_t* map = hmap_new_default(16);

    hmap_put(map, "a", "1");
    hmap_put(map, "b", "2");
    hmap_put(map, "c", "3");

    TEST_ASSERT_EQUAL_UINT64(3, hmap_count(map));

    hmap_free(map);
}

void test_null_inputs(void)
{
    hash_map_t* map = hmap_new_default(16);

    TEST_ASSERT_FALSE(hmap_put(NULL, "a", "b"));
    TEST_ASSERT_FALSE(hmap_put(map, NULL, "b"));

    TEST_ASSERT_NULL(hmap_get(NULL, "a"));
    TEST_ASSERT_NULL(hmap_get(map, NULL));

    TEST_ASSERT_FALSE(hmap_remove(NULL, "a"));
    TEST_ASSERT_FALSE(hmap_remove(map, NULL));

    hmap_free(map);
}

void test_resize(void)
{
    hash_map_t* map = hmap_new(2, free, NULL);

    for (int i = 0; i < 100; i++)
    {
        char key[16];
        sprintf(key, "key%d", i);

        char* dup = malloc(strlen(key) + 1);
        strcpy(dup, key);

        TEST_ASSERT_TRUE(hmap_put(map, key, dup));
    }

    for (int i = 0; i < 100; i++)
    {
        char key[16];
        sprintf(key, "key%d", i);

        TEST_ASSERT_NOT_NULL(hmap_get(map, key));
    }

    TEST_ASSERT_EQUAL_UINT64(100, hmap_count(map));

    hmap_free(map);
}

void test_tombstone_reuse(void)
{
    hash_map_t* map = hmap_new_default(16);

    hmap_put(map, "a", "1");
    hmap_put(map, "b", "2");

    TEST_ASSERT_TRUE(hmap_remove(map, "a"));

    // should reuse tombstone
    TEST_ASSERT_TRUE(hmap_put(map, "c", "3"));

    TEST_ASSERT_NOT_NULL(hmap_get(map, "c"));
    TEST_ASSERT_NULL(hmap_get(map, "a"));

    hmap_free(map);
}

void test_iteration(void)
{
    hash_map_t* map = hmap_new_default(16);

    hmap_put(map, "a", "1");
    hmap_put(map, "b", "2");
    hmap_put(map, "c", "3");

    int count = 0;

    hmap_iterate(map, iter, &count);

    TEST_ASSERT_EQUAL_INT(3, count);

    hmap_free(map);
}

void test_free_function_called(void)
{
    free_counter = 0;

    hash_map_t* map = hmap_new(16, counting_free, NULL);

    char* val1 = malloc(10);
    char* val2 = malloc(10);

    hmap_put(map, "a", val1);
    hmap_put(map, "b", val2);

    hmap_remove(map, "a");

    // one free from remove
    TEST_ASSERT_EQUAL_INT(1, free_counter);

    hmap_free(map);

    // second free from remaining element
    TEST_ASSERT_EQUAL_INT(2, free_counter);
}

void test_replace_calls_free(void)
{
    free_counter = 0;

    hash_map_t* map = hmap_new(16, counting_free, NULL);

    char* val1 = malloc(10);
    char* val2 = malloc(10);

    hmap_put(map, "a", val1);
    hmap_put(map, "a", val2); // overwrite

    TEST_ASSERT_EQUAL_INT(1, free_counter);

    hmap_free(map);

    TEST_ASSERT_EQUAL_INT(2, free_counter);
}

// --------------------------------------------------
// Main
// --------------------------------------------------

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_put_and_get);
    RUN_TEST(test_get_nonexistent);
    RUN_TEST(test_overwrite_value);
    RUN_TEST(test_remove);
    RUN_TEST(test_remove_nonexistent);
    RUN_TEST(test_count_multiple);
    RUN_TEST(test_null_inputs);
    RUN_TEST(test_resize);
    RUN_TEST(test_tombstone_reuse);
    RUN_TEST(test_iteration);
    RUN_TEST(test_free_function_called);
    RUN_TEST(test_replace_calls_free);

    return UNITY_END();
}