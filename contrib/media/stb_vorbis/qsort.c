/*******************************************************************************
*
*  Author:  Remi Dufour - remi.dufour@gmail.com
*  Date:    July 23rd, 2012
*
*  Name:        Quicksort
*
*  Description: This is a well-known sorting algorithm developed by C. A. R. 
*               Hoare. It is a comparison sort and in this implementation,
*               is not a stable sort.
*
*  Note:        This is public-domain C implementation written from
*               scratch.  Use it at your own risk.
*
*******************************************************************************/

#include <limits.h>
#include <stddef.h>

/* Insertion sort threshold shift
 *
 * This macro defines the threshold shift (power of 2) at which the insertion
 * sort algorithm replaces the Quicksort.  A zero threshold shift disables the
 * insertion sort completely.
 *
 * The value is optimized for Linux and MacOS on the Intel x86 platform.
 */
#ifndef INSERTION_SORT_THRESHOLD_SHIFT
# ifdef __APPLE__ & __MACH__
#  define INSERTION_SORT_THRESHOLD_SHIFT 0
# else
#  define INSERTION_SORT_THRESHOLD_SHIFT 2
# endif
#endif

/* Macro SWAP
 *
 * Swaps the elements of two arrays.
 *
 * The length of the swap is determined by the value of "SIZE".  While both
 * arrays can't overlap, the case in which both pointers are the same works.
 */
#define SWAP(A,B,SIZE)                               \
    {                                                \
        register char       *a_byte = A;             \
        register char       *b_byte = B;             \
        register const char *a_end = a_byte + SIZE;  \
                                                     \
        while (a_byte < a_end)                       \
        {                                            \
            register const char swap_byte = *b_byte; \
            *b_byte++ = *a_byte;                     \
            *a_byte++ = swap_byte;                   \
        }                                            \
    }

/* Macro SWAP_NEXT
 *
 * Swaps the elements of an array with its next value.
 *
 * The length of the swap is determined by the value of "SIZE".  This macro
 * must be used at the beginning of a scope and "A" shouldn't be an expression.
 */
#define SWAP_NEXT(A,SIZE)                                 \
    register char       *a_byte = A;                      \
    register const char *a_end  = A + SIZE;               \
                                                          \
    while (a_byte < a_end)                                \
    {                                                     \
        register const char swap_byte = *(a_byte + SIZE); \
        *(a_byte + SIZE) = *a_byte;                       \
        *a_byte++ = swap_byte;                            \
    }

/* Function Quicksort
 *
 * This function performs a basic Quicksort.  This implementation is the
 * in-place version of the algorithm and is done in he following way:
 *
 * 1. In the middle of the array, we determine a pivot that we temporarily swap
 *    to the end.
 * 2. From the beginning to the end of the array, we swap any elements smaller
 *    than this pivot to the start, adjacent to other elements that were
 *    already moved.
 * 3. We swap the pivot next to these smaller elements.
 * 4. For both sub-arrays on sides of the pivot, we repeat this process
 *    recursively.
 * 5. For a sub-array smaller than a certain threshold, the insertion sort
 *    algorithm takes over.
 *
 * As an optimization, rather than performing a real recursion, we keep a
 * global stack to track boundaries for each recursion level.
 *
 * To ensure that at most O(log2 N) space is used, we recurse into the smaller
 * partition first.  The log2 of the highest unsigned value of an integer type
 * is the number of bits needed to store that integer. 
 */
void qsort(void   *array,
               size_t  length,
               size_t  size,
               int(*compare)(const void *, const void *))
{
    /* Recursive stacks for array boundaries (both inclusive) */
    struct stackframe
    {
        void *left;
        void *right;
    } stack[CHAR_BIT * sizeof(void *)];

    /* Recursion level */
    struct stackframe *recursion = stack;

#if INSERTION_SORT_THRESHOLD_SHIFT != 0
    /* Insertion sort threshold */
    const int threshold = size << INSERTION_SORT_THRESHOLD_SHIFT;
#endif

    /* Assign the first recursion level of the sorting */
    recursion->left = array;
    recursion->right = (char *)array + size * (length - 1);

    do
    {
        /* Partition the array */
        register char *index = recursion->left;
        register char *right = recursion->right;
        char          *left  = index;

        /* Assigning store to the left */
        register char *store = index;

        /* Pop the stack */
        --recursion;

        /* Determine a pivot (in the middle) and move it to the end */
        const size_t middle = (right - left) >> 1;
        SWAP(left + middle - middle % size,right,size)

        /* From left to right */
        while (index < right)
        {
            /* If item is smaller than pivot */
            if (compare(right, index) > 0)
            {
                /* Swap item and store */
                SWAP(index,store,size)

                /* We increment store */
                store += size;
            }

            index += size;
        }

        /* Move the pivot to its final place */
        SWAP(right,store,size)

/* Performs a recursion to the left */
#define RECURSE_LEFT                     \
    if (left < store - size)             \
    {                                    \
        (++recursion)->left = left;      \
        recursion->right = store - size; \
    }

/* Performs a recursion to the right */
#define RECURSE_RIGHT                       \
    if (store + size < right)               \
    {                                       \
        (++recursion)->left = store + size; \
        recursion->right = right;           \
    }

/* Insertion sort inner-loop */
#define INSERTION_SORT_LOOP(LEFT)                                 \
    {                                                             \
        register char *trail = index - size;                      \
        while (trail >= LEFT && compare(trail, trail + size) > 0) \
        {                                                         \
            SWAP_NEXT(trail,size)                                 \
            trail -= size;                                        \
        }                                                         \
    }

/* Performs insertion sort left of the pivot */
#define INSERTION_SORT_LEFT                                \
    for (index = left + size; index < store; index +=size) \
        INSERTION_SORT_LOOP(left)

/* Performs insertion sort right of the pivot */
#define INSERTION_SORT_RIGHT                                        \
    for (index = store + (size << 1); index <= right; index +=size) \
        INSERTION_SORT_LOOP(store + size)

/* Sorts to the left */
#if INSERTION_SORT_THRESHOLD_SHIFT == 0
# define SORT_LEFT RECURSE_LEFT
#else
# define SORT_LEFT                 \
    if (store - left <= threshold) \
    {                              \
        INSERTION_SORT_LEFT        \
    }                              \
    else                           \
    {                              \
        RECURSE_LEFT               \
    }
#endif

/* Sorts to the right */
#if INSERTION_SORT_THRESHOLD_SHIFT == 0
# define SORT_RIGHT RECURSE_RIGHT
#else
# define SORT_RIGHT                 \
    if (right - store <= threshold) \
    {                               \
        INSERTION_SORT_RIGHT        \
    }                               \
    else                            \
    {                               \
        RECURSE_RIGHT               \
    }
#endif

        /* Recurse into the smaller partition first */
        if (store - left < right - store)
        {
            /* Left side is smaller */
            SORT_RIGHT
            SORT_LEFT

            continue;
        }

        /* Right side is smaller */
        SORT_LEFT
        SORT_RIGHT

#undef RECURSE_LEFT
#undef RECURSE_RIGHT
#undef INSERTION_SORT_LOOP
#undef INSERTION_SORT_LEFT
#undef INSERTION_SORT_RIGHT
#undef SORT_LEFT
#undef SORT_RIGHT
    }
    while (recursion >= stack);
}

#undef INSERTION_SORT_THRESHOLD_SHIFT
#undef SWAP
#undef SWAP_NEXT 