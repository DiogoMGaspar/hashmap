# Hash Map

A small, simple hash map implementation in C using open addressing and linear probing.

This project is primarily for personal use and learning, but it’s written to be reusable in other C projects.

## Features

* String keys (`const char*`)
* Generic values (`void*`)
* Open addressing with linear probing
* Automatic resizing
* Iteration over all entries
* Optional:
  * Custom hash function
  * Custom value free function

## Usage

### 1. Add to Your Project

#### Option A — Copy

Copy these files into your project:

```
src/hashmap.c
src/hashmap.h
```

Then include in your build:

```sh
gcc your_code.c hashmap.c -o your_program
```

#### Option B — Git Submodule

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
#include <stdlib.h>

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

If a *free function* is provided when creating the hash map, it effectively owns the data added to it, and will call this function every time a deletion is necessary. Otherwise, the values are never directly freed by the hash map.

A custom *hash function* can be provided as well, though only string keys are supported. The default hash function is FNV1-a.

The values are not copied, only pointers are stored. On the other hand, the keys are copied internally.

This implementation is not thread-safe, nor is it particularly performant.

## Running Tests

```sh
make test
```

This will:

* clone the Unity test framework
* build the test runner
* run all tests