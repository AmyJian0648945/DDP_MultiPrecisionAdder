/*
 * mp_arith.c
 *
 */
#include <stdint.h>
// Calculates res = a + b.
// a and b represent large integers stored in uint32_t arrays
// a and b are arrays of size elements, res has size+1 elements


void mp_add(uint32_t *a, uint32_t *b, uint32_t *res, uint32_t size)
{
	uint32_t c = 0;
	int i = 0;
	uint64_t  temp=0;

	for(i = 0;i < size;i++){
		res[i] = a[i] + b[i] + c;
		if(a[i] > 4294967295 - b[i]) c = 1;
		else c = 0;
	}
	res[size] = c;
}


// Calculates res = a - b.
// a and b represent large integers stored in uint32_t arrays
// a, b and res are arrays of size elements


void mp_sub(uint32_t *a, uint32_t *b, uint32_t *res, uint32_t size)
{
	uint32_t c = 0;
	int i = 0;
	uint32_t temp;
	for(i = 0;i < size;i++){
		res[i]= a[i] - b[i] + c;
		if(a[i] >= b[i]) c = 0;
		else c =-1;
	}
}


// Calculates res = (a + b) mod N.
// a and b represent operands, N is the modulus. They are large integers stored in uint32_t arrays of size elements


void mod_add(uint32_t *a, uint32_t *b, uint32_t *N, uint32_t *res, uint32_t size)
{
	uint32_t c = 0;
	int i = 0;
	int c2 = 0;
	int flag = 0;
	uint32_t  temp;
	for(i = 0 ;i < size ;i++){

		res[i] = a[i] + b[i] + c;
		if(a[i] > 4294967295 - b[i]) c = 1;
		else c = 0;
	}

	res[size] = c;

	for(i = size - 1 ; i >= 0 ; i--){
		if(res[i] > N[i]){
			flag = 1;
			break;
		}
	}

	if(flag == 1  || res[size] == 1 ){
		for(i = 0 ; i <size ; i++){
			temp = res[i] - N[i] + c2;
			if(res[i] >= N[i]) c2 = 0;
			else c2 = -1;
			res[i] = temp;
		}
	}
}

// Calculates res = (a - b) mod N.
// a and b represent operands, N is the modulus. They are large integers stored in uint32_t arrays of size elements
void mod_sub(uint32_t *a, uint32_t *b, uint32_t *N, uint32_t *res, uint32_t size)
{
	uint32_t c = 0;
	int i = 0;
	int c2 = 0;
	int c3 = 0;
	int flag = 0;
	uint32_t temp;

	for(i = size-1 ; i >=0 ; i--){
		if(a[i] < b[i]){
			flag = 1;
			break;
		}
	}


	if(flag==1){
		for( i = 0 ;i < size;i++){
			temp = a[i];
			a[i] = b[i];
			b[i] = temp;
		}
		flag = 1;
	}
	for( i = 0 ; i < size ; i++){
		res[i] = a[i] - b[i]+c;
		if(a[i] >= b[i]) c = 0;
		else c = -1;
	}
	
		if( res[31] > N[31] || res[size] == 1 ){
			for(i = 0 ; i <size ; i++){
				temp = res[i]-N[i] + c2;
				if(res[i] >= N[i]) c2=0;
				else c2=-1;
				res[i] = temp;
			}
		}

	if(flag==1){
		for( i = 0 ; i < size ; i++){
				res[i]= N[i] - res[i] + c3;
				if(N[i] >= res[i]) c3 = 0;
				else c3 = -1;
			}
	}
}


