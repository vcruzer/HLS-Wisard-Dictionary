#include "string.h"
#include "hls-wisard.hpp"
#include "ap_int.h"
#include "math.h"


#define HTABLE_ADDR_LENGHT 8
#define INPUT_ADDR_LENGHT 28 // for each value on the input, this represents maximum possible size in bits
#define HTABLE_SIZE 256 //pow(2,HTABLE_ADDR_LENGHT)
#define NUM_OF_HTABLES 28
#define NUM_OF_DISCS 10
#define MAX_COLLISION 150 //maximum of collisions
#define get_bitN(value,N) ((value & (1<<N)) >> N)  // gets the 'n'th bit from a integer(value)

//uses H3 hash functions
int hash_func(int addr)
{
	//INPUT_ADDR_LENGHT = COLS - HTABLE_ADDR_LENGT = ROWS
	static const bool H3_Matrix[INPUT_ADDR_LENGHT*HTABLE_ADDR_LENGHT]={ 0 , 0 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 1 , 0 , 0 , 0 , 0 , 1 , 1 , 1 , 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 , 0 , 1 , 1 , 0 , 0 , 0 , 1 , 1 , 1 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 0 , 1 , 0 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 1 , 0 , 1 , 0 , 1 , 1 , 1 , 0 , 0 , 1 , 1 , 1 , 0 , 0 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 0 , 0 , 1 , 0 , 0 , 1 , 0 , 0 , 1 , 0 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 0 , 1 , 0 , 0 , 0 , 1 , 1 , 0 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 1 , 1 , 1 , 1 , 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0 };
	int hash_addr=0,aux=0;
	bool nbit;

	//goes through all the bits of addr
	for(int i=0; i<INPUT_ADDR_LENGHT;i++)
	{
		aux=0;
		for(int j=0;j<HTABLE_ADDR_LENGHT;j++)
		{
			//Gets the 'i'th bit from the integer addr and does 'bit-wise &' with 'i'th row of h3
			nbit = get_bitN(addr,i) & H3_Matrix[i*HTABLE_ADDR_LENGHT+j];
			aux = (aux << j) | nbit; //adds the bit value  to output a integer of HTABLE_ADDR_LENGHT

		}
		hash_addr ^= aux; // does a xor with all the values
	}

	return hash_addr;

}

void wisard(bool mode, int disc_id, volatile int *input_addr, int *res, int *res_cc)
{
	#pragma HLS INTERFACE m_axi depth=28 port=input_addr offset=slave bundle=image
	#pragma HLS INTERFACE s_axilite port=input_addr bundle=control
	#pragma HLS INTERFACE s_axilite port=mode bundle=control
	#pragma HLS INTERFACE s_axilite port=disc_id bundle=control
	#pragma HLS INTERFACE s_axilite port=res bundle=control
	#pragma HLS INTERFACE s_axilite port=res_cc bundle=control
	#pragma HLS INTERFACE s_axilite port=return bundle=control

	static int discs[NUM_OF_DISCS*NUM_OF_HTABLES*HTABLE_SIZE]; //-1 represents empty
	static bool initialize;
	int counters[NUM_OF_DISCS]={0};
	static int aux=0;
	int best_id;

	int input[NUM_OF_HTABLES]; //each input position goes to one HTABLE(RAM)
	int start=0,index=0,hash_addr=0,ram_start=0;

	#pragma HLS array_partition variable=discs block factor=256 dim=1
	#pragma HLS array_partition variable=counters block factor=10 dim=1

	if(!initialize) //initialize discriminators with -1 (only once)
	{
		for(int i =0;i<NUM_OF_DISCS*NUM_OF_HTABLES*HTABLE_SIZE;i++)
				discs[i]=-1;
		initialize = 1;
	}

	memcpy(input, (const int*) input_addr, NUM_OF_HTABLES * sizeof(int));


	if (!mode) //training
	{
		start = disc_id * NUM_OF_HTABLES * HTABLE_SIZE; // starting position of discriminator with id=disc_id
		for(int i=0; i < NUM_OF_HTABLES; i++)
		{
			hash_addr = hash_func(input[i]); //calculates hash address
			ram_start = start+(HTABLE_SIZE*i); // position where the Ram 'i' starts for discriminator with id=disc_id
			for(int k=0; k<MAX_COLLISION;k++) //treats collision by looking at the next position
			{
				index = ram_start + ((hash_addr+k)% HTABLE_SIZE); // the '%' prevents from going into other RAMs territory
				if((discs[index]==-1) || (discs[index] == input[i])) //if position empty or position already trained with that value
				{
					discs[index] = input[i]; //writes the input address to the position
					break;
				}
			}
		}

		*res=1; //returns true after completion
	}
	else // classification
	{

		for(int i=0;i<NUM_OF_HTABLES;i++)
		{
			for(int j=0;j<NUM_OF_DISCS;j++)
			{
				start = j*NUM_OF_HTABLES*HTABLE_SIZE; // jumps from disc to disc
				hash_addr = hash_func(input[i]); //calculates hash addr
				ram_start = start+(HTABLE_SIZE*i); // position where the Ram 'i' starts for discriminator with id=disc_id
				for(int k=0; k<MAX_COLLISION;k++)
				{
					index = ram_start + ((hash_addr+k)% HTABLE_SIZE); // position where the Ram 'i' starts for discriminator with id=disc_id
					if(discs[index]== input[i]) //if the addr is the same was found
					{
						counters[j] += 1;
						break;
					}
					else if(discs[index]==-1) //if empty position
					{
						// counter[i]+=0;
						break;
					}
				}
			}
		}

		//finds the best response
		best_id=0;
		for(int i=1;i<NUM_OF_DISCS;i++)
		{
			best_id = counters[best_id]<counters[i] ? i:best_id;
		}

		*res = best_id;
		*res_cc = counters[best_id];
	}

}
