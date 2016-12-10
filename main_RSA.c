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

#define GET_VAR_NAME(Variable)(#Variable)

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

    xil_printf("Results: \n\r");
    my_montgomery_port[0] = 0x2; //Send a command to P1
    port2_wait_for_done(); //Wait until Port2=1
    print_bram_contents(); //Print BRAM to serial port.

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
    //xil_printf("Printing: %x\n\r", GET_VAR_NAME(*output));

    for (i=0; i<32; i+=4)
        xil_printf("%08x %08x %08x %08x\n\r",output[i], output[i+1], output[i+2], output[i+3]);
	xil_printf("\n\r");
}








void encryption(uint32_t *rsqModM, uint32_t *rModM, uint32_t e, uint8_t numOfBits, uint32_t *modullus, uint32_t *input_message, uint32_t *output_ciphertext){





    /////////// Initialise variables: A, x_delta
    uint32_t A[32] = {0}, x_delta[32] = {0}, temp[32] = {0};
    uint32_t bin = 0b100000000;  //use this to bitwise-AND
    int i = 0;


    xil_printf("Check if x_delta, input_message, rsqModM, modullus are correct:\n\r");
    print_output(x_delta);
    print_output(input_message);
    print_output(rsqModM);
    print_output(modullus);





    for(i = 0; i < 32; i++){ A[i] = rModM[i]; } // A = rModM
    mont(x_delta, input_message, rsqModM, modullus);  // x_delta = Mont(message, rsqModM)


    // FOR DEBUGGING - CHECK IF X_DELTA AND A ARE CORRECT
    xil_printf("check if x_delta and A are correct!: \n\r");
	print_output(x_delta);
	print_output(A);
    /////////// Calculate A depending on e(i) ///////////
    // With each iteration, bin = bin * 2 to get the next bit


    for(i = numOfBits; i >= 0; i--){
        mont(A, A, A, modullus);
		print_output(A);
        if((e & bin) != 0) {
			mont(A, A, x_delta, modullus);
			print_output(A);
		}
        //xil_printf("[e = %x, b = %d, res = %d]\n\r", e, bin, (e & bin));
        bin = bin >> 1;
    }



   

    //temp[0] = 0x1;
    /////////// Finalise A ///////////
    mont(output_ciphertext, A, 0x1, modullus);


}





