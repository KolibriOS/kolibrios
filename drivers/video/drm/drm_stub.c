/*
 * Created: Fri Jan 19 10:48:35 2001 by faith@acm.org
 *
 * Copyright 2001 VA Linux Systems, Inc., Sunnyvale, California.
 * All Rights Reserved.
 *
 * Author Rickard E. (Rik) Faith <faith@valinux.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <drm/drmP.h>
#include <drm/drm_core.h>
#include "drm_internal.h"

unsigned int drm_debug = 0;	/* 1 to enable debug output */
EXPORT_SYMBOL(drm_debug);

unsigned int drm_rnodes = 0;	/* 1 to enable experimental render nodes API */
EXPORT_SYMBOL(drm_rnodes);

/* 1 to allow user space to request universal planes (experimental) */
unsigned int drm_universal_planes = 0;
EXPORT_SYMBOL(drm_universal_planes);

unsigned int drm_vblank_offdelay = 5000;    /* Default to 5000 msecs. */
EXPORT_SYMBOL(drm_vblank_offdelay);

unsigned int drm_timestamp_precision = 20;  /* Default to 20 usecs. */
EXPORT_SYMBOL(drm_timestamp_precision);

struct idr drm_minors_idr;

void drm_err(const char *format, ...)
{
    struct va_format vaf;
    va_list args;

    va_start(args, format);

    vaf.fmt = format;
    vaf.va = &args;

    printk(KERN_ERR "[" DRM_NAME ":%pf] *ERROR* %pV",
           __builtin_return_address(0), &vaf);

    va_end(args);
}
EXPORT_SYMBOL(drm_err);

void drm_ut_debug_printk(const char *function_name, const char *format, ...)
{
	struct va_format vaf;
	va_list args;

//   if (drm_debug & request_level) {
//       if (function_name)
//           printk(KERN_DEBUG "[%s:%s], ", prefix, function_name);
//       va_start(args, format);
//       vprintk(format, args);
//       va_end(args);
//   }
}
EXPORT_SYMBOL(drm_ut_debug_printk);

#if 0
struct drm_master *drm_master_create(struct drm_minor *minor)
{
	struct drm_master *master;

	master = kzalloc(sizeof(*master), GFP_KERNEL);
	if (!master)
		return NULL;

	kref_init(&master->refcount);
	spin_lock_init(&master->lock.spinlock);
	init_waitqueue_head(&master->lock.lock_queue);
	if (drm_ht_create(&master->magiclist, DRM_MAGIC_HASH_ORDER)) {
		kfree(master);
		return NULL;
	}
	INIT_LIST_HEAD(&master->magicfree);
	master->minor = minor;

	return master;
}

struct drm_master *drm_master_get(struct drm_master *master)
{
	kref_get(&master->refcount);
	return master;
}
EXPORT_SYMBOL(drm_master_get);

static void drm_master_destroy(struct kref *kref)
{
	struct drm_master *master = container_of(kref, struct drm_master, refcount);
	struct drm_magic_entry *pt, *next;
	struct drm_device *dev = master->minor->dev;
	struct drm_map_list *r_list, *list_temp;

	mutex_lock(&dev->struct_mutex);
	if (dev->driver->master_destroy)
		dev->driver->master_destroy(dev, master);

	list_for_each_entry_safe(r_list, list_temp, &dev->maplist, head) {
		if (r_list->master == master) {
			drm_rmmap_locked(dev, r_list->map);
			r_list = NULL;
		}
	}

	if (master->unique) {
		kfree(master->unique);
		master->unique = NULL;
		master->unique_len = 0;
	}

	list_for_each_entry_safe(pt, next, &master->magicfree, head) {
		list_del(&pt->head);
		drm_ht_remove_item(&master->magiclist, &pt->hash_item);
		kfree(pt);
	}

	drm_ht_remove(&master->magiclist);

	mutex_unlock(&dev->struct_mutex);
	kfree(master);
}

