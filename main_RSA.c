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


void mont(uint32_t result, uint32_t A, uint32_t B, uint32_t m){
    // Send message
    xil_printf("Sending A\n\r");
    my_montgomery_port[0] = 0x0; //Send a command to P1
    bram_dma_transfer(dma_config, A, DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1


    // Send rsqModM
    xil_printf("Sending B\n\r");
    my_montgomery_port[0] = 0x0; //Send a command to P1
    bram_dma_transfer(dma_config, B, DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1


    // Send modullus
    xil_printf("Sending m\n\r");
    my_montgomery_port[0] = 0x0; //Send a command to P1
    bram_dma_transfer(dma_config, m, DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1


    // Compute Mont(message, rsqModM)
    my_montgomery_port[0] = 0x1; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents(); //Print BRAM to serial port.


    // Give the results to 'result'

    // Print x_delta values
    xil_printf("Results! \n\r");
    my_montgomery_port[0] = 0x2; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents(); //Print BRAM to serial port.
}




void encryption(uint32_t rsqModM, uint32_t rModM, uint32_t e, uint32_t modullus, uint32_t input_message, uint32_t output_ciphertext){
    /////////// Initialise variable A
    A = rModM;

    /////////// Initialise variable x_delta = Mont(message, rsqModM) ///////////

    // Send message
    xil_printf("Initialise x_delta: sending message\n\r");
    my_montgomery_port[0] = 0x0; //Send a command to P1
    bram_dma_transfer(dma_config,input_message,DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1


    // Send rsqModM
    xil_printf("Initialise x_delta: sending rsqModM\n\r");
    my_montgomery_port[0] = 0x0; //Send a command to P1
    bram_dma_transfer(dma_config,rsqModM,DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1


    // Send modullus
    xil_printf("Initialise x_delta: sending modullus\n\r");
    my_montgomery_port[0] = 0x0; //Send a command to P1
    bram_dma_transfer(dma_config,modullus,DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1


    // Compute Mont(message, rsqModM)
    my_montgomery_port[0] = 0x1; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents(); //Print BRAM to serial port.


    // Print x_delta values
    xil_printf("Results! \n\r");
    my_montgomery_port[0] = 0x2; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents(); //Print BRAM to serial port.






    /////////// Calculate A depending on e(i) ///////////
    t = 3; // t = number of bits of e
    for(i = t; i >= 0; i--){
        if()
    }






    /////////// Finalise A ///////////
}

void decryption(uint32_t rsqModM, uint32_t rModM, uint32_t d, uint32_t modullus, uint32_t input_ciphertext, uint32_t output_message){
    /////////// Initialise variables x_delta and A ///////////
    /////////// Calculate A depending on e(i) ///////////
    /////////// Finalise A ///////////
}


int main()
{
	int i;
	init_platform();

    xil_printf("Startup..\n\r");

    test_dma_transfer();

    /////////// Initialise variables - turn these into array of 32bit numbers!!
    // Seed = 2016. 

    // rsqModM 0x7C4D620EEE8D2AA7BBAA8784F92EE2AFF35EE001A23B4EC9367BFBFDCCE2831BB09504\
    // BA0CC74F9C9C23ED41A451D99F7B8CE10AD0AF741F323CBD3178A960878A263EFC7360E045B5FA0\
    // F88377E4C8923CD92465E525B979FF92E2260AE04FB918A54E7927115A1ED1B354CC4D245846C43\
    // A8A08E6F2AC4CAEFE3AA2CF8DC4

    // rModM 0x3D63B8F702B6DBF5B3657143E6BA3A916C9DD3F7A8C4CDD8C90F45FF6E1529EB9DB25D5\
    // A70719231347B7FCF6BC4555C4502B14C3CD9ABD23571700DC876C4590240034C2E5AF2A2200A01\
    // 52D0FB23A3553872CB83B1D6EA430508B0ADDBDBD33E3837ACE0D5C825CCD7F24E416E25FFFC628\
    // 7822CF6A6618471F1F844DC949F

    // e 0x101

    // d 0xAEEC18011B718DF753CBAB209A33C165619BF9C1C4F178C899F93E97E\
    // BBF28046053BA095B2E56BEDAC9C5E5EFD204F4C8F1A063D1464FA5AC1A895A35E6EDE43606A04F\
    // DEC017053AE7B0F0F9F03713F4F1AF73756249962DF9497127A789D81FE04C3921693086983323B\
    // A6F2DCDCD47A4F73E1902D801593BDD0F6E4A09D1

    // modullus 0xC29C4708FD49240A4C9A8EBC1945C56E93622C08573B322736F0BA0091EAD614624D\
    // A2A58F8E6DCECB848030943BAAA3BAFD4EB3C326542DCA8E8FF237893BA6FDBFFCB3D1A50D5DDFF\
    // 5FEAD2F04DC5CAAC78D347C4E2915BCFAF74F5224242CC1C7C8531F2A37DA33280DB1BE91DA0003\
    // 9D787DD309599E7B8E0E07BB236B61

    // message 0xB0C20CFD982B1F572862A26E2F799929FAF17B14507BD3C584622ABB53EED310644DE\
    // C242CF10C2CC3F5B1AF9DA5A9FA4E7EF60D282F1C7724A7C49168ABF083DB7B1F7E05CDE1398ED2\
    // D0FFF9A2DCEB4835FDD9FF3F0338551F0E202CEAD1A698167F469B3A1C1D5352B15E965BBE2F5D5\
    // 197186A66E7693E2F3CC708D27022



	
	/* TODO:
			1. go to some place with linux, make the long integers into arrays of uint32_t [32]
			2. finish mont (A,B,m)
				- find out how to get the results into an array instead of just printing it
			3. finish encryption and decryption 
	 */
	
	
	
	
	
	
	
	
	

    /////////// Encryption
    void encryption(uint32_t rsqModM, uint32_t rModM, uint32_t e, uint32_t message, uint32_t output_ciphertext));



    /////////// Print out encryption results
    print(output_ciphertext);



    /////////// Decryption
    void decryption(uint32_t rsqModM, uint32_t rModM, uint32_t d, uint32_t output_ciphertext, uint32_t output_message);



    /////////// Print out decryption results
    print(output_message);



    /////////// Compare the encrypted & decrypted results









    cleanup_platform();
    return 0;
}
