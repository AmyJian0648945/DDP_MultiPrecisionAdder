// compile command: gcc formatting.c -lgmp -o formatting
// run: ./formatting

#include<gmp.h>
#include<stdio.h>

main()
{
	int i;
	mpz_t in;
	mpz_init(in);
	mpz_t mask, word;
	mpz_init(mask); mpz_init(word);

	mpz_set_str(mask, "4294967295", 10);

	
	printf("enter the 1024 bit input in hexadecimal\n");
	gmp_scanf("%Zx", in);

	//printf("enter the input in decimal\n");
	//gmp_scanf("%Zd", in);
	
	printf("Printing in 32-bit words from least-significant to most significant\n");
	for(i=0; i<32; i++)
	{
		mpz_and(word, in, mask);	
		mpz_sub(in, in, word);
		mpz_fdiv_q_2exp(in, in, 32);
		
		if(i<31)
		gmp_printf("0x%Zx, ", word);
		if(i==31)
		gmp_printf("0x%Zx;\n", word);		
	}
}
