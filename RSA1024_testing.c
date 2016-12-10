// Compile command: gcc -lgmp rsa1024.c -o rsa1024
// Run command: ./rsa1024
// You may dump the o/p in a file: ./rsa1024>log1024.txt

#include<stdio.h>
#include<gmp.h>

#define GET_VAR_NAME(Variable)(#Variable)


void print_wordarray(mpz_t in)
{
	mpz_t temp;
	mpz_init(temp);
	mpz_t word;
	mpz_init(word);
	
	mpz_t mask;
	mpz_init(mask);
	mpz_set_str(mask, "4294967295", 10);

	int i;

	mpz_set(temp, in);
	printf("32-bit words [least-most]:\n");
	for(i=0; i<32; i++)
	{
		mpz_and(word, temp, mask);
		mpz_sub(temp, temp, word);
		mpz_fdiv_q_2exp(temp, temp, 32);
		if(i<31)
		gmp_printf("%ZX, ", word);
		else
		gmp_printf("%ZX", word); 
	}
	printf("\n");
}

void montgo1024(mpz_t A_in, mpz_t B_in, mpz_t C_out)	// C = Mont(A, B)
{
	int i;
	mpz_t M, one, two;
	mpz_init(M); mpz_init(one); mpz_init(two); 
	mpz_t A; mpz_init(A);
	mpz_t B; mpz_init(B);	
	mpz_t C; mpz_init(C);
	
	mpz_set(A, A_in);
	mpz_set(B, B_in);


	mpz_set_str(one, "1", 10);
	mpz_set_str(two, "2", 10);


	//mpz_set_str(A, "1BA", 16);	// e.g. hexadecimal value of input operand A
	//mpz_set_str(B, "91B", 16);	// e.g. hexadecimal value of input operand B
	mpz_set_str(M, "5CEFC1C526A44E4F196A2E86809DC0EC882447E5568CCE0C19FFBA87FD975064E76A\
CC86B576CF6D5BABC44FB8C0B891BECCE05E9A3CE8AEA487EE14643DCC483C46D7E0DAB02388FA8\
B6B92F346F55638ECAC8B975B79668CBDBB09A82DD132D9D635D68360E64A6F4AFBA0B3F79F6658\
C227FA5E067888759258C224BBF1F3", 16);


	int lsb;

	mpz_set_str(C,"0",10);
	for(i=0;i<1024;i++)
	{
	       	lsb = mpz_divisible_p(A, two);	// lsb is nonzero when a is even
		if(lsb == 0)	// A is odd
		{
			mpz_sub(A, A, one);		// a <-- a-1
			mpz_add(C, C, B);		// C = C + B		 
       		}			
		mpz_cdiv_q(A, A, two);		// a <-- a/2

	       	lsb = mpz_divisible_p(C, two);	// lsb is nonzero when R is even
		if(lsb == 0)	// R is odd
		{
			mpz_add(C, C, M);		// R <-- R + M
		}
		mpz_cdiv_q(C, C, two);			// R <-- R/2
		
		//printf("[i]: %d\n", i);
		//gmp_printf("[A]: %ZX\n", A);
		//gmp_printf("[C]: %ZX\n\n", C);
	}

	if(mpz_cmp(C, M) >= 0)				// C >= M
	mpz_sub(C, C, M);

	mpz_set(C_out, C);
}

