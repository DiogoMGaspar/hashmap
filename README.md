# Hash Map

A small, simple hash map implementation in C using open addressing and linear probing.

This project is primarily for personal use and learning, but it’s written to be reusable in other C projects.

## Features

* String keys
* Generic values
* Automatic resizing
* Iteration over all entries
* Optional:
  * Custom hash function
  * Custom value free function

## How to use

### Option A — Copy

Copy these files into your project:

```
src/hashmap.c
src/hashmap.h
```

Then include in your build:

```sh
gcc your_code.c path/to/hashmap.c -o your_program
```

### Option B — Git Submodule

```sh
git submodule add <your-repo-url> external/hashmap
```

Then compile with:

```sh
gcc your_code.c external/hashmap/src/hashmap.c -o your_program
```

Include in your code:

```c
#include "hashmap.h"
```

## Basic Example

```c
#include "hashmap.h"
#include <stdio.h>

int main(void)
{
    hash_map_t* map = hmap_new_default(16);

    hmap_put(map, "name", "Alice");
    hmap_put(map, "city", "Lisbon");

    printf("Name: %s\n", (char*)hmap_get(map, "name"));
    printf("City: %s\n", (char*)hmap_get(map, "city"));

    hmap_free(map);
    return 0;
}
```

## Notes

If a *free function* is provided when creating the hash map, the map takes ownership of the stored values. This function will be called whenever a value is removed, replaced, or when the map is freed. If no free function is provided, the user is responsible for managing the lifetime of stored values.

A custom *hash function* can be provided as well, though only string keys are supported. The default hash function is FNV-1a.

The values are not copied, only pointers are stored. On the other hand, the keys are copied internally.

This implementation is not thread-safe and is designed for simplicity over maximum performance.

## Running Tests

```sh
make test
```

This will:

* clone the Unity test framework
* build the test runner
* run all tests