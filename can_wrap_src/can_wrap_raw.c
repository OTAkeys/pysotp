
#include "can_wrap.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define MAX_NAME_LEN 20
#define MAX_SOCKETS 20
#define MAX_DATA_LEN 8

/* globals */
static int sockets[MAX_SOCKETS];
static uint32_t ids[MAX_SOCKETS];
static char can_iface[MAX_NAME_LEN];
static unsigned int sock_index;

/* prototypes */
static int read_timeout(int fd, void *buf, long int msec);


/* initializes can interface */
int raw_can_init(char *iface, unsigned int len) {

	if (!iface || !len || len >= MAX_NAME_LEN)
		return ERR;

	/* fail if can_iface is already set */
	if (strlen(can_iface))
		return ERR;

	sock_index = 0;
	strncpy(can_iface, iface, len + 1);
	return OK;
}

/* opens and bind socket to certain can id*/
/* returns channel id */
int raw_can_map_channel(uint32_t can_id) {
	int s;
	struct sockaddr_can addr;
	struct can_filter filter;

	if (sock_index + 1 >= MAX_SOCKETS)
		return ERR;

	/* fail if can_iface is not set */
	if (!strlen(can_iface))
		return ERR;

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("socket");
		return ERR;
	}

	filter.can_id   = can_id;
	filter.can_mask = CAN_SFF_MASK;

	if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter))){
		perror("filter");
		return ERR;
	}

	addr.can_family = AF_CAN;
	addr.can_ifindex = (int)if_nametoindex(can_iface);

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(s);
		return ERR;
	}

	sockets[sock_index] = s;
	ids[sock_index] = can_id;

	return (int)sock_index++;
}

/* sends some data over raw can */
int raw_can_send(unsigned int channel, char *data, unsigned int len) {
	int s, ret;
	uint32_t can_id;
	struct can_frame cf = {0};

	if (!data || !len || len > MAX_DATA_LEN || channel >= sock_index)
		return ERR;

	/* fail if can_iface is not set */
	if (!strlen(can_iface))
		return ERR;

	s = sockets[channel];
	can_id = ids[channel];

	cf.can_id = can_id;
	cf.can_dlc = (uint8_t)len;
	memcpy(cf.data, data, len);

	ret = write(s, &cf, CAN_MTU);

	if (ret < 0 || ret != CAN_MTU)
		return ERR;

	return OK;
}

/* receive data from socket*/
int raw_can_receive(unsigned int channel, char *data, unsigned int *len, long int timeout) {
	int ret, s;
	struct can_frame cf = {0};

	if (!data || channel >= sock_index)
		return ERR;

	/* fail if can_iface is not set */
	if (!strlen(can_iface))
		return ERR;

	s = sockets[channel];
	ret = read_timeout(s, &cf, timeout);

	if (ret < 0)
		return ret;

	*len = (unsigned int)cf.can_dlc;
	memcpy(data, cf.data, *len);

	return OK;
}

/* empties the receive queue */
int raw_can_flush_rx(unsigned int channel) {
	int s;
	uint32_t can_id;
	struct sockaddr_can addr;
	struct can_filter filter;

	if (channel >= sock_index)
		return ERR;

	/* fail if can_iface is not set */
	if (!strlen(can_iface))
		return ERR;

	s = sockets[channel];
	can_id = ids[channel];

	if(close(s) < 0) {
		perror("close");
		return ERR;
	}

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("socket");
		return ERR;
	}

	filter.can_id   = can_id;
	filter.can_mask = CAN_SFF_MASK;

	if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter))){
		perror("filter");
		return ERR;
	}

	addr.can_family = AF_CAN;
	addr.can_ifindex = (int)if_nametoindex(can_iface);

	if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_can)) < 0) {
		perror("bind");
		close(s);
		return ERR;
	}

	sockets[channel] = s;

	return OK;
}

/* closes all opened sockets */
void raw_can_stop(void) {
	for (; sock_index != 0; sock_index--)
		close(sockets[sock_index]);
	memset(can_iface, 0, MAX_NAME_LEN);
}

/* read with timeout */
static int read_timeout(int fd, void *buf, long int msec) {
	fd_set set;
	struct timeval timeout;
	int ret;

	FD_ZERO(&set); /* clear the set */
	FD_SET(fd, &set); /* add our file descriptor to the set */

	timeout.tv_sec = msec/1000;
	timeout.tv_usec = msec%1000*1000;

	ret = select(fd + 1, &set, NULL, NULL, &timeout);

	if(ret == -1) {
		perror("select"); /* an error occured */
		return ERR;
	} else if(ret == 0) { /* a timeout occured */
		return ERR_TIMEOUT;
	} else {
		return read(fd, buf, CAN_MTU); /* there was data to read */
	}
}

