/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <kernel.h>
#include <uzlib.h>
#include <string.h>
#include "compressed_data.h"



#define K32 32768
unsigned char temp_buf[256];
unsigned char out_buf[68012];
unsigned int offset = 0;

#define BUFFER_SIZE

int read_source_cb(struct uzlib_uncomp *data)
{
	unsigned int len = 256;
	int res;
	data->dest = temp_buf;
	data->dest_limit = temp_buf + len;
	res = uzlib_uncompress_chksum(data);
	printk("Decompressed chunk\n");
	if (res == TINF_OK) {
		printk("Decompressed done\n");
		/* Write temp buffer */
		memcpy(&out_buf[offset], temp_buf, len);
		offset += len;
		return 0; //END OF FILE
	}
	if ( res < 0){
		printk("Decompressed err \n");
		return -EFAULT;
	}
	return len;
}

void main(void)
{
	unsigned int len = app_update_bin_gz_len;
	unsigned int dlen;
	unsigned int outlen;
	unsigned char * source = app_update_bin_gz;
	unsigned char * dest;
	printk("Len bytes: %d\n", app_update_bin_gz_len);

	uzlib_init();

	dlen = len*4;
	outlen = dlen;
	dlen++;
	dest = (unsigned char *)k_malloc(dlen);
	struct uzlib_uncomp data;
	uzlib_uncompress_init(&data, NULL, 0);


	data.source = source;
	data.source_limit = source + len - 4;
	//data.source_read_cb = read_source_cb;

	int res = uzlib_gzip_parse_header(&data);
	if (res != TINF_OK) {
		printk("Error parsing header: %d\n", res);
		__ASSERT(true, "error");
	}

	for (int i = 0; i < 68012; i++) {
		printk("%x", out_buf[i]);
	}
	printk("\nPrinted\n");

	/* destination is larger than compressed */
	/*unsigned int dlen = len*4;*/
	/* Add extra byte incase header is out of bounds */
	data.dest_start = data.dest = dest;
	while(1){
		unsigned int chunk_len = dlen < 256 ? dlen : 256;
		data.dest_limit = data.dest + chunk_len;
		res = uzlib_uncompress_chksum(&data);
		dlen -= chunk_len;
		if(res != TINF_OK){
			break;
		}
	}
	if (res != TINF_DONE){
		printk("decompress CRC failure: %d\n", res);
	}
	printk("Decompressed %d bytes\n", data.dest - dest);
	for (int i = 0; i < 256 ; i++) {
		printk("%x", out_buf[i]);
	}
	for (int i = 0; i < 256 ; i++) {
		printk("%x", dest[i]);
	}
	printk("\nPrinted\n");

	printk("Hello World! %s\n", CONFIG_BOARD);
}
