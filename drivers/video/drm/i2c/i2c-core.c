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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.		     */
/* ------------------------------------------------------------------------- */

/* With some changes from Kyösti Mälkki <kmalkki@cc.hut.fi>.
   All SMBus-related things are written by Frodo Looijaard <frodol@dds.nl>
   SMBus 2.0 support by Mark Studebaker <mdsxyz123@yahoo.com> and
   Jean Delvare <khali@linux-fr.org> */

#include <types.h>
#include <list.h>
#include <errno.h>
#include <linux/i2c.h>
#include <syscall.h>


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
        orig_jiffies = 0;
        for (ret = 0, try = 0; try <= adap->retries; try++) {

            ret = adap->algo->master_xfer(adap, msgs, num);
            if (ret != -EAGAIN)
                break;
//            if (time_after(jiffies, orig_jiffies + adap->timeout))
//                break;
                delay(1);
        }
//        mutex_unlock(&adap->bus_lock);
        return ret;
    } else {
//        dev_dbg(&adap->dev, "I2C level transfers not supported\n");
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



