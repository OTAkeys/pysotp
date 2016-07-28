
#pragma once
#include <stdint.h>

#define OK 0
#define ERR -1
#define ERR_TIMEOUT -2

/* initializes can interface */
int raw_can_init(char *iface, unsigned int len);

/* opens and bind socket to certain can id*/
/* returns channel id */
int raw_can_map_channel(uint32_t can_id);

/* sends some data over raw can */
int raw_can_send(unsigned int channel, char *data, unsigned int len);

/* receive data from socket*/
int raw_can_receive(unsigned int channel, char *data, unsigned int *len, long int timeout);

/* empties the receive queue */
int raw_can_flush_rx(unsigned int channel);

/* closes all opened sockets */
void raw_can_stop(void);


/* initializes can interface */
int iso_tp_init(char *iface, unsigned int len);

/* opens and bind socket to certain address pair */
/* returns channel id */
int iso_tp_map_channel(uint32_t tx_id, uint32_t rx_id);

/* sends some data over iso-tp */
int iso_tp_send(unsigned int channel, char *data, unsigned int len);

/* receive data from socket*/
int iso_tp_receive(unsigned int channel, char *data, unsigned int *len, long int timeout);

/* emties the receive queue */
int iso_tp_flush_rx(unsigned int channel);

/* closes all opened sockets */
void iso_tp_stop(void);

