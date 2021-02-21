// Copyright (c) 2015 Evan Teran
// Copyright (c) 2020 Magomed Kostoev
//
// You may use, distribute and modify this code under the terms of the MIT license.
//
// You should have received a copy of the MIT license with this file. If not, please visit
// https://opensource.org/licenses/MIT for full license details.

// cvec.h - std::vector (ish) implementation in C. Based on https://github.com/eteran/c-vector/.
//
// Unlike a real std::vector this one is implemented as a fat array, so metadata is placed inside
// an allocated buffer itself.
//
// Configuration (definitions):
// CVEC_TYPE:    Type of the vector's elements, after instantiation these functions will be visible
//               as cvec_<CVEC_TYPE>_funcname, so no stars and subscripting marks allowed - named
//               types only
// CVEC_INST:    Instantiate the functions if defined
// CVEC_LOGG:    Multiply capacity by CVEC_LOGG each expansion if defined (should be >= 1)
// CVEC_ASSERT:  Replacement for assert from <assert.h>
// CVEC_MALLOC:  Replacement for malloc from <stdlib.h>
// CVEC_REALLOC: Replacement for realloc from <stdlib.h>
// CVEC_FREE:    Replacement for free from <stdlib.h>
// CVEC_OOBH:    Out-of-bounds handler (gets __func__, vector data address and index of overflow)
// CVEC_OOBVAL:  Default value to return on out of bounds access
//
// Minimal definitions for declaration: CVEC_TYPE
// Minimal definitions for instantiation: CVEC_TYPE, CVEC_INST, CVEC_OOBVAL if the type object
// can't be represented by 0 value.
//
// WARNING: All used definitions will be undefined on header exit.
//
// Dependencies:
// <stddef.h> or another source of size_t and ptrdiff_t
// <stdint.h> or another source of SIZE_MAX
// <stdlib.h> or another source of malloc, calloc and realloc
// <assert.h> or another source of assert

//
// Input macros
//

#ifndef CVEC_LOGG
#   define CVEC_LOGG 1.5
#endif
#ifndef CVEC_ASSERT
#   define CVEC_ASSERT(x) assert(x)
#endif
#ifndef CVEC_MALLOC
#   define CVEC_MALLOC(size) malloc(size)
#endif
#ifndef CVEC_REALLOC
#   define CVEC_REALLOC(ptr, size) realloc(ptr, size)
#endif
#ifndef CVEC_FREE
#   define CVEC_FREE(size) free(size)
#endif
#ifndef CVEC_OOBH
#   define CVEC_OOBH(funcname, vec, index)
#endif
#ifndef CVEC_OOBVAL
#   define CVEC_OOBVAL { 0 }
#endif

//
// Internal macros
//

#define CVEC_CONCAT2_IMPL(x, y) cvec_ ## x ## _ ## y
#define CVEC_CONCAT2(x, y) CVEC_CONCAT2_IMPL(x, y)

/// Creates method name according to CVEC_TYPE
#define CVEC_FUN(name) CVEC_CONCAT2(CVEC_TYPE, name)

