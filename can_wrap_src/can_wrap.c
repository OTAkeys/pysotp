
#include "can_wrap.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/isotp.h>

#define MAX_NAME_LEN 20
#define MAX_SOCKETS 20
#define MAX_DATA_LEN 4095

/* globals */
static int sockets[MAX_SOCKETS];
static char can_iface[MAX_NAME_LEN];
static unsigned int sock_index;
static struct sockaddr_can addr[MAX_SOCKETS];

/* prototypes */
static int read_timeout(int fd, void *buf, long int msec);


/* initializes can interface */
int iso_tp_init(char *iface, unsigned int len) {

	if (!iface || !len || len >= MAX_NAME_LEN)
		return ERR;

	/* fail if can_iface is already set */
	if (strlen(can_iface))
		return ERR;

	sock_index = 0;
	strncpy(can_iface, iface, len + 1);
	return OK;
}

/* opens and bind socket to certain address pair */
/* returns channel id */
int iso_tp_map_channel(uint32_t tx_id, uint32_t rx_id) {
	int s;

	if (sock_index + 1 >= MAX_SOCKETS)
		return ERR;

	/* fail if can_iface is not set */
	if (!strlen(can_iface))
		return ERR;

	addr[sock_index].can_addr.tp.tx_id = (canid_t) tx_id;
	addr[sock_index].can_addr.tp.rx_id = (canid_t) rx_id;
	addr[sock_index].can_family = AF_CAN;
	addr[sock_index].can_ifindex = (int)if_nametoindex(can_iface);

	if ((s = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP)) < 0) {
		perror("socket");
		return ERR;
	}

	if (bind(s, (struct sockaddr *)&addr[sock_index], sizeof(struct sockaddr)) < 0) {
		perror("bind");
		close(s);
		return ERR;
	}

	sockets[sock_index] = s;

	return (int)sock_index++;
}

/* sends some data over iso-tp */
int iso_tp_send(unsigned int channel, char *data, unsigned int len) {
	int s, ret;

	if (!data || !len || len > MAX_DATA_LEN || channel >= sock_index)
		return ERR;

	/* fail if can_iface is not set */
	if (!strlen(can_iface))
		return ERR;

	s = sockets[channel];
	ret = write(s, data, len);

	if (ret < 0 || ret != (int)len)
		return ERR;

	return OK;
}

/* receive data from socket*/
int iso_tp_receive(unsigned int channel, char *data, unsigned int *len, long int timeout) {
	int ret, s;

	if (channel >= sock_index)
		return ERR;

	if (!data)
		return ERR;

	/* fail if can_iface is not set */
	if (!strlen(can_iface))
		return ERR;

	s = sockets[channel];
	ret = read_timeout(s, data, timeout);

	if (ret < 0)
		return ret;

	*len = (unsigned int)ret;
	return OK;
}

/* empties the receive queue */
int iso_tp_flush_rx(unsigned int channel) {
	int s;

	if (channel >= sock_index)
		return ERR;

	/* fail if can_iface is not set */
	if (!strlen(can_iface))
		return ERR;

	s = sockets[channel];

	if(close(s) < 0) {
		perror("close");
		return ERR;
	}

	if ((s = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP)) < 0) {
		perror("socket");
		return ERR;
	}

	if (bind(s, (struct sockaddr *)&addr[channel], sizeof(struct sockaddr_can)) < 0) {
		perror("bind");
		close(s);
		return ERR;
	}

	sockets[channel] = s;

	return OK;
}

/* closes all opened sockets */
void iso_tp_stop(void) {
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
		return read(fd, buf, MAX_DATA_LEN); /* there was data to read */
	}
}

