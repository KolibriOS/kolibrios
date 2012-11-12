/* i2c-core.c - a device driver for the iic-bus interface		     */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 1995-99 Simon G. Vogl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.							     */
/* ------------------------------------------------------------------------- */

/* With some changes from Kyösti Mälkki <kmalkki@cc.hut.fi>.
   All SMBus-related things are written by Frodo Looijaard <frodol@dds.nl>
   SMBus 2.0 support by Mark Studebaker <mdsxyz123@yahoo.com> and
   Jean Delvare <khali@linux-fr.org>
   Mux support by Rodolfo Giometti <giometti@enneenne.com> and
   Michael Lawnick <michael.lawnick.ext@nsn.com> */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <list.h>
#include <errno.h>
#include <linux/i2c.h>
#include <syscall.h>


































#if 0

static ssize_t
show_modalias(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	return sprintf(buf, "%s%s\n", I2C_MODULE_PREFIX, client->name);
}

static DEVICE_ATTR(name, S_IRUGO, show_name, NULL);
static DEVICE_ATTR(modalias, S_IRUGO, show_modalias, NULL);

static struct attribute *i2c_dev_attrs[] = {
	&dev_attr_name.attr,
	/* modalias helps coldplug:  modprobe $(cat .../modalias) */
	&dev_attr_modalias.attr,
	NULL
};

static struct attribute_group i2c_dev_attr_group = {
	.attrs		= i2c_dev_attrs,
};

static const struct attribute_group *i2c_dev_attr_groups[] = {
	&i2c_dev_attr_group,
	NULL
};

static const struct dev_pm_ops i2c_device_pm_ops = {
	.suspend = i2c_device_pm_suspend,
	.resume = i2c_device_pm_resume,
	.freeze = i2c_device_pm_freeze,
	.thaw = i2c_device_pm_thaw,
	.poweroff = i2c_device_pm_poweroff,
	.restore = i2c_device_pm_restore,
	SET_RUNTIME_PM_OPS(
		pm_generic_runtime_suspend,
		pm_generic_runtime_resume,
		pm_generic_runtime_idle
	)
};

struct bus_type i2c_bus_type = {
	.name		= "i2c",
	.match		= i2c_device_match,
	.probe		= i2c_device_probe,
	.remove		= i2c_device_remove,
	.shutdown	= i2c_device_shutdown,
	.pm		= &i2c_device_pm_ops,
};
EXPORT_SYMBOL_GPL(i2c_bus_type);

static struct device_type i2c_client_type = {
	.groups		= i2c_dev_attr_groups,
	.uevent		= i2c_device_uevent,
	.release	= i2c_client_dev_release,
};


/**
 * i2c_verify_client - return parameter as i2c_client, or NULL
 * @dev: device, probably from some driver model iterator
 *
 * When traversing the driver model tree, perhaps using driver model
 * iterators like @device_for_each_child(), you can't assume very much
 * about the nodes you find.  Use this function to avoid oopses caused
 * by wrongly treating some non-I2C device as an i2c_client.
 */
struct i2c_client *i2c_verify_client(struct device *dev)
{
	return (dev->type == &i2c_client_type)
			? to_i2c_client(dev)
			: NULL;
}
EXPORT_SYMBOL(i2c_verify_client);


/* This is a permissive address validity check, I2C address map constraints
 * are purposely not enforced, except for the general call address. */
static int i2c_check_client_addr_validity(const struct i2c_client *client)
{
	if (client->flags & I2C_CLIENT_TEN) {
		/* 10-bit address, all values are valid */
		if (client->addr > 0x3ff)
			return -EINVAL;
	} else {
		/* 7-bit address, reject the general call address */
		if (client->addr == 0x00 || client->addr > 0x7f)
			return -EINVAL;
	}
	return 0;
}

/* And this is a strict address validity check, used when probing. If a
 * device uses a reserved address, then it shouldn't be probed. 7-bit
 * addressing is assumed, 10-bit address devices are rare and should be
 * explicitly enumerated. */
static int i2c_check_addr_validity(unsigned short addr)
{
	/*
	 * Reserved addresses per I2C specification:
	 *  0x00       General call address / START byte
	 *  0x01       CBUS address
	 *  0x02       Reserved for different bus format
	 *  0x03       Reserved for future purposes
	 *  0x04-0x07  Hs-mode master code
	 *  0x78-0x7b  10-bit slave addressing
	 *  0x7c-0x7f  Reserved for future purposes
	 */
	if (addr < 0x08 || addr > 0x77)
		return -EINVAL;
	return 0;
}

static int __i2c_check_addr_busy(struct device *dev, void *addrp)
{
	struct i2c_client	*client = i2c_verify_client(dev);
	int			addr = *(int *)addrp;

	if (client && client->addr == addr)
		return -EBUSY;
	return 0;
}

/* walk up mux tree */
static int i2c_check_mux_parents(struct i2c_adapter *adapter, int addr)
{
	struct i2c_adapter *parent = i2c_parent_is_i2c_adapter(adapter);
	int result;

	result = device_for_each_child(&adapter->dev, &addr,
					__i2c_check_addr_busy);

	if (!result && parent)
		result = i2c_check_mux_parents(parent, addr);

	return result;
}