#define cvec_x_new CVEC_FUN(new)
#define cvec_x_capacity CVEC_FUN(capacity)
#define cvec_x_size CVEC_FUN(size)
#define cvec_x_empty CVEC_FUN(empty)
#define cvec_x_pop_back CVEC_FUN(pop_back)
#define cvec_x_erase CVEC_FUN(erase)
#define cvec_x_free CVEC_FUN(free)
#define cvec_x_begin CVEC_FUN(begin)
#define cvec_x_cbegin CVEC_FUN(cbegin)
#define cvec_x_end CVEC_FUN(end)
#define cvec_x_cend CVEC_FUN(cend)
#define cvec_x_push_back CVEC_FUN(push_back)
#define cvec_x_at CVEC_FUN(at)
#define cvec_x_reserve CVEC_FUN(reserve)
#define cvec_x_shrink_to_fit CVEC_FUN(shrink_to_fit)
#define cvec_x_assign_fill CVEC_FUN(assign_fill)
#define cvec_x_assign_range CVEC_FUN(assign_range)
#define cvec_x_assign_other CVEC_FUN(assign_other)
#define cvec_x_data CVEC_FUN(data)
#define cvec_x_resize CVEC_FUN(resize)
#define cvec_x_resize_v CVEC_FUN(resize_v)
#define cvec_x_clear CVEC_FUN(clear)
#define cvec_x_front CVEC_FUN(front)
#define cvec_x_front_p CVEC_FUN(front_p)
#define cvec_x_back CVEC_FUN(back)
#define cvec_x_back_p CVEC_FUN(back_p)
#define cvec_x_max_size CVEC_FUN(max_size)
#define cvec_x_insert CVEC_FUN(insert)
#define cvec_x_insert_it CVEC_FUN(insert_it)

#define cvec_x_grow CVEC_FUN(grow)
#define cvec_x_set_capacity CVEC_FUN(set_capacity)
#define cvec_x_set_size CVEC_FUN(set_size)

//
// External declarations
//

/// Allocates new vector of specified capacity.
CVEC_TYPE *cvec_x_new(size_t count);

/// Gets the current capacity of the vector.
size_t cvec_x_capacity(CVEC_TYPE **vec);

/// Gets the current size of the vector.
size_t cvec_x_size(CVEC_TYPE **vec);

/// Returns non-zero if the vector is empty.
int cvec_x_empty(CVEC_TYPE **vec);

/// Removes the last element from the vector.
void cvec_x_pop_back(CVEC_TYPE **vec);

/// Removes the element at index i from the vector.
void cvec_x_erase(CVEC_TYPE **vec, size_t i);

/// Frees all memory associated with the vector.
void cvec_x_free(CVEC_TYPE **vec);

/// Returns an iterator to first element of the vector.
CVEC_TYPE *cvec_x_begin(CVEC_TYPE **vec);

/// Returns a const iterator to first element of the vector
const CVEC_TYPE *cvec_x_cbegin(CVEC_TYPE **vec);

/// Returns an iterator to one past the last element of the vector.
CVEC_TYPE *cvec_x_end(CVEC_TYPE **vec);

/// Returns a const iterator to one past the last element of the vector.
const CVEC_TYPE *cvec_x_cend(CVEC_TYPE **vec);

/// Adds an element to the end of the vector.
void cvec_x_push_back(CVEC_TYPE **vec, CVEC_TYPE value);

/// Gets element with bounds checking. On out of bounds calls CVEC_OOBH and returns CVEC_OOBVAL.
CVEC_TYPE cvec_x_at(CVEC_TYPE **vec, size_t i);

/// Increases the capacity of the vector to a value that's equal to new_cap.
void cvec_x_reserve(CVEC_TYPE **vec, size_t new_cap);

/// Requests the removal of unused capacity.
void cvec_x_shrink_to_fit(CVEC_TYPE **vec);

/// Replaces the contents with count copies of value value.
void cvec_x_assign_fill(CVEC_TYPE **vec, size_t count, CVEC_TYPE value);

/// Replaces the contents with data from range [first, last).
void cvec_x_assign_range(CVEC_TYPE **vec, CVEC_TYPE *first, CVEC_TYPE *last);

/// Replaces the contents with contetns of other.
void cvec_x_assign_other(CVEC_TYPE **vec, CVEC_TYPE **other);

/// Gives direct access to buffer.
CVEC_TYPE *cvec_x_data(CVEC_TYPE **vec);

/// Resizes the container to contain count elements.
void cvec_x_resize(CVEC_TYPE **vec, size_t new_size);

/// Resizes the container to contain count elements, initializes new elements by value.
void cvec_x_resize_v(CVEC_TYPE **vec, size_t new_size, CVEC_TYPE value);

