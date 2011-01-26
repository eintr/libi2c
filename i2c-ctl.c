/*
 * i2c-ctl: demo program for i2c read/write
 *
 * (c) 2010 Andrey Yurovsky <yurovsky@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <i2c-io.h>

void print_usage(char *name)
{
	printf("usage:	%s <adapter> r<s> <address> <register> <count>\n",
			name);
	printf("\t%s <adapter> w<s> <address> <register> <b1> [b2] [b3] ...\n",
			name);
	printf("\n\twhere <s> is one of: b (byte), h (short), w (word)\n");
}

void print_buffer(void *buf, unsigned int nb, unsigned int ds)
{
	unsigned char *b = (unsigned char *)buf;
	int i, j;

	for (i = 0; i < nb; i++) {
		for (j = 0; j < ds; j++)
			printf("%02x ", b[j]);
		b += ds;
	}
	putchar('\n');
}

int loadb_tx(int argc, char **argv, uint8_t *dest)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (sscanf(argv[i], "%hhx", &dest[i]) != 1) {
			fprintf(stderr, "argument %d is invalid\n", i+4);
			return 1;
		}
	}
	
	return 0;
}

int loads_tx(int argc, char **argv, uint16_t *dest)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (sscanf(argv[i], "%hx", &dest[i]) != 1) {
			fprintf(stderr, "argument %d is invalid\n", i+4);
			return 1;
		}
	}
	
	return 0;
}

int loadw_tx(int argc, char **argv, uint32_t *dest)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (sscanf(argv[i], "%x", &dest[i]) != 1) {
			fprintf(stderr, "argument %d is invalid\n", i+4);
			return 1;
		}
	}
	
	return 0;
}

int load_tx(int argc, char **argv, unsigned int ds, void *dest)
{
	switch (ds) {
		case 1:
			return loadb_tx(argc, argv, dest);
		case 2:
			return loads_tx(argc, argv, dest);
		case 4:
			return loadw_tx(argc, argv, dest);
	}

	return 1;
}

int main(int argc, char **argv)
{
	int fd;
	int ret;
	unsigned int i;
	struct stat st;
	unsigned int addr, reg, nb, ds;
	void *buf;

	if (argc < 6 || (argv[2][0] != 'r' && argv[2][0] != 'w') ||
			strlen(argv[2]) < 2) {
		print_usage(argv[0]);
		return 0;
	}

	if (sscanf(argv[3], "%x", &addr) != 1) {
		printf("error: invalid address\n");
		print_usage(argv[0]);
		return 1;
	}

	if (sscanf(argv[4], "%x", &reg) != 1) {
		printf("error: invalid register\n");
		print_usage(argv[0]);
		return 2;
	}

	if (stat("/sys/class/i2c-dev", &st)) {
		printf("error: i2c-dev not loaded\n");
		return 3;
	}

	fd = i2c_open(argv[1], addr);
	if (fd < 0)
		return 4;

	switch (argv[2][1]) {
		default:
		case 'b':
			ds = sizeof(uint8_t);
			break;
		case 'h':
			ds = sizeof(uint16_t);
			break;
		case 'w':
			ds = sizeof(uint32_t);
			break;
	}

	if (argv[2][0] == 'r') {
		if (sscanf(argv[5], "%u", &nb) != 1 || nb < 1) {
			fprintf(stderr, "error: invalid count\n");
			print_usage(argv[0]);
			close(fd);
			return 5;
		}

		buf = malloc(nb*ds);
		if (!buf) {
			perror("malloc");
			close(fd);
			return 6;
		}

		ret = i2c_read(fd, addr, reg, buf, nb, ds);
		if (ret < 0) {
			printf("error: %d\n", ret);
			free(buf);
			close(fd);
			return 7;
		}

		print_buffer(buf, nb, ds);
		free(buf);
	} else if (argv[2][0] == 'w') {
		buf = malloc((argc - 4)*ds);
		if (!buf) {
			perror("malloc");
			close(fd);
			return 6;
		}
		
		if (load_tx(argc - 4, argv + 4, ds, buf) != 0) {
			free(buf);
			close(fd);
			return 8;
		}

		ret = i2c_write(fd, addr, buf, argc - 4, ds);
		if (ret < 0) {
			printf("error: %d\n", ret);
			free(buf);
			close(fd);
			return 7;
		}
		free(buf);
	}

	close(fd);
	return 0;
}
