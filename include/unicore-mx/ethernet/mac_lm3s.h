
/* Ethernet INTERRUPT Values 		*/
#define ETH_INT_RXC			(1 << 0)	/* RX complete 		*/
#define ETH_INT_TXE			(1 << 1)	/* TX error 		*/
#define ETH_INT_TXC			(1 << 2)	/* TX complete 		*/
#define ETH_INT_RXO			(1 << 3)	/* RX fifo overrun 	*/
#define ETH_INT_RXE			(1 << 4)	/* RX error 		*/
#define ETH_INT_MGM			(1 << 5)	/* MGMT transaction	*/
#define ETH_INT_PHY			(1 << 6)	/* PHY event		*/
#define ETH_INT_ALL			(ETH_INT_RXC | ETH_INT_TXE | ETH_INT_TXC | ETH_INT_RXO | ETH_INT_RXE | ETH_INT_MGM | ETH_INT_PHY)

/* Ethernet TX Control Values 		*/
#define ETH_TCTL_TXON			(1 << 0)
#define ETH_TCTL_PADDING		(1 << 1)
#define ETH_TCTL_CRCGEN			(1 << 2)
#define ETH_TCTL_DUPLEX			(1 << 4)

/* Ethernet RX Control Values		*/
#define ETH_RCTL_RXON			(1 << 0)
#define ETH_RCTL_MCAST			(1 << 1)
#define ETH_RCTL_PROMISC		(1 << 2)
#define ETH_RCTL_CRCFILTER		(1 << 3)
#define ETH_RCTL_CLRFIFO		(1 << 4)


#define SYSTEMCONTROL_ETH       0x20105000


enum eth_clk {
	ETH_CLK_50MHZ = 1
};