/// Erases all elements from the container.
void cvec_x_clear(CVEC_TYPE **vec);

/// Returns the first element of the vector.
CVEC_TYPE cvec_x_front(CVEC_TYPE **vec);

/// Returns a pointer to the first element of the vector.
CVEC_TYPE *cvec_x_front_p(CVEC_TYPE **vec);

/// Returns the last element of the vector.
CVEC_TYPE cvec_x_back(CVEC_TYPE **vec);

/// Returns a pointer to the last element of the vector.
CVEC_TYPE *cvec_x_back_p(CVEC_TYPE **vec);

/// Returns maximal size of the vector.
size_t cvec_x_max_size(CVEC_TYPE **vec);

/// Inserts a value into vector by index.
CVEC_TYPE *cvec_x_insert(CVEC_TYPE **vec, size_t index, CVEC_TYPE value);

/// Inserts a value into vector by iterator (pointer in vector).
CVEC_TYPE *cvec_x_insert_it(CVEC_TYPE **vec, CVEC_TYPE *it, CVEC_TYPE value);

//
// Function definitions
//

#ifdef CVEC_INST

/// Ensures that the vector is at least <count> elements big.
static void cvec_x_grow(CVEC_TYPE **vec, size_t count);

/// Sets the capacity variable of the vector.
static void cvec_x_set_capacity(CVEC_TYPE **vec, size_t size);

/// Sets the size variable of the vector.
static void cvec_x_set_size(CVEC_TYPE **vec, size_t size);

//
// Public functions
//

CVEC_TYPE *cvec_x_new(size_t count) {
    const size_t cv_sz = count * sizeof(CVEC_TYPE) + sizeof(size_t) * 2;
    size_t *cv_p = CVEC_MALLOC(cv_sz);
    CVEC_ASSERT(cv_p);
    CVEC_TYPE *vec = (void *)(&cv_p[2]);
    cvec_x_set_capacity(&vec, count);
    cvec_x_set_size(&vec, 0);
    return vec;
}

size_t cvec_x_capacity(CVEC_TYPE **vec) {
    CVEC_ASSERT(vec);
    return *vec ? ((size_t *)*vec)[-1] : (size_t)0;
}

size_t cvec_x_size(CVEC_TYPE **vec) {
    CVEC_ASSERT(vec);
    return *vec ? ((size_t *)*vec)[-2] : (size_t)0;
}

int cvec_x_empty(CVEC_TYPE **vec) {
    return cvec_x_size(vec) == 0;
}

void cvec_x_pop_back(CVEC_TYPE **vec) {
    cvec_x_set_size(vec, cvec_x_size(vec) - 1);
}

void cvec_x_erase(CVEC_TYPE **vec, size_t i) {
    CVEC_ASSERT(vec);
    if (*vec) {
        const size_t cv_sz = cvec_x_size(vec);
        if (i < cv_sz) {
            cvec_x_set_size(vec, cv_sz - 1);
            for (size_t cv_x = i; cv_x < (cv_sz - 1); ++cv_x) {
                (*vec)[cv_x] = (*vec)[cv_x + 1];
            }
        }
    }
}

void cvec_x_free(CVEC_TYPE **vec) {
    CVEC_ASSERT(vec);
    if (*vec) {
        size_t *p1 = &((size_t *)*vec)[-2];
        CVEC_FREE(p1);
    }
}

CVEC_TYPE *cvec_x_begin(CVEC_TYPE **vec) {
    CVEC_ASSERT(vec);
    return *vec;
}

const CVEC_TYPE *cvec_x_cbegin(CVEC_TYPE **vec) {
    return cvec_x_begin(vec);
}

CVEC_TYPE *cvec_x_end(CVEC_TYPE **vec) {
    CVEC_ASSERT(vec);
    return *vec ? &((*vec)[cvec_x_size(vec)]) : NULL;
}

