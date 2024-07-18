#include "pico/enc28j60/ethernetif_setup.h"

static queue_t rx_queue;
static critical_section_t spi_cs;
static struct netif netif;
static struct enc28j60 enc28j60;

void ethernetif_setup(struct spi_inst* spi, uint int_pin, uint sck_pin, uint si_pin, uint so_pin, uint cs_pin, uint8_t* mac_address, struct ip4_addr* ipaddr, struct ip4_addr* netmask, struct ip4_addr* gw) {
    gpio_init(cs_pin);
    gpio_set_dir(cs_pin, true);

    gpio_set_function(sck_pin, GPIO_FUNC_SPI);
    gpio_set_function(si_pin, GPIO_FUNC_SPI);
    gpio_set_function(so_pin, GPIO_FUNC_SPI);
    spi_init(spi, SPI_BAUD_RATE);
    
    queue_init(&rx_queue, sizeof(struct pbuf *), RX_QUEUE_SIZE);
    
    enc28j60.spi = spi;
    enc28j60.cs_pin = cs_pin;
    memcpy(enc28j60.mac_address, mac_address, 6);
    enc28j60.next_packet = 0;
    enc28j60.critical_section = &spi_cs;

    netif_add(&netif, ipaddr, netmask, gw, &enc28j60, ethernetif_init, netif_input);
    
    netif_set_up(&netif);

    netif_set_link_up(&netif);

    gpio_set_irq_enabled_with_callback(int_pin, GPIO_IRQ_EDGE_FALL, true, eth_irq);
    enc28j60_interrupts(&enc28j60, ENC28J60_PKTIE | ENC28J60_TXERIE | ENC28J60_RXERIE);
}

void ethernetif_input()
{
	struct pbuf *p = NULL;
    queue_try_remove(&rx_queue, &p);
	if (p != NULL) {
		if (netif.input(p, &netif) != ERR_OK) {
			LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
			pbuf_free(p);
			p = NULL;
		}
	}
    sys_check_timeouts();
    best_effort_wfe_or_timeout(make_timeout_time_ms(sys_timeouts_sleeptime()));
}

void eth_irq(uint gpio, uint32_t events) {
    enc28j60_isr_begin(&enc28j60);
    uint8_t flags = enc28j60_interrupt_flags(&enc28j60);

    if (flags & ENC28J60_PKTIF) {
        struct pbuf *packet = low_level_input(&netif);
        if (packet != NULL) {
            if (!queue_try_add(&rx_queue, &packet)) {
                pbuf_free(packet);
            }
        }
    }

    if (flags & ENC28J60_TXERIF) {
        LWIP_DEBUGF(NETIF_DEBUG, ("eth_irq: transmit error\n"));
    }

    if (flags & ENC28J60_RXERIF) {
        LWIP_DEBUGF(NETIF_DEBUG, ("eth_irq: receive error\n"));
    }

    enc28j60_interrupt_clear(&enc28j60, flags);
    enc28j60_isr_end(&enc28j60);
}
