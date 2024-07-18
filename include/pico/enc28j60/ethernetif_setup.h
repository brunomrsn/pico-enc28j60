#ifndef ENC28J60_ETHERNETIF_SETUP_H
#define ENC28J60_ETHERNETIF_SETUP_H

#define LWIP_VERSION_LESSER_THAN(major, minor) ((LWIP_VERSION_MAJOR < 2) || (LWIP_VERSION_MAJOR == 2 && LWIP_VERSION_MINOR < 1))
#define LWIP_VERSION_GREATER_THAN_OR_EQUAL_TO(major, minor) (!LWIP_VERSION_LESSER_THAN(major, minor))

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "lwip/netif.h"
#include "lwip/timeouts.h"

#include "pico/util/queue.h"
#include "pico/critical_section.h"
#include "pico/enc28j60/enc28j60.h"
#include "pico/enc28j60/ethernetif.h"

#include <string.h>

#define RX_QUEUE_SIZE 10
#define SPI_BAUD_RATE 2000000

void ethernetif_input();
void eth_irq(uint gpio, uint32_t events);
void ethernetif_setup(struct spi_inst* spi, uint int_pin, uint sck_pin, uint si_pin, uint so_pin, uint cs_pin, uint8_t* mac_address, struct ip4_addr* ipaddr, struct ip4_addr* netmask, struct ip4_addr* gw);
#endif
