/*
 * i2c interface for userspace
 *
 * (c) 2010 Andrey Yurovsky <yurovsky@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301 USA
 */
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

int i2c_write(int fd, uint8_t addr, void *buf, unsigned int nb,
		unsigned int ds)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msg;

	msgset.nmsgs = 1;
	msgset.msgs = &msg;

	msg.addr = addr;
	msg.flags = 0;
	msg.buf = (unsigned char *)buf;
	msg.len = nb*ds;

	return ioctl(fd, I2C_RDWR, &msgset);
}

int i2c_read(int fd, uint8_t addr, uint8_t reg, void *buf, unsigned int nb,
		unsigned int ds)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs[2];

	if (nb < 1)
		return 0;

	msgset.nmsgs = 2;
	msgset.msgs = msgs;

	msgs[0].addr = addr;
	msgs[0].flags = 0;
	msgs[0].buf = (unsigned char *)&reg;
	msgs[0].len = 1;

	msgs[1].addr = addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = (unsigned char *)buf;
	msgs[1].len = nb*ds;

	return ioctl(fd, I2C_RDWR, &msgset);
}

int i2c_smbus_read(int fd, uint8_t addr, uint8_t *buf, unsigned int nb)
{
	unsigned int i;

	for (i = 0; i < nb; i++)
		if (read(fd, &buf[i], 1) != 1)
			return -1;
	return 0;
}

int i2c_open(char *adapter, uint8_t addr)
{
	int fd;
	unsigned long funcs;

	fd = open(adapter, O_RDWR);
	if (fd < 0) {
		perror("open");
		return -ENODEV;
	}

	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		perror("set slave address");
		return -ENODEV;
	}

	/* sanity check */
	if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
		perror("get functions");
		return -EINVAL;
	}

	/* make sure this is an i2c device */
	if (!(funcs & I2C_FUNC_I2C))
		return -EINVAL;

	return fd;
}
