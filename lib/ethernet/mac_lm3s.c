#include <string.h>
#include <stdint.h>
#include <unicore-mx/ethernet/mac.h>
#include <unicore-mx/ethernet/phy.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/cm3/systick.h>
#include <unicore-mx/lm3s/memorymap.h>
#include <unicore-mx/lm3s/systemcontrol.h>

void eth_set_mac(uint8_t *mac)
{
    ETH_MAC_ADDR0 = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | (mac[3]);
    ETH_MAC_ADDR1 = (mac[4]<< 24) | (mac[6] << 16);
}

// void eth_desc_init(uint8_t *buf, uint32_t nTx, uint32_t nRx, uint32_t cTx, uint32_t cRx, bool isext);

bool eth_tx(uint8_t *ppkt, uint32_t n)
{
    uint32_t word;
    unsigned int i = 0;
    if (n > (2048 - 2))
        return false;

    if (ETH_MAC_TR != 0)
        return false;

    word = ((n - 14) & 0xFFFF);
    word |= (ppkt[i++] << 16);
    word |= (ppkt[i++] << 24);
    ETH_MAC_DATA = word;

    while (i < (n - 4)) {
        word = ppkt[i++];
        word |= (ppkt[i++] << 8);
        word |= (ppkt[i++] << 16);
        word |= (ppkt[i++] << 24);
        ETH_MAC_DATA = word;
    }
    word = ppkt[i++];
    if (i < n)
        word |= ppkt[i++] << 8;
    if (i < n)
        word |= ppkt[i++] << 16;
    if (i < n)
        word |= ppkt[i++] << 24;

    ETH_MAC_DATA = word;
    ETH_MAC_TR = 1;
    return true;
}

bool eth_rx(uint8_t *ppkt, uint32_t *len, uint32_t maxlen) 
{
    uint32_t word;
    unsigned int i = 0;
    if ((ETH_MAC_NP & 0x3F) == 0) {
        return false;
    }

    word = ETH_MAC_DATA;
    *len = (word & 0xFFFF);
    if (maxlen < *len) {
        *len = maxlen;
    }

    ppkt[i++] = (uint8_t)((word & 0x00FF0000) >> 16);
    ppkt[i++] = (uint8_t)((word & 0xFF000000) >> 24);

    while(i < (*len - 6)) {
        word = ETH_MAC_DATA;
        ppkt[i++] = (word & 0x000000FF);
        ppkt[i++] = (uint8_t)((word & 0x0000FF00) >> 8);
        ppkt[i++] = (uint8_t)((word & 0x00FF0000) >> 16);
        ppkt[i++] = (uint8_t)((word & 0xFF000000) >> 24);
    }
    word = ETH_MAC_DATA;
    ppkt[i++] = (word & 0x000000FF);
    if (i < *len)
        ppkt[i++] = (uint8_t)((word & 0x0000FF00) >> 8);
    if (i < *len)
        ppkt[i++] = (uint8_t)((word & 0x00FF0000) >> 16);
    if (i < *len)
        ppkt[i++] = (uint8_t)((word & 0xFF000000) >> 24);
    return true;
}



void eth_enable_checksum_offload(void)
{
    ETH_MAC_TCTL = ETH_TCTL_CRCGEN;
    ETH_MAC_RCTL = ETH_RCTL_CRCFILTER;
}



void eth_init(uint8_t phy, enum eth_clk clock)
{
    (void)phy;
    /* Enable device */
    SYSTEMCONTROL_SCGC2 |= (SYSCTL_CGR2_ETH_PHY | SYSCTL_CGR2_ETH_MAC);
    /* Reset device */
    SYSTEMCONTROL_SRCR2 |= (SYSCTL_CGR2_ETH_PHY | SYSCTL_CGR2_ETH_MAC);

    /* Disable interrupts */
    ETH_MAC_IM = 0;
    /* Clear pending IRQs */
    ETH_MAC_RISACK = ETH_INT_ALL;

    /* Set ethernet clock speed */
    ETH_MAC_MDV = clock;

    /* Set transmit control register */
    ETH_MAC_TCTL = ETH_TCTL_DUPLEX | ETH_TCTL_PADDING | ETH_TCTL_CRCGEN;

    /* Set receive control register */
    ETH_MAC_RCTL = ETH_RCTL_MCAST;

    /* Disable timestamps */
    ETH_MAC_TS = 0;

}

void eth_start(void) {
    /* Enable Ethernet */
    ETH_MAC_RCTL |= ETH_RCTL_CLRFIFO;
    ETH_MAC_RCTL |= ETH_RCTL_RXON;
    

    ETH_MAC_TCTL |= ETH_TCTL_TXON;


    ETH_MAC_RCTL |= ETH_RCTL_CLRFIFO;
}

void eth_irq_enable(uint32_t reason) 
{
    (void)reason; 
    /* TODO */
}

void eth_irq_disable(uint32_t reason)
{
    (void)reason; 
    /* TODO */
}

bool eth_irq_is_pending(uint32_t reason) {
    (void)reason; 
    /* TODO */
    return false;
}


bool eth_irq_ack_pending(uint32_t reason)
{
    (void)reason; 
    /* TODO */
    return false;
}

void eth_isr(void)
{



}