void drm_master_put(struct drm_master **master)
{
	kref_put(&(*master)->refcount, drm_master_destroy);
	*master = NULL;
}
EXPORT_SYMBOL(drm_master_put);

int drm_setmaster_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file_priv)
{
	int ret = 0;

	mutex_lock(&dev->master_mutex);
	if (file_priv->is_master)
		goto out_unlock;

	if (file_priv->minor->master) {
		ret = -EINVAL;
		goto out_unlock;
	}

	if (!file_priv->master) {
		ret = -EINVAL;
		goto out_unlock;
	}

	file_priv->minor->master = drm_master_get(file_priv->master);
	file_priv->is_master = 1;
	if (dev->driver->master_set) {
		ret = dev->driver->master_set(dev, file_priv, false);
		if (unlikely(ret != 0)) {
			file_priv->is_master = 0;
			drm_master_put(&file_priv->minor->master);
		}
	}

out_unlock:
	mutex_unlock(&dev->master_mutex);
	return ret;
}

int drm_dropmaster_ioctl(struct drm_device *dev, void *data,
			 struct drm_file *file_priv)
{
	int ret = -EINVAL;

	mutex_lock(&dev->master_mutex);
	if (!file_priv->is_master)
		goto out_unlock;

	if (!file_priv->minor->master)
		goto out_unlock;

	ret = 0;
	if (dev->driver->master_drop)
		dev->driver->master_drop(dev, file_priv, false);
	drm_master_put(&file_priv->minor->master);
	file_priv->is_master = 0;

out_unlock:
	mutex_unlock(&dev->master_mutex);
	return ret;
}

/*
 * DRM Minors
 * A DRM device can provide several char-dev interfaces on the DRM-Major. Each
 * of them is represented by a drm_minor object. Depending on the capabilities
 * of the device-driver, different interfaces are registered.
 *
 * Minors can be accessed via dev->$minor_name. This pointer is either
 * NULL or a valid drm_minor pointer and stays valid as long as the device is
 * valid. This means, DRM minors have the same life-time as the underlying
 * device. However, this doesn't mean that the minor is active. Minors are
 * registered and unregistered dynamically according to device-state.
 */

static struct drm_minor **drm_minor_get_slot(struct drm_device *dev,
					     unsigned int type)
{
	switch (type) {
	case DRM_MINOR_LEGACY:
		return &dev->primary;
	case DRM_MINOR_RENDER:
		return &dev->render;
	case DRM_MINOR_CONTROL:
		return &dev->control;
	default:
		return NULL;
	}
}

static int drm_minor_alloc(struct drm_device *dev, unsigned int type)
{
	struct drm_minor *minor;

	minor = kzalloc(sizeof(*minor), GFP_KERNEL);
	if (!minor)
		return -ENOMEM;

	minor->type = type;
	minor->dev = dev;

	*drm_minor_get_slot(dev, type) = minor;
	return 0;
}

static void drm_minor_free(struct drm_device *dev, unsigned int type)
{
	struct drm_minor **slot;

	slot = drm_minor_get_slot(dev, type);
	if (*slot) {
		drm_mode_group_destroy(&(*slot)->mode_group);
		kfree(*slot);
		*slot = NULL;
	}
}