void decryption(uint32_t *rsqModM, uint32_t *rModM, uint32_t *d, uint8_t numOfBits, uint32_t *modullus, uint32_t *input_ciphertext, uint32_t *output_message){

    /////////// Initialise variables: A, x_delta
    uint32_t A[32], x_delta, temp;
    uint32_t bin = 0b10000000000000000000000000000000;  //use this to bitwise-AND
    uint8_t i = 0, j = 0, flag = 0;

    for(i = 0; i < 32; i++){ A[i] = rModM[i]; } // A = rModM
    mont(x_delta, input_ciphertext, rsqModM, modullus);  // x_delta = Mont(message, rsqModM)


    /////////// Calculate A depending on e(i) ///////////
    // With each iteration, bin = bin * 2 to get the next bit
    // i toggles the words 
    // j toggles the bits within d[i]
    for(i = numOfBits; i >= 0; i--){
        temp = d[i];
        for(j = 32; j > 0; j--){
            if(flag == 0){ // makes sure first bit to compute is 1
                if(temp & bin != 0){ 
                    mont(A, A, x_delta, modullus);
                    flag = 1;
                }
            }
            else{ // starts doing the mont stuff
                if(temp & bin == 0) mont(A, A, A, modullus); // A = mont(A, A) with implicit mod modullus
                else                mont(A, A, x_delta, modullus);
            }

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


    uint32_t rsqModM_t[32] = { 0x8cf805fd, 0xfaf6e1b1, 0xc738c287, 0xb60638e, 0xee386014, 0xcd5908a, 0xf1654044, 0x54d4a876, 0xfd89d508, 0xd3700427, 0x8d427386, 0x3f734a5, 0xb18a35b7, 0x6116e78, 0x9458f4, 0x6a18d5b2, 0x2474e016, 0x20d5c5a1, 0x87e2ae0, 0x6962d2b, 0x4fb69212, 0xb9d3f3e4, 0xcf649200, 0x9e7dc826, 0xd2748f7e, 0xc0f25644, 0x514032a, 0xd155419f, 0x900034aa, 0x7a8f5f54, 0x4d55b6f9, 0x187c74c7  };
    uint32_t rModM_t[32] = {  0xb6881c1a, 0x14db4e7b, 0x43f30eef, 0x4e7bb00b, 0x9810c133, 0x216a08be, 0xf93e336b, 0x4c539452, 0xafa45d9a, 0xe68489ec, 0xd1490d32, 0x8e26a6e8, 0x19721553, 0xae928da, 0x4a9fb8ee, 0x8772503e, 0x3784676f, 0xb6f023d7, 0xcb862ea2, 0x82663f42, 0x8e7e8edc, 0x48a87760, 0x95126125, 0x312a66f2, 0x4d15f36, 0xcc008af0, 0x52e663e7, 0xefb77035, 0xfec47e26, 0xcd2ba2f2, 0xb2b76361, 0x46207c75  };
    uint32_t e = 0x101;
    uint32_t modullus_t[32] = { 0x24bbf1f3, 0x759258c2, 0x5e067888, 0x58c227fa, 0xb3f79f66, 0x6f4afba0, 0x8360e64a, 0xd9d635d6, 0xa82dd132, 0x8cbdbb09, 0x975b7966, 0x38ecac8b, 0xf346f556, 0xfa8b6b92, 0xdab02388, 0x3c46d7e0, 0x643dcc48, 0xa487ee14, 0x9a3ce8ae, 0xbecce05e, 0xb8c0b891, 0x5babc44f, 0xb576cf6d, 0xe76acc86, 0xfd975064, 0x19ffba87, 0x568cce0c, 0x882447e5, 0x809dc0ec, 0x196a2e86, 0x26a44e4f, 0x5cefc1c5  };
    uint32_t d_t[32] = { 0xe8c0d201, 0x2b9dc874, 0x78800fc9, 0x7f7156d6, 0xb8731da0, 0x1fd8473b, 0xc78f6549, 0x687629f7, 0xd973eae6, 0x412f0b0c, 0x803b6c49, 0xa11de2ff, 0xa88256a3, 0xc6eaaa9a, 0x20ff91b8, 0xa0109153, 0x4bf66b74, 0x43ec805e, 0x47845034, 0x225ee07a, 0x4c733f63, 0xf6f8b637, 0xc9eadff4, 0x8d2e26e0, 0x562ad01e, 0xb1aba20, 0xcdf38079, 0xa8404a2, 0xdc05f4de, 0xdbb37ad, 0x837c9013, 0x397f69ef };
    uint32_t message_t[32] = {0x6395c695, 0x3f4bd35b, 0x4776e77d, 0xfbd369c, 0x19384a9e, 0x7c662a00, 0xf0ac06d9, 0x6e3d9096, 0x15d274b5, 0x44a415b0, 0xb0233f1d, 0x2cf0a6d3, 0x33c43a4e, 0x9a5df015, 0x87c4b38e, 0x2fdb4086, 0x2d29831d, 0x5414383e, 0x7c853481, 0x977e9e01, 0x9fb4ecfc, 0x90036226, 0x1ee5cba8, 0xb5476321, 0xe781ee92, 0xbec3335b, 0x1e005a53, 0xd7e014e2, 0x165a3d1a, 0xf550bd84, 0xd03de810, 0x815cb73 };



    //815CB73D03DE810F550BD84165A3D1AD7E014E21E005A53BEC3335BE781EE92B54763211EE5CBA8900362269FB4ECFC977E9E017C8534815414383E2D29831D2FDB408687C4B38E9A5DF01533C43A4E2CF0A6D3B0233F1D44A415B015D274B56E3D9096F0AC06D97C662A0019384A9E0FBD369C4776E77D3F4BD35B6395C695

    init_platform();
    xil_printf("\n\n\n\n\n\n\n\nStartup..\n\r");
    test_dma_transfer();

    /////////// Initialize variables - change 'testVector.h' if necessary!
    // Seed = 2016.
    // numbers are least significant to most significant




    uint32_t result[32] = {0};

    uint32_t a[32] = {0x1ba, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 , 0x0};
    uint32_t b[32] = {0x91b, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    uint32_t N[32] = {0xe67a6f31, 0xa677bce5, 0x3b9e66f8, 0x32dff402, 0x2ab9a7c8, 0xbe08bc1e, 0x2ade1186, 0x8e88e8cf, 0x716a5c3b, 0x5453c47, 0x93ade446, 0xdc771f42, 0xe1709725, 0x7f89ebb5, 0x1d501ea9, 0x93c9933, 0xcf1ae468, 0x56ba1eb0, 0x358e7c24, 0x2daae084, 0x4da058ec, 0x27028a89, 0x4f2e5438, 0x29ab96b2, 0xf9072101, 0xe7990185, 0x199af8fe, 0x6887926d, 0xcfb0450e, 0x36a53dd9, 0xed3982c, 0xd1d3b4d6};

    mont(result, a, b, N);
    print_output(result);









    // FLIPPIN the values around

    for(i=0; i<32; i++){
        rsqModM[i] = rsqModM_t[31-i];
        rModM[i] = rModM_t[31-i];
        modullus[i] = modullus_t[31-i];
        d[i] = d_t[31-i];
        message[i] = message_t[31-i];
    }


    // FOR DEBUGGING - DELETE LATER
    xil_printf("check if info is being read in!: \n\r");
    print_output(modullus);



    /////////// Encryption
    numOfBits = 9; // t = number of bits of e IN BINARY THOUGH
    //encryption(rsqModM, rModM, e, numOfBits, modullus, message, output_ciphertext);
    encryption(b, rModM, e, numOfBits, N, a, output_ciphertext);

    /////////// Print out encryption results
    print_output(output_ciphertext);




/*

    /////////// Decryption
    //numOfBits = 1024;
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