const CVEC_TYPE *cvec_x_cend(CVEC_TYPE **vec) {
    return cvec_x_end(vec);
}

void cvec_x_push_back(CVEC_TYPE **vec, CVEC_TYPE value) {
    CVEC_ASSERT(vec);
    size_t cv_cap = cvec_x_capacity(vec);
    if (cv_cap <= cvec_x_size(vec)) {
        cvec_x_grow(vec, cv_cap * CVEC_LOGG + 1);
    }
    (*vec)[cvec_x_size(vec)] = value;
    cvec_x_set_size(vec, cvec_x_size(vec) + 1);
}

CVEC_TYPE cvec_x_at(CVEC_TYPE **vec, size_t i) {
    CVEC_ASSERT(vec);
    if (i >= cvec_x_size(vec) || i < 0) {
        CVEC_OOBH(__func__, vec, i);
        CVEC_TYPE ret = CVEC_OOBVAL;
        return ret;
    }
    return (*vec)[i];
}

void cvec_x_reserve(CVEC_TYPE **vec, size_t new_cap) {
    if (new_cap <= cvec_x_capacity(vec)) {
        return;
    }
    cvec_x_grow(vec, new_cap);
}

void cvec_x_shrink_to_fit(CVEC_TYPE **vec) {
    if (cvec_x_capacity(vec) > cvec_x_size(vec)) {
        cvec_x_grow(vec, cvec_x_size(vec));
    }
}

void cvec_x_assign_fill(CVEC_TYPE **vec, size_t count, CVEC_TYPE value) {
    CVEC_ASSERT(vec);
    cvec_x_reserve(vec, count);
    cvec_x_set_size(vec, count); // If the buffer was bigger than new_cap, set size ourselves
    for (size_t i = 0; i < count; i++) {
        (*vec)[i] = value;
    }
}

void cvec_x_assign_range(CVEC_TYPE **vec, CVEC_TYPE *first, CVEC_TYPE *last) {
    CVEC_ASSERT(vec);
    size_t new_size = ((ptrdiff_t)(last - first)) / sizeof(*first);
    cvec_x_reserve(vec, new_size);
    cvec_x_set_size(vec, new_size);
    size_t i = 0;
    for (CVEC_TYPE *it = first; it < last; it++, i++) {
        (*vec)[i] = *it;
    }
}

void cvec_x_assign_other(CVEC_TYPE **vec, CVEC_TYPE **other) {
    cvec_x_assign_range(vec, cvec_x_begin(other), cvec_x_end(other));
}

CVEC_TYPE *cvec_x_data(CVEC_TYPE **vec) {
    CVEC_ASSERT(vec);
    return (*vec);
}

void cvec_x_resize(CVEC_TYPE **vec, size_t count) {
    CVEC_TYPE value = { 0 };
    cvec_x_resize_v(vec, count, value);
}

void cvec_x_resize_v(CVEC_TYPE **vec, size_t count, CVEC_TYPE value) {
    CVEC_ASSERT(vec);
    size_t old_size = cvec_x_size(vec);
    cvec_x_set_size(vec, count);
    if (cvec_x_capacity(vec) < count) {
        cvec_x_reserve(vec, count);
        for (CVEC_TYPE *it = (*vec) + old_size; it < cvec_x_end(vec); it++) {
            *it = value;
        }
    }
}

void cvec_x_clear(CVEC_TYPE **vec) {
    cvec_x_set_size(vec, 0);
}

CVEC_TYPE cvec_x_front(CVEC_TYPE **vec) {
    CVEC_ASSERT(vec);
    return (*vec)[0];
}

CVEC_TYPE *cvec_x_front_p(CVEC_TYPE **vec) {
    CVEC_ASSERT(vec);
    return (*vec);
}

CVEC_TYPE cvec_x_back(CVEC_TYPE **vec) {
    return cvec_x_end(vec)[-1];
}

