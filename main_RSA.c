#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "testVector.h"

//Uses an AXI DMA IP Core
//Transfers data from data_read_addr to the M_AXIS_MM2S port
//The sequence is explained in detail in AXI DMA v7.1 - LogiCORE IP Product Guide
//(http://www.xilinx.com/support/documentation/ip_documentation/axi_dma/v7_1/pg021_axi_dma.pdf) on page 72)

//Divide by four to get the 32-bit offsets
#define MM2S_DMACR_OFFSET (0/4)
#define MM2S_SA_OFFSET (0x18/4)
#define MM2S_LENGTH_OFFSET (0x28/4)

unsigned int * dma_config         = (unsigned int *)0x40400000;
unsigned int * my_montgomery_port = (unsigned int *)0x43C00000;
unsigned int * my_montgomery_data = (unsigned int *)0x43C10000;

//1024 = 32 * 32
#define DMA_TRANSFER_NUMBER_OF_WORDS 32

//Simple test: mwr 0x40400000 1; mwr 0x40400028 1; mwr 0x40400018 0x100000;
void bram_dma_transfer(unsigned int* dma_config, unsigned int * data_addr, unsigned int data_len){
    Xil_DCacheFlush(); //Flush the cache to ensure that fresh data is stored inside DRAM

    //Perform the DMA transfer
    dma_config[MM2S_DMACR_OFFSET] = 1; //Stop
    dma_config[MM2S_SA_OFFSET] = (int)data_addr; //Specify read address
    dma_config[MM2S_LENGTH_OFFSET] = data_len*4; //Specify number of bytes
}

