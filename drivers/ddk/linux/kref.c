#include <linux/kref.h>
#include <asm/atomic.h>


void kref_set(struct kref *kref, int num)
{
    atomic_set(&kref->refcount, num);
}

/**
 * kref_init - initialize object.
 * @kref: object in question.
 */
void kref_init(struct kref *kref)
{
    kref_set(kref, 1);
}

void kref_get(struct kref *kref)
{
//    WARN_ON(!atomic_read(&kref->refcount));
    atomic_inc(&kref->refcount);
}


int kref_put(struct kref *kref, void (*release)(struct kref *kref))
{
//    WARN_ON(release == NULL);
//    WARN_ON(release == (void (*)(struct kref *))kfree);

    if (atomic_dec_and_test(&kref->refcount)) {
        release(kref);
        return 1;
    }
    return 0;
}


