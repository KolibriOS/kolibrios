# cvec - partial `std::vector` implementation in C.
## Partial implementation of `std::vector`

Member functions table:

| Status | Name | Function or reason if not implemented |
| :---: | --- | --- |
| :heavy_check_mark: | `(constructor)` | `new` |
| :heavy_check_mark: | `(destructor)` | `free` |
| :heavy_check_mark: | `operator=` | `assign_other` |
| :heavy_check_mark: | `assign` | `assign_fill`, `assign_range` |
| :heavy_minus_sign: | `get_allocator` | No `allocator` objects in the language |
| :heavy_check_mark: | `at` | `at` |
| :heavy_check_mark: | `operator[]` | `[]` |
| :heavy_check_mark: | `front` | `front`, `front_p` |
| :heavy_check_mark: | `back` | `back`, `back_p` |
| :heavy_check_mark: | `data` | `data` |
| :heavy_check_mark: | `begin` | `begin` |
| :heavy_check_mark: | `cbegin` | `cbegin` |
| :heavy_check_mark: | `end` | `end` |
| :heavy_check_mark: | `cend` | `cend` |
| :heavy_minus_sign: | `rbegin` | No reverse iterators in the language |
| :heavy_minus_sign: | `crbegin` | No reverse iterators in the language |
| :heavy_minus_sign: | `rend` | No reverse iterators in the language |
| :heavy_minus_sign: | `crend` | No reverse iterators in the language |
| :heavy_check_mark: | `empty` | `empty` |
| :heavy_check_mark: | `size` | `size` |
| :heavy_check_mark: | `max_size` | `max_size` |
| :heavy_check_mark: | `reserve` | `reserve` |
| :heavy_check_mark: | `capacity` | `capacity` |
| :heavy_check_mark: | `shrink_to_fit` | `shrink_to_fit` |
| :heavy_check_mark: | `clear` | `clear` |
| :heavy_check_mark: | `insert` | `insert`, `insert_it` |
| :heavy_minus_sign: | `emplace` | I know no way to preserve the original signature |
| :heavy_check_mark: | `erase` | `erase` |
| :heavy_check_mark: | `push_back` | `push_back` |
| :heavy_minus_sign: | `emplace_back` | I know no way to preserve the original signature |
| :heavy_check_mark: | `pop_back` | `pop_back` |
| :heavy_check_mark: | `resize` | `resize` |
| :heavy_minus_sign: | `swap` | Would have n complexity in this implementation |

## Easy to use

To use the std::vector implementation for specified type they should be declared as follows:

```C
#define CVEC_TYPE TypeOfVectorElement
#include "cvec.h"

// ...

    TypeOfVectorElement *vec = cvec_TypeOfVectorElement_new(128);
    
    cvec_TypeOfVectorElement_push_back(&vec, value);
```

Also somewhere in the project the functinos should be instantiated as follows:

```C
#define CVEC_TYPE TypeOfVectorElement
#define CVEC_INST
#include "cvec.h"
```

## Allows using of custom allocators.

```C
#define CVEC_TYPE pchar
#define CVEC_INST
#define CVEC_MALLOC custom_malloc
#define CVEC_REALLOC custom_realloc
#define CVEC_FREE custom_free
#include "cvec.h"
```

## Allows handling of exceptional cases.

```C
#define CVEC_TYPE pchar
#define CVEC_INST
// Set Out Of Bounds handler
#define CVEC_OOBH(funcname, vec, index) printf("Out of bounds in %s (vec = %p, i = %d)", funcname, vec, index); abort();
#include "cvec.h"
```

## Has no fixed dependencies

Every function it uses may be overridden. More information about dependencies in [cvec.h](cvec.h).