/* recurse down mux tree */
static int i2c_check_mux_children(struct device *dev, void *addrp)
{
	int result;

	if (dev->type == &i2c_adapter_type)
		result = device_for_each_child(dev, addrp,
						i2c_check_mux_children);
	else
		result = __i2c_check_addr_busy(dev, addrp);

	return result;
}

static int i2c_check_addr_busy(struct i2c_adapter *adapter, int addr)
{
	struct i2c_adapter *parent = i2c_parent_is_i2c_adapter(adapter);
	int result = 0;

	if (parent)
		result = i2c_check_mux_parents(parent, addr);

	if (!result)
		result = device_for_each_child(&adapter->dev, &addr,
						i2c_check_mux_children);

	return result;
}

/**
 * i2c_lock_adapter - Get exclusive access to an I2C bus segment
 * @adapter: Target I2C bus segment
 */
void i2c_lock_adapter(struct i2c_adapter *adapter)
{
	struct i2c_adapter *parent = i2c_parent_is_i2c_adapter(adapter);

	if (parent)
		i2c_lock_adapter(parent);
	else
		rt_mutex_lock(&adapter->bus_lock);
}
EXPORT_SYMBOL_GPL(i2c_lock_adapter);

/**
 * i2c_trylock_adapter - Try to get exclusive access to an I2C bus segment
 * @adapter: Target I2C bus segment
 */
static int i2c_trylock_adapter(struct i2c_adapter *adapter)
{
	struct i2c_adapter *parent = i2c_parent_is_i2c_adapter(adapter);

	if (parent)
		return i2c_trylock_adapter(parent);
	else
		return rt_mutex_trylock(&adapter->bus_lock);
}

/**
 * i2c_unlock_adapter - Release exclusive access to an I2C bus segment
 * @adapter: Target I2C bus segment
 */
void i2c_unlock_adapter(struct i2c_adapter *adapter)
{
	struct i2c_adapter *parent = i2c_parent_is_i2c_adapter(adapter);

	if (parent)
		i2c_unlock_adapter(parent);
	else
		rt_mutex_unlock(&adapter->bus_lock);
}
EXPORT_SYMBOL_GPL(i2c_unlock_adapter);

#endif


/**
 * i2c_transfer - execute a single or combined I2C message
 * @adap: Handle to I2C bus
 * @msgs: One or more messages to execute before STOP is issued to
 *  terminate the operation; each message begins with a START.
 * @num: Number of messages to be executed.
 *
 * Returns negative errno, else the number of messages executed.
 *
 * Note that there is no requirement that each message be sent to
 * the same slave address, although that is the most common model.
 */
int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    unsigned long orig_jiffies;
    int ret, try;

    /* REVISIT the fault reporting model here is weak:
     *
     *  - When we get an error after receiving N bytes from a slave,
     *    there is no way to report "N".
     *
     *  - When we get a NAK after transmitting N bytes to a slave,
     *    there is no way to report "N" ... or to let the master
     *    continue executing the rest of this combined message, if
     *    that's the appropriate response.
     *
     *  - When for example "num" is two and we successfully complete
     *    the first message but get an error part way through the
     *    second, it's unclear whether that should be reported as
     *    one (discarding status on the second message) or errno
     *    (discarding status on the first one).
     */

    if (adap->algo->master_xfer) {

        /* Retry automatically on arbitration loss */
        orig_jiffies = GetTimerTicks();

        for (ret = 0, try = 0; try <= adap->retries; try++) {

            ret = adap->algo->master_xfer(adap, msgs, num);
            if (ret != -EAGAIN)
                break;

            if (time_after(GetTimerTicks(), orig_jiffies + adap->timeout))
                break;

            delay(1);
        }
//        mutex_unlock(&adap->bus_lock);
        return ret;
    } else {
        dbgprintf("I2C level transfers not supported\n");
        return -EOPNOTSUPP;
    }
}
EXPORT_SYMBOL(i2c_transfer);


/**
 * i2c_new_device - instantiate an i2c device
 * @adap: the adapter managing the device
 * @info: describes one I2C device; bus_num is ignored
 * Context: can sleep
 *
 * Create an i2c device. Binding is handled through driver model
 * probe()/remove() methods.  A driver may be bound to this device when we
 * return from this function, or any later moment (e.g. maybe hotplugging will
 * load the driver module).  This call is not appropriate for use by mainboard
 * initialization logic, which usually runs during an arch_initcall() long
 * before any i2c_adapter could exist.
 *
 * This returns the new i2c client, which may be saved for later use with
 * i2c_unregister_device(); or NULL to indicate an error.
 */
struct i2c_client *
i2c_new_device(struct i2c_adapter *adap, struct i2c_board_info const *info)
{
    struct i2c_client   *client;
    int         status;

    client = kzalloc(sizeof *client, GFP_KERNEL);
    if (!client)
        return NULL;

    client->adapter = adap;

    client->flags = info->flags;
    client->addr = info->addr;
    client->irq = info->irq;

    strlcpy(client->name, info->type, sizeof(client->name));

    return client;
}



