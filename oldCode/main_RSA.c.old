#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_cache.h"

//Uses an AXI DMA IP Core
//Transfers data from data_read_addr to the M_AXIS_MM2S port
//The sequence is explained in detail in AXI DMA v7.1 - LogiCORE IP Product Guide
//(http://www.xilinx.com/support/documentation/ip_documentation/axi_dma/v7_1/pg021_axi_dma.pdf) on page 72)

//Divide by four to get the 32-bit offsets
#define MM2S_DMACR_OFFSET (0/4)
#define MM2S_SA_OFFSET (0x18/4)
#define MM2S_LENGTH_OFFSET (0x28/4)

unsigned int * dma_config 		  = (unsigned int *)0x40400000;
unsigned int * my_montgomery_port = (unsigned int *)0x43C00000;
unsigned int * my_montgomery_data = (unsigned int *)0x43C10000;

//1024 = 32 * 32
#define DMA_TRANSFER_NUMBER_OF_WORDS 32

//Simple test: mwr 0x40400000 1; mwr 0x40400028 1; mwr 0x40400018 0x100000;
void bram_dma_transfer(unsigned int* dma_config, unsigned int * data_addr, unsigned int data_len)
{
	Xil_DCacheFlush(); //Flush the cache to ensure that fresh data is stored inside DRAM

	//Perform the DMA transfer
	dma_config[MM2S_DMACR_OFFSET] = 1; //Stop
	dma_config[MM2S_SA_OFFSET] = (int)data_addr; //Specify read address
	dma_config[MM2S_LENGTH_OFFSET] = data_len*4; //Specify number of bytes
}

void test_dma_transfer()
{
	int h,i;
    unsigned int src[DMA_TRANSFER_NUMBER_OF_WORDS];
	Xil_DCacheFlush(); //Flush the contents of testmem to DRAM

	xil_printf("Testing DMA<->BRAM");
	for (h=0; h<10; h++)
	{
		xil_printf(".");
		for (i=0; i< DMA_TRANSFER_NUMBER_OF_WORDS; i++)
		{
			src[i]=rand();
		}

		bram_dma_transfer(dma_config,src,DMA_TRANSFER_NUMBER_OF_WORDS);
		//Wait for 100 cycles to be sure that the DMA transfer has completed
		i=0;
		while (i<100)
			i = i + 1;

		//Compare the source and contents of the BRAM
		for (i=0; i<DMA_TRANSFER_NUMBER_OF_WORDS; i++)
		{
			int val1,val2;

			val1=src[i];
			val2=my_montgomery_data[i];

			//A mismatch in the first couple of words is OK - some cycles are required by the DMA transfer
			if (val1!=val2)
			{
				xil_printf("DMA<->BRAM PROBLEM: i=%d, addr(bram[i])=%x and addr(src[i])=%x, bram[i]=%x, src[i]=%x\n\r",i,&my_montgomery_data[i],&src[i],val2,val1);
				return;
			}
		}
	}
	xil_printf("OK\n\r");
}

void port2_wait_for_done()
{
	while(my_montgomery_port[1]==0)  //Read a response from P2
	{}
}

void print_bram_contents()
{
	int i;
	xil_printf("BRAM contents:\n\r");
	for (i=0; i<DMA_TRANSFER_NUMBER_OF_WORDS; i+=4)
		xil_printf("%08x %08x %08x %08x\n\r",my_montgomery_data[i], my_montgomery_data[i+1], my_montgomery_data[i+2], my_montgomery_data[i+3]);
}

int main()
{
	int i;
	init_platform();

    xil_printf("Startup..\n\r");

    test_dma_transfer();

    uint32_t a[32] = {0x1ba, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 , 0x0};
    uint32_t b[32] = {0x91b, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    uint32_t N[32] = {0xe67a6f31, 0xa677bce5, 0x3b9e66f8, 0x32dff402, 0x2ab9a7c8, 0xbe08bc1e, 0x2ade1186, 0x8e88e8cf, 0x716a5c3b, 0x5453c47, 0x93ade446, 0xdc771f42, 0xe1709725, 0x7f89ebb5, 0x1d501ea9, 0x93c9933, 0xcf1ae468, 0x56ba1eb0, 0x358e7c24, 0x2daae084, 0x4da058ec, 0x27028a89, 0x4f2e5438, 0x29ab96b2, 0xf9072101, 0xe7990185, 0x199af8fe, 0x6887926d, 0xcfb0450e, 0x36a53dd9, 0xed3982c, 0xd1d3b4d6};




    // Read 'A'
    xil_printf("A: 	PORT1=%x\n\r",0x0);
    my_montgomery_port[0] = 0x0; //Send a command to P1

    bram_dma_transfer(dma_config,a,DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents();



    // Read 'B'
    xil_printf("B: 	PORT1=%x\n\r",0x0);
    my_montgomery_port[0] = 0x0; //Send a command to P1

    bram_dma_transfer(dma_config,b,DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents();



    // Read 'M'
    xil_printf("M: 	PORT1=%x\n\r",0x0);
    my_montgomery_port[0] = 0x0; //Send a command to P1

    bram_dma_transfer(dma_config,N,DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents();




    xil_printf("Computing! \n\r",0x0);
    // Perform the compute operation
    my_montgomery_port[0] = 0x1; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents(); //Print BRAM to serial port.



    // Write out the results
    xil_printf("Results! \n\r",0x0);
    my_montgomery_port[0] = 0x2; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents(); //Print BRAM to serial port.



    //Test the port-based comms
    /*
    xil_printf("PORT1=%x\n\r",0x0);
    my_montgomery_port[0] = 0x0; //Send a command to P1

    bram_dma_transfer(dma_config,srcA,DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1

    print_bram_contents(); //Print BRAM to serial port.

    //Perform the compute operation
    xil_printf("PORT1=%x\n\r",0x1);
    my_montgomery_port[0] = 0x1; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1

    //Write the result to BRAM
    xil_printf("PORT1=%x\n\r",0x2);
    my_montgomery_port[0] = 0x2; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1

    print_bram_contents(); //Print BRAM to serial port.
	*/

    cleanup_platform();
    return 0;
}
