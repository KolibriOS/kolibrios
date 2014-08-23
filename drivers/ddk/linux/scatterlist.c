/*
 * Copyright (C) 2007 Jens Axboe <jens.axboe@oracle.com>
 *
 * Scatterlist handling helpers.
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
 */
#include <linux/export.h>
#include <linux/scatterlist.h>

/**
 * sg_next - return the next scatterlist entry in a list
 * @sg:		The current sg entry
 *
 * Description:
 *   Usually the next entry will be @sg@ + 1, but if this sg element is part
 *   of a chained scatterlist, it could jump to the start of a new
 *   scatterlist array.
 *
 **/
struct scatterlist *sg_next(struct scatterlist *sg)
{
#ifdef CONFIG_DEBUG_SG
	BUG_ON(sg->sg_magic != SG_MAGIC);
#endif
    if (sg_is_last(sg))
        return NULL;

    sg++;
    if (unlikely(sg_is_chain(sg)))
            sg = sg_chain_ptr(sg);

    return sg;
}
EXPORT_SYMBOL(sg_next);

/**
 * sg_nents - return total count of entries in scatterlist
 * @sg:		The scatterlist
 *
 * Description:
 * Allows to know how many entries are in sg, taking into acount
 * chaining as well
 *
 **/
int sg_nents(struct scatterlist *sg)
{
	int nents;
	for (nents = 0; sg; sg = sg_next(sg))
		nents++;
	return nents;
}
EXPORT_SYMBOL(sg_nents);


/**
 * sg_last - return the last scatterlist entry in a list
 * @sgl:	First entry in the scatterlist
 * @nents:	Number of entries in the scatterlist
 *
 * Description:
 *   Should only be used casually, it (currently) scans the entire list
 *   to get the last entry.
 *
 *   Note that the @sgl@ pointer passed in need not be the first one,
 *   the important bit is that @nents@ denotes the number of entries that
 *   exist from @sgl@.
 *
 **/
struct scatterlist *sg_last(struct scatterlist *sgl, unsigned int nents)
{
#ifndef ARCH_HAS_SG_CHAIN
	struct scatterlist *ret = &sgl[nents - 1];
#else
	struct scatterlist *sg, *ret = NULL;
	unsigned int i;

	for_each_sg(sgl, sg, nents, i)
		ret = sg;

#endif
#ifdef CONFIG_DEBUG_SG
	BUG_ON(sgl[0].sg_magic != SG_MAGIC);
	BUG_ON(!sg_is_last(ret));
#endif
	return ret;
}
EXPORT_SYMBOL(sg_last);

/**
 * sg_init_table - Initialize SG table
 * @sgl:	   The SG table
 * @nents:	   Number of entries in table
 *
 * Notes:
 *   If this is part of a chained sg table, sg_mark_end() should be
 *   used only on the last table part.
 *
 **/
void sg_init_table(struct scatterlist *sgl, unsigned int nents)
{
    memset(sgl, 0, sizeof(*sgl) * nents);
#ifdef CONFIG_DEBUG_SG
    {
            unsigned int i;
            for (i = 0; i < nents; i++)
                    sgl[i].sg_magic = SG_MAGIC;
    }
#endif
    sg_mark_end(&sgl[nents - 1]);
}
EXPORT_SYMBOL(sg_init_table);

/**
 * sg_init_one - Initialize a single entry sg list
 * @sg:		 SG entry
 * @buf:	 Virtual address for IO
 * @buflen:	 IO length
 *
 **/
//void sg_init_one(struct scatterlist *sg, const void *buf, unsigned int buflen)
//{
//   sg_init_table(sg, 1);
//   sg_set_buf(sg, buf, buflen);
//}
EXPORT_SYMBOL(sg_init_one);

/*
 * The default behaviour of sg_alloc_table() is to use these kmalloc/kfree
 * helpers.
 */
static struct scatterlist *sg_kmalloc(unsigned int nents, gfp_t gfp_mask)
{
	return kmalloc(nents * sizeof(struct scatterlist), gfp_mask);
}

static void sg_kfree(struct scatterlist *sg, unsigned int nents)
{
	kfree(sg);
}

/**
 * __sg_free_table - Free a previously mapped sg table
 * @table:	The sg table header to use
 * @max_ents:	The maximum number of entries per single scatterlist
 * @skip_first_chunk: don't free the (preallocated) first scatterlist chunk
 * @free_fn:	Free function
 *
 *  Description:
 *    Free an sg table previously allocated and setup with
 *    __sg_alloc_table().  The @max_ents value must be identical to
 *    that previously used with __sg_alloc_table().
 *
 **/
void __sg_free_table(struct sg_table *table, unsigned int max_ents,
		     bool skip_first_chunk, sg_free_fn *free_fn)
{
    struct scatterlist *sgl, *next;

    if (unlikely(!table->sgl))
            return;

    sgl = table->sgl;
    while (table->orig_nents) {
        unsigned int alloc_size = table->orig_nents;
        unsigned int sg_size;

        /*
         * If we have more than max_ents segments left,
         * then assign 'next' to the sg table after the current one.
         * sg_size is then one less than alloc size, since the last
         * element is the chain pointer.
         */
        if (alloc_size > max_ents) {
                next = sg_chain_ptr(&sgl[max_ents - 1]);
                alloc_size = max_ents;
                sg_size = alloc_size - 1;
        } else {
                sg_size = alloc_size;
                next = NULL;
        }

        table->orig_nents -= sg_size;
		if (!skip_first_chunk) {
			free_fn(sgl, alloc_size);
			skip_first_chunk = false;
		}
        sgl = next;
    }

    table->sgl = NULL;
}
EXPORT_SYMBOL(__sg_free_table);