static int drm_minor_register(struct drm_device *dev, unsigned int type)
{
	struct drm_minor *new_minor;
	unsigned long flags;
	int ret;
	int minor_id;

	DRM_DEBUG("\n");

	new_minor = *drm_minor_get_slot(dev, type);
	if (!new_minor)
		return 0;

	idr_preload(GFP_KERNEL);
	spin_lock_irqsave(&drm_minor_lock, flags);
	minor_id = idr_alloc(&drm_minors_idr,
			     NULL,
			     64 * type,
			     64 * (type + 1),
			     GFP_NOWAIT);
	spin_unlock_irqrestore(&drm_minor_lock, flags);
	idr_preload_end();

	if (minor_id < 0)
		return minor_id;

	new_minor->index = minor_id;

	ret = drm_debugfs_init(new_minor, minor_id, drm_debugfs_root);
	if (ret) {
		DRM_ERROR("DRM: Failed to initialize /sys/kernel/debug/dri.\n");
		goto err_id;
	}

	ret = drm_sysfs_device_add(new_minor);
	if (ret) {
		DRM_ERROR("DRM: Error sysfs_device_add.\n");
		goto err_debugfs;
	}

	/* replace NULL with @minor so lookups will succeed from now on */
	spin_lock_irqsave(&drm_minor_lock, flags);
	idr_replace(&drm_minors_idr, new_minor, new_minor->index);
	spin_unlock_irqrestore(&drm_minor_lock, flags);

	DRM_DEBUG("new minor assigned %d\n", minor_id);
	return 0;

err_debugfs:
	drm_debugfs_cleanup(new_minor);
err_id:
	spin_lock_irqsave(&drm_minor_lock, flags);
	idr_remove(&drm_minors_idr, minor_id);
	spin_unlock_irqrestore(&drm_minor_lock, flags);
	new_minor->index = 0;
	return ret;
}

static void drm_minor_unregister(struct drm_device *dev, unsigned int type)
{
	struct drm_minor *minor;
	unsigned long flags;

	minor = *drm_minor_get_slot(dev, type);
	if (!minor || !minor->kdev)
		return;

	spin_lock_irqsave(&drm_minor_lock, flags);
	idr_remove(&drm_minors_idr, minor->index);
	spin_unlock_irqrestore(&drm_minor_lock, flags);
	minor->index = 0;

	drm_debugfs_cleanup(minor);
	drm_sysfs_device_remove(minor);
}

/**
 * drm_minor_acquire - Acquire a DRM minor
 * @minor_id: Minor ID of the DRM-minor
 *
 * Looks up the given minor-ID and returns the respective DRM-minor object. The
 * refence-count of the underlying device is increased so you must release this
 * object with drm_minor_release().
 *
 * As long as you hold this minor, it is guaranteed that the object and the
 * minor->dev pointer will stay valid! However, the device may get unplugged and
 * unregistered while you hold the minor.
 *
 * Returns:
 * Pointer to minor-object with increased device-refcount, or PTR_ERR on
 * failure.
 */
struct drm_minor *drm_minor_acquire(unsigned int minor_id)
{
	struct drm_minor *minor;
	unsigned long flags;

	spin_lock_irqsave(&drm_minor_lock, flags);
	minor = idr_find(&drm_minors_idr, minor_id);
	if (minor)
		drm_dev_ref(minor->dev);
	spin_unlock_irqrestore(&drm_minor_lock, flags);

	if (!minor) {
		return ERR_PTR(-ENODEV);
	} else if (drm_device_is_unplugged(minor->dev)) {
		drm_dev_unref(minor->dev);
		return ERR_PTR(-ENODEV);
	}

	return minor;
}

/**
 * drm_minor_release - Release DRM minor
 * @minor: Pointer to DRM minor object
 *
 * Release a minor that was previously acquired via drm_minor_acquire().
 */
void drm_minor_release(struct drm_minor *minor)
{
	drm_dev_unref(minor->dev);
}

/**
 * drm_put_dev - Unregister and release a DRM device
 * @dev: DRM device
 *
 * Called at module unload time or when a PCI device is unplugged.
 *
 * Use of this function is discouraged. It will eventually go away completely.
 * Please use drm_dev_unregister() and drm_dev_unref() explicitly instead.
 *
 * Cleans up all DRM device, calling drm_lastclose().
 */
void drm_put_dev(struct drm_device *dev)
{
	DRM_DEBUG("\n");

	if (!dev) {
		DRM_ERROR("cleanup called no dev\n");
		return;
	}

	drm_dev_unregister(dev);
	drm_dev_unref(dev);
}
EXPORT_SYMBOL(drm_put_dev);