void test_dma_transfer(){
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

void port2_wait_for_done(){
    while(my_montgomery_port[1]==0)  //Read a response from P2
    {}
}

void print_bram_contents(){
    int i;
    xil_printf("BRAM contents:\n\r");
    for (i=0; i<DMA_TRANSFER_NUMBER_OF_WORDS; i+=4)
        xil_printf("%08x %08x %08x %08x\n\r",my_montgomery_data[i], my_montgomery_data[i+1], my_montgomery_data[i+2], my_montgomery_data[i+3]);
    xil_printf("\n\r");
}





void mont(uint32_t *result, uint32_t *A, uint32_t *B, uint32_t *m){

    xil_printf("Doing Mont...");

    // Send message
    //xil_printf("Sending A...\n\r");
    my_montgomery_port[0] = 0x0; //Send a command to P1
    bram_dma_transfer(dma_config, A, DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1


    // Send rsqModM
    //xil_printf("Sending B...\n\r");
    my_montgomery_port[0] = 0x0; //Send a command to P1
    bram_dma_transfer(dma_config, B, DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1


    // Send modullus
    //xil_printf("Sending m...\n\r");
    my_montgomery_port[0] = 0x0; //Send a command to P1
    bram_dma_transfer(dma_config, m, DMA_TRANSFER_NUMBER_OF_WORDS);
    port2_wait_for_done(); //Wait until Port2=1


    // Compute Mont(message, rsqModM)
    //xil_printf("Computing...");
    my_montgomery_port[0] = 0x1; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1
    //print_bram_contents(); //Print BRAM to serial port.


    // Print x_delta values
/*
    xil_printf("Results: \n\r");
    my_montgomery_port[0] = 0x2; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents(); //Print BRAM to serial port.
*/
    xil_printf("Done!\n\r");
    int i = 0;

    // Give the results to 'result'
    for (i=0; i<DMA_TRANSFER_NUMBER_OF_WORDS; i+=4){
        result[i]   = my_montgomery_data[i];
        result[i+1] = my_montgomery_data[i+1];
        result[i+2] = my_montgomery_data[i+2];
        result[i+3] = my_montgomery_data[i+3];
    }

}



void print_output(uint32_t *output){
    int i;
    xil_printf("Encrypted/Decrypted content:\n\r");
    for (i=0; i<32; i+=4)
        xil_printf("%08x %08x %08x %08x\n\r",output[i], output[i+1], output[i+2], output[i+3]);
}



void encryption(uint32_t *rsqModM, uint32_t *rModM, uint32_t e, uint8_t numOfBits, uint32_t *modullus, uint32_t *input_message, uint32_t *output_ciphertext){





    /////////// Initialise variables: A, x_delta
    uint32_t A[32] = {0}, x_delta[32] = {0}, temp[32] = {0};
    uint32_t bin = 0b100000000;  //use this to bitwise-AND
    int i = 0;

    for(i = 0; i < 32; i++){ A[i] = rModM[i]; } // A = rModM
    mont(x_delta, input_message, rsqModM, modullus);  // x_delta = Mont(message, rsqModM)




    /////////// Calculate A depending on e(i) ///////////
    // With each iteration, bin = bin * 2 to get the next bit


    for(i = numOfBits; i >= 0; i--){
        mont(A, A, A, modullus);
        if((e & bin) != 0) mont(A, A, x_delta, modullus);
        //xil_printf("[e = %x, b = %d, res = %d]\n\r", e, bin, (e & bin));
        bin = bin >> 1;
    }





    xil_printf("////////////////////////\n\rDEBUGGING HERE: \n\r");
    print_output(A);
    xil_printf("////////////////////////\n\rOUT__BUGGING HERE: \n\r");



    //temp[0] = 0x1;
    /////////// Finalise A ///////////
    mont(output_ciphertext, A, 0x1, modullus);


}





void decryption(uint32_t *rsqModM, uint32_t *rModM, uint32_t *d, uint8_t numOfBits, uint32_t *modullus, uint32_t *input_ciphertext, uint32_t *output_message){

    /////////// Initialise variables: A, x_delta
    uint32_t A[32], x_delta, temp;
    uint32_t bin = 0b10000000000000000000000000000000;  //use this to bitwise-AND
    int i = 0, j = 0;

    for(i = 0; i < 32; i++){ A[i] = rModM[i]; } // A = rModM
    mont(x_delta, input_ciphertext, rsqModM, modullus);  // x_delta = Mont(message, rsqModM)


    /////////// Calculate A depending on e(i) ///////////
    // With each iteration, bin = bin * 2 to get the next bit
    // i toggles d[i]
    // j toggles the bits within d[i]
    for(i = numOfBits; i >= 0; i--){
        temp = d[i];
        for(j = 32; j > 0; j--){
            if(temp & bin == 0) mont(A, A, A,       modullus); // A = mont(A, A) with implicit mod modullus
            else mont(A, A, x_delta, modullus);

            bin = bin >> 1; // bin = bin / 2;
        }
    }






    /////////// Finalise A ///////////
    mont(output_message, A, 1, modullus);



}




int main()
{
    int i;
    uint8_t numOfBits;
    uint32_t output_ciphertext[32]={0}, output_message[32]={0};



    init_platform();
    xil_printf("\n\n\n\n\n\n\n\nStartup..\n\r");
    test_dma_transfer();

    /////////// Initialize variables - change 'testVector.h' if necessary!
    // Seed = 2016.
    // numbers are least significant to most significant





/*
    //TESTING PURPOSES ONLY -

    uint32_t a[32] = {0x1ba, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 , 0x0};
    uint32_t b[32] = {0x91b, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    uint32_t N[32] = {0xe67a6f31, 0xa677bce5, 0x3b9e66f8, 0x32dff402, 0x2ab9a7c8, 0xbe08bc1e, 0x2ade1186, 0x8e88e8cf, 0x716a5c3b, 0x5453c47, 0x93ade446, 0xdc771f42, 0xe1709725, 0x7f89ebb5, 0x1d501ea9, 0x93c9933, 0xcf1ae468, 0x56ba1eb0, 0x358e7c24, 0x2daae084, 0x4da058ec, 0x27028a89, 0x4f2e5438, 0x29ab96b2, 0xf9072101, 0xe7990185, 0x199af8fe, 0x6887926d, 0xcfb0450e, 0x36a53dd9, 0xed3982c, 0xd1d3b4d6};
    uint32_t result[32] = {0};

    mont(result, a, b, N);
    print_output(result);
*/



    uint32_t rsqModM[32]={0}, rModM[32]={0}, modullus[32]={0}, d[32]={0}, message[32]={0};


    // FLIPPIN
    for(i=0; i<32; i++){
        rsqModM[i] = rsqModM_t[31-i];
        rModM[i] = rModM_t[31-i];
        modullus[i] = modullus_t[31-i];
        d[i] = d_t[31-i];
        message[i] = message_t[31-i];
    }




    /////////// Encryption
    numOfBits = 9; // t = number of bits of e IN BINARY THOUGH
    encryption(rsqModM, rModM, e, numOfBits, modullus, message, output_ciphertext);



    /////////// Print out encryption results
    print_output(output_ciphertext);


/*

    /////////// Decryption
    //numOfBits = 32;
    //decryption(rsqModM, rModM, d, numOfBits, modullus, output_ciphertext, output_message);



    /////////// Print out decryption results
    //print_output(output_message);



    /////////// Compare the encrypted & decrypted results
    //output_ciphertext - output_message;
     *
     *
*/

    /************************
    DEBUG TODO:
    - check the warnings!
    - make sure mont is correct
        - make sure that the output of it is correct
    - double check that the mont exponentiation is following the correct flow:
        -
    -
    - make sure that the array is processing through the info in the correct order






    ************************/




    cleanup_platform();
    return 0;
}