/**
 * sg_free_table - Free a previously allocated sg table
 * @table:	The mapped sg table header
 *
 **/
void sg_free_table(struct sg_table *table)
{
	__sg_free_table(table, SG_MAX_SINGLE_ALLOC, false, sg_kfree);
}
EXPORT_SYMBOL(sg_free_table);

/**
 * __sg_alloc_table - Allocate and initialize an sg table with given allocator
 * @table:	The sg table header to use
 * @nents:	Number of entries in sg list
 * @max_ents:	The maximum number of entries the allocator returns per call
 * @gfp_mask:	GFP allocation mask
 * @alloc_fn:	Allocator to use
 *
 * Description:
 *   This function returns a @table @nents long. The allocator is
 *   defined to return scatterlist chunks of maximum size @max_ents.
 *   Thus if @nents is bigger than @max_ents, the scatterlists will be
 *   chained in units of @max_ents.
 *
 * Notes:
 *   If this function returns non-0 (eg failure), the caller must call
 *   __sg_free_table() to cleanup any leftover allocations.
 *
 **/
int __sg_alloc_table(struct sg_table *table, unsigned int nents,
		     unsigned int max_ents, struct scatterlist *first_chunk,
		     gfp_t gfp_mask, sg_alloc_fn *alloc_fn)
{
    struct scatterlist *sg, *prv;
    unsigned int left;

	memset(table, 0, sizeof(*table));

	if (nents == 0)
		return -EINVAL;
#ifndef ARCH_HAS_SG_CHAIN
	if (WARN_ON_ONCE(nents > max_ents))
		return -EINVAL;
#endif

    left = nents;
    prv = NULL;
    do {
        unsigned int sg_size, alloc_size = left;

        if (alloc_size > max_ents) {
                alloc_size = max_ents;
                sg_size = alloc_size - 1;
        } else
                sg_size = alloc_size;

        left -= sg_size;

		if (first_chunk) {
			sg = first_chunk;
			first_chunk = NULL;
		} else {
			sg = alloc_fn(alloc_size, gfp_mask);
		}
        if (unlikely(!sg)) {
                /*
                 * Adjust entry count to reflect that the last
                 * entry of the previous table won't be used for
                 * linkage.  Without this, sg_kfree() may get
                 * confused.
                 */
                if (prv)
                        table->nents = ++table->orig_nents;

 			return -ENOMEM;
        }

        sg_init_table(sg, alloc_size);
        table->nents = table->orig_nents += sg_size;

        /*
         * If this is the first mapping, assign the sg table header.
         * If this is not the first mapping, chain previous part.
         */
        if (prv)
                sg_chain(prv, max_ents, sg);
        else
                table->sgl = sg;

        /*
         * If no more entries after this one, mark the end
         */
        if (!left)
                sg_mark_end(&sg[sg_size - 1]);

        prv = sg;
    } while (left);

    return 0;
}
EXPORT_SYMBOL(__sg_alloc_table);

/**
 * sg_alloc_table - Allocate and initialize an sg table
 * @table:	The sg table header to use
 * @nents:	Number of entries in sg list
 * @gfp_mask:	GFP allocation mask
 *
 *  Description:
 *    Allocate and initialize an sg table. If @nents@ is larger than
 *    SG_MAX_SINGLE_ALLOC a chained sg table will be setup.
 *
 **/
int sg_alloc_table(struct sg_table *table, unsigned int nents, gfp_t gfp_mask)
{
	int ret;

	ret = __sg_alloc_table(table, nents, SG_MAX_SINGLE_ALLOC,
			       NULL, gfp_mask, sg_kmalloc);
	if (unlikely(ret))
		__sg_free_table(table, SG_MAX_SINGLE_ALLOC, false, sg_kfree);

	return ret;
}
EXPORT_SYMBOL(sg_alloc_table);




void __sg_page_iter_start(struct sg_page_iter *piter,
              struct scatterlist *sglist, unsigned int nents,
              unsigned long pgoffset)
{
    piter->__pg_advance = 0;
    piter->__nents = nents;

    piter->sg = sglist;
    piter->sg_pgoffset = pgoffset;
}
EXPORT_SYMBOL(__sg_page_iter_start);

static int sg_page_count(struct scatterlist *sg)
{
    return PAGE_ALIGN(sg->offset + sg->length) >> PAGE_SHIFT;
}

bool __sg_page_iter_next(struct sg_page_iter *piter)
{
    if (!piter->__nents || !piter->sg)
        return false;

    piter->sg_pgoffset += piter->__pg_advance;
    piter->__pg_advance = 1;

    while (piter->sg_pgoffset >= sg_page_count(piter->sg)) {
        piter->sg_pgoffset -= sg_page_count(piter->sg);
        piter->sg = sg_next(piter->sg);
        if (!--piter->__nents || !piter->sg)
            return false;
    }

    return true;
}
EXPORT_SYMBOL(__sg_page_iter_next);