void drm_unplug_dev(struct drm_device *dev)
{
	/* for a USB device */
	drm_minor_unregister(dev, DRM_MINOR_LEGACY);
	drm_minor_unregister(dev, DRM_MINOR_RENDER);
	drm_minor_unregister(dev, DRM_MINOR_CONTROL);

	mutex_lock(&drm_global_mutex);

	drm_device_set_unplugged(dev);

	if (dev->open_count == 0) {
		drm_put_dev(dev);
	}
	mutex_unlock(&drm_global_mutex);
}
EXPORT_SYMBOL(drm_unplug_dev);

/*
 * DRM internal mount
 * We want to be able to allocate our own "struct address_space" to control
 * memory-mappings in VRAM (or stolen RAM, ...). However, core MM does not allow
 * stand-alone address_space objects, so we need an underlying inode. As there
 * is no way to allocate an independent inode easily, we need a fake internal
 * VFS mount-point.
 *
 * The drm_fs_inode_new() function allocates a new inode, drm_fs_inode_free()
 * frees it again. You are allowed to use iget() and iput() to get references to
 * the inode. But each drm_fs_inode_new() call must be paired with exactly one
 * drm_fs_inode_free() call (which does not have to be the last iput()).
 * We use drm_fs_inode_*() to manage our internal VFS mount-point and share it
 * between multiple inode-users. You could, technically, call
 * iget() + drm_fs_inode_free() directly after alloc and sometime later do an
 * iput(), but this way you'd end up with a new vfsmount for each inode.
 */

static int drm_fs_cnt;
static struct vfsmount *drm_fs_mnt;

static const struct dentry_operations drm_fs_dops = {
	.d_dname	= simple_dname,
};

static const struct super_operations drm_fs_sops = {
	.statfs		= simple_statfs,
};

static struct dentry *drm_fs_mount(struct file_system_type *fs_type, int flags,
				   const char *dev_name, void *data)
{
	return mount_pseudo(fs_type,
			    "drm:",
			    &drm_fs_sops,
			    &drm_fs_dops,
			    0x010203ff);
}

static struct file_system_type drm_fs_type = {
	.name		= "drm",
	.owner		= THIS_MODULE,
	.mount		= drm_fs_mount,
	.kill_sb	= kill_anon_super,
};

#endif





int drm_fill_in_dev(struct drm_device *dev,
			   const struct pci_device_id *ent,
			   struct drm_driver *driver)
{
	int ret;
	dev->driver = driver;

	INIT_LIST_HEAD(&dev->filelist);
	INIT_LIST_HEAD(&dev->ctxlist);
	INIT_LIST_HEAD(&dev->vmalist);
	INIT_LIST_HEAD(&dev->maplist);
	INIT_LIST_HEAD(&dev->vblank_event_list);

	spin_lock_init(&dev->buf_lock);
	spin_lock_init(&dev->event_lock);
	mutex_init(&dev->struct_mutex);
	mutex_init(&dev->ctxlist_mutex);

//	if (drm_ht_create(&dev->map_hash, 12)) {
//		return -ENOMEM;
//	}



	if (driver->driver_features & DRIVER_GEM) {
		ret = drm_gem_init(dev);
		if (ret) {
			DRM_ERROR("Cannot initialize graphics execution manager (GEM)\n");
			goto err_ctxbitmap;
		}
	}

	return 0;

err_ctxbitmap:
//   drm_lastclose(dev);
	return ret;
}
EXPORT_SYMBOL(drm_fill_in_dev);
/**
 * Compute size order.  Returns the exponent of the smaller power of two which
 * is greater or equal to given number.
 *
 * \param size size.
 * \return order.
 *
 * \todo Can be made faster.
 */
int drm_order(unsigned long size)
{
    int order;
    unsigned long tmp;

    for (order = 0, tmp = size >> 1; tmp; tmp >>= 1, order++) ;

    if (size & (size - 1))
        ++order;

    return order;
}

int drm_sysfs_connector_add(struct drm_connector *connector)
{
    return 0;
}

void drm_sysfs_connector_remove(struct drm_connector *connector)
{ }

void drm_sysfs_hotplug_event(struct drm_device *dev)
{
    DRM_DEBUG("generating hotplug event\n");
}


