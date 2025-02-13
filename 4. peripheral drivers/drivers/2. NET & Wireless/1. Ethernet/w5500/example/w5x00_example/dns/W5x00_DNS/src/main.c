#include <stdio.h>
#include <stdlib.h>
#include "HAL_Config.h"
#include "HALInit.h"
#include "wizchip_conf.h"
#include "inttypes.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "W5x00RelFunctions.h"
#include "serialCommand.h"
#include "Internet\DNS\dns.h"

wiz_NetInfo gWIZNETINFO = { .mac = {0x00,0x08,0xdc,0x78,0x91,0x71},
							.ip = {192,168,0,15},
							.sn = {255, 255, 255, 0},
							.gw = {192, 168, 0, 1},
							.dns = {168, 126, 63, 1},
							.dhcp = NETINFO_STATIC};

#define ETH_MAX_BUF_SIZE	2048

unsigned char ethBuf0[ETH_MAX_BUF_SIZE];
unsigned char ethBuf1[ETH_MAX_BUF_SIZE];
unsigned char ethBuf2[ETH_MAX_BUF_SIZE];
unsigned char ethBuf3[ETH_MAX_BUF_SIZE];


u_char URL[] = "www.google.com";
uint8_t dns_server_ip[4] = {168,126,63,1};

uint8_t bLoopback = 1;
uint8_t bRandomPacket = 0;
uint8_t bAnyPacket = 0;
uint16_t pack_size = 0;
volatile uint32_t msTicks; 		/* counts 1ms timeTicks */


void print_network_information(void);
uint32_t Buttons_GetStatus(void);
static uint8_t Check_Buttons_Pressed(void);
void SysTick_Handler(void);
int main(void)
{
	uint8_t dnsclient_ip[16] = {0,};
	//datetime time;
	volatile int i;
	volatile int j,k;
	RCCInitialize();
	gpioInitialize();
	usartInitialize();
	timerInitialize();
	//SysTick_Config((SystemCoreClock/1000));
	printf("System start.\r\n");




#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
	// SPI method callback registration
	reg_wizchip_spi_cbfunc(spiReadByte, spiWriteByte);
	// CS function register
	reg_wizchip_cs_cbfunc(csEnable,csDisable);

#else
	// Indirect bus method callback registration
	reg_wizchip_bus_cbfunc(busReadByte, busWriteByte);
#endif

#if _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_
	FSMCInitialize();
#else
	spiInitailize();
#endif
	resetAssert();
	delay(10);
	resetDeassert();
	delay(10);
	W5x00Initialze();
	

#if _WIZCHIP_ != W5200
	printf("\r\nCHIP Version: %02x\r\n", getVER());
#endif
	wizchip_setnetinfo(&gWIZNETINFO);

	print_network_information();
	DNS_init(0, ethBuf0);	// timezone: Korea, Republic of
	while(1)
    {
		/* Button: USER0 */
		if(GPIO_ReadInputDataBit(USER0_PORT, USER0_PIN)== Bit_RESET)
		{

			while(GPIO_ReadInputDataBit(USER0_PORT, USER0_PIN)== Bit_RESET);
				printf("\r\n########## USer0 was released. \r\n");
					while (DNS_run(dns_server_ip,URL,dnsclient_ip)  != 1);
				printf("> Translated %s to %d.%d.%d.%d\r\n",URL,dnsclient_ip[0],dnsclient_ip[1],dnsclient_ip[2],dnsclient_ip[3]);

	   }
    }
}

void delay(unsigned int count)
{
	int temp;
	temp = count + TIM2_gettimer();
	while(temp > TIM2_gettimer()){}
}

void print_network_information(void)
{
	memset(&gWIZNETINFO,0,sizeof(gWIZNETINFO));

	wizchip_getnetinfo(&gWIZNETINFO);
	printf("MAC Address : %02x:%02x:%02x:%02x:%02x:%02x\n\r",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
	printf("IP  Address : %d.%d.%d.%d\n\r",gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
	printf("Subnet Mask : %d.%d.%d.%d\n\r",gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
	printf("Gateway     : %d.%d.%d.%d\n\r",gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
	printf("DNS Server  : %d.%d.%d.%d\n\r",gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
}