void rsa1024(mpz_t x, mpz_t e, mpz_t c)
{
	mpz_t rmodm, r2modm;
	mpz_init(rmodm); mpz_init(r2modm);
	
	mpz_t x_tilda;
	mpz_init(x_tilda);

	mpz_t A;
	mpz_init(A);
	int i;	

	mpz_t two_pow1023; mpz_init(two_pow1023);	
	mpz_t one; mpz_init(one);

	mpz_set_str(rmodm, "46207C75B2B76361CD2BA2F2FEC47E26EFB7703552E663E7CC008AF004D15F36312A66F\
29512612548A877608E7E8EDC82663F42CB862EA2B6F023D73784676F8772503E4A9FB8EE0AE928\
DA197215538E26A6E8D1490D32E68489ECAFA45D9A4C539452F93E336B216A08BE9810C1334E7BB\
00B43F30EEF14DB4E7BB6881C1A", 16);

	mpz_set_str(r2modm, "187C74C74D55B6F97A8F5F54900034AAD155419F0514032AC0F25644D2748F7E9E7DC8\
26CF649200B9D3F3E44FB6921206962D2B087E2AE020D5C5A12474E0166A18D5B2009458F406116\
E78B18A35B703F734A58D427386D3700427FD89D50854D4A876F16540440CD5908AEE3860140B60\
638EC738C287FAF6E1B18CF805FD", 16);
	
	mpz_set_ui(two_pow1023, 1);
	mpz_mul_2exp(two_pow1023, two_pow1023, 1023);	
	mpz_set_ui(one, 1);


	// pre-computation
	montgo1024(x, r2modm, x_tilda);
	mpz_set(A, rmodm);
	gmp_printf("Precomputation Result =%ZX\n\n", x_tilda);

	int bit;
	int first_nonzero_bit_found = 0;
	for(i=1023; i>=0; i--)
	{
		// scan the bits of the exponent
		if(mpz_cmp(e, two_pow1023)>=0) bit = 1; else bit = 0;
		//shift the exponent by one bit
		if(bit==1) mpz_sub(e, e, two_pow1023);
		mpz_mul_2exp(e, e, 1);
		if(bit==1) first_nonzero_bit_found = 1;

		//printf("i = %d bit = %d\n", i, bit); 
		if(first_nonzero_bit_found == 1)
		{
			// Montgomery squqring
			montgo1024(A, A, A);
			gmp_printf("Squaring Result =%ZX\n\n",A);
			
			// Montgomery multiplication
			if(bit==1)
			{
				montgo1024(A, x_tilda, A);	
				gmp_printf("Conditional Multiplication Result =%ZX\n\n",A);
			}
		}
	}

	montgo1024(A, one, c);
}

main()
{
	// initialise test vector values	
	mpz_t message, ciphertext, decrypted_message, e, d, difference, M;
	mpz_init(message); 	mpz_init(ciphertext); mpz_init(decrypted_message); 
	mpz_init(e); mpz_init(d); mpz_init(difference); mpz_init(M);		

   	gmp_randstate_t r_state;

    int seed = 123456;

    gmp_randinit_default (r_state);
    gmp_randseed_ui(r_state, seed);


	mpz_set_str(M, "5CEFC1C526A44E4F196A2E86809DC0EC882447E5568CCE0C19FFBA87FD975064E76A\
CC86B576CF6D5BABC44FB8C0B891BECCE05E9A3CE8AEA487EE14643DCC483C46D7E0DAB02388FA8\
B6B92F346F55638ECAC8B975B79668CBDBB09A82DD132D9D635D68360E64A6F4AFBA0B3F79F6658\
C227FA5E067888759258C224BBF1F3", 16);

	int COUNTER;

	for(COUNTER=0; COUNTER<1; COUNTER++)
	{
		mpz_urandomb(message, r_state,1023);
		mpz_mod(message, message, M);
		gmp_printf("message=%ZX\n", message);

		mpz_set_str(e, "101", 16);
		mpz_set_str(d, "397F69EF837C90130DBB37ADDC05F4DE0A8404A2CDF380790B1ABA205\
	62AD01E8D2E26E0C9EADFF4F6F8B6374C733F63225EE07A4784503443EC805E4BF66B74A0109153\
	20FF91B8C6EAAA9AA88256A3A11DE2FF803B6C49412F0B0CD973EAE6687629F7C78F65491FD8473\
	BB8731DA07F7156D678800FC92B9DC874E8C0D201", 16);

		rsa1024(message, e, ciphertext);
	
		gmp_printf("Ciphertext=%ZX\n", ciphertext);

		rsa1024(ciphertext, d, decrypted_message);
		//print_wordarray(c);
		
		gmp_printf("Decrypted message=%ZX\n", decrypted_message);

		mpz_sub(difference, decrypted_message, message);

		gmp_printf("Difference = %Zd\n", difference);
	}

}




