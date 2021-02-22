# CDict - a simple dictionary implementation in C
## Ready to use!

It may be used out of the box with key and value of type `CStr` (which is `char *` - zero-terminated string). Once it instantiated in some file using `CDICT_INST` definition:

```C
#define CDICT_INST
#include "cdict.h"
```

It may be then declared just using `#include`:

```C
#include "cdict.h"

int main() {
    CDict_CStr_CStr dict;
    if (!cdict_CStr_CStr_init(&dict)) {
    	printf("CDict returned error #%d", dict.error_code);
    	return 0;
    }
    cdict_CStr_CStr_add_vv(&dict, "key_a", "value_a", CDICT_REPLACE_EXIST);
    printf("[key_a] = \"%s\"\n", cdict_CStr_CStr_get_v(&dict, "key_a"));
}
```

## Easy to configure!

If you want to create a dictionary for other key types you should provide your own keys hashing and comparsion functions, in such case the instantiation of the library will look like this:

```C
#define CDICT_INST
#define CDICT_KEY_T MyType
#define CDICT_HASH_FN(pkey) my_hash(pkey)
#define CDICT_CMP_FN(pkey0, pkey1) my_cmp(pkey0, pkey1)
#include "cdict.h"

int my_cmp(MyType *pkey0, MyType *pkey1) {
    // Return `whatever_negative` if `key0 < key1`, `0` if `key0 == key1` and `whatever_positive` if `key0 > key1` 
}

unsigned long my_hash(MyType *key) {
    // Return the hash of the key
}
```

Then to use the new dictionary you only need to define key type before the header inclusion:

```C
#define CDICT_KEY_T MyType
#include "cdict.h"

// ...
    CDict_MyType_CStr dict;
    cdict_MyType_CStr_init(&dict);
// ...
```

If you want to specify the type of values - just define the type:

```C
#define CDICT_VAL_T MyValueType
```

And so on.

## Dependency-free!

Every single used dependency may be redefined:

```C
#define CDICT_ASSERT_FN(x) my_assert(x);
```

## Flexible!

May define user data to be used in overriden functions (for example - custom allocators):

```C
#define CDICT_USER_DATA_T UserData
#define CDICT_HASHTAB_ITEM_ALLOC_FN(cdict, size) item_alloc(cdict, size)
#define CDICT_HASHTAB_ITEM_FREE_FN(cdict, ptr) item_free(cdict, ptr)
#define CDICT_HASHTAB_ALLOC_FN(cdict, size) hashtab_alloc(cdict, size)
```

## Checkout

[The library](cdict.h).