CVEC_TYPE *cvec_x_back_p(CVEC_TYPE **vec) {
    return cvec_x_end(vec) - 1;
}

size_t cvec_x_max_size(CVEC_TYPE **vec) {
    return SIZE_MAX / sizeof(**vec);
}

CVEC_TYPE *cvec_x_insert(CVEC_TYPE **vec, size_t index, CVEC_TYPE value) {
    CVEC_ASSERT(vec);
    if (index > cvec_x_size(vec) || index < 0) {
        return NULL; // TODO: What?
    }
    size_t new_size = cvec_x_size(vec) + 1;
    cvec_x_reserve(vec, new_size);
    cvec_x_set_size(vec, new_size);
    CVEC_TYPE *ret = *vec + index;
    for (CVEC_TYPE *it = cvec_x_back_p(vec); it > ret; it--) {
        *it = it[-1];
    }
    *ret = value;
    return ret;
}

CVEC_TYPE *cvec_x_insert_it(CVEC_TYPE **vec, CVEC_TYPE *it, CVEC_TYPE value) {
    CVEC_ASSERT(vec);
    size_t index = (it - *vec) / sizeof(**vec);
    return cvec_x_insert(vec, index, value);
}

//
// Private functions
//

static void cvec_x_set_capacity(CVEC_TYPE **vec, size_t size) {
    CVEC_ASSERT(vec);
    if (*vec) {
        ((size_t *)*vec)[-1] = size;
    }
}

static void cvec_x_set_size(CVEC_TYPE **vec, size_t size) {
    CVEC_ASSERT(vec);
    if (*vec) {
        ((size_t *)*vec)[-2] = size;
    }
}

static void cvec_x_grow(CVEC_TYPE **vec, size_t count) {
    CVEC_ASSERT(vec);
    const size_t cv_sz = count * sizeof(**vec) + sizeof(size_t) * 2;
    size_t *cv_p1 = &((size_t *)*vec)[-2];
    size_t *cv_p2 = CVEC_REALLOC(cv_p1, (cv_sz));
    CVEC_ASSERT(cv_p2);
    *vec = (void *)(&cv_p2[2]);
    cvec_x_set_capacity(vec, count);
}

#endif

#undef CVEC_TYPE

#ifdef CVEC_INST
#   undef CVEC_INST
#   ifdef CVEC_LOGG
#       undef CVEC_LOGG
#   endif
#   ifdef CVEC_OOBH
#       undef CVEC_OOBH
#   endif
#   ifdef CVEC_OOBVAL
#       undef CVEC_OOBVAL
#   endif
#   undef CVEC_ASSERT
#   undef CVEC_MALLOC
#   undef CVEC_REALLOC
#   undef CVEC_FREE
#endif

#undef CVEC_CONCAT2_IMPL
#undef CVEC_CONCAT2

#undef CVEC_FUN

#undef cvec_x_new
#undef cvec_x_capacity
#undef cvec_x_size
#undef cvec_x_empty
#undef cvec_x_pop_back
#undef cvec_x_erase
#undef cvec_x_free
#undef cvec_x_begin
#undef cvec_x_cbegin
#undef cvec_x_end
#undef cvec_x_cend
#undef cvec_x_push_back
#undef cvec_x_at
#undef cvec_x_reserve
#undef cvec_x_shrink_to_fit
#undef cvec_x_assign_fill
#undef cvec_x_assign_range
#undef cvec_x_assign_other
#undef cvec_x_data
#undef cvec_x_resize
#undef cvec_x_resize_v
#undef cvec_x_clear
#undef cvec_x_front
#undef cvec_x_front_p
#undef cvec_x_back
#undef cvec_x_back_p
#undef cvec_x_max_size
#undef cvec_x_insert
#undef cvec_x_insert_it
#undef cvec_x_grow
#undef cvec_x_set_capacity
#undef cvec_x_set_size
