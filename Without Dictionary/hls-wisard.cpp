#include "string.h"
#include "hls-wisard.hpp"

// TODO change values
#define RAM_SIZE 8
#define NUM_OF_RAMS 112
#define NUM_OF_DISCS 2

//1 discriminator = 28 rams with 2^28 positions


void wisard(bool mode, int disc_id, volatile int *img_addr, int *res)
{
	#pragma HLS INTERFACE m_axi depth=112 port=img_addr offset=slave bundle=image
	#pragma HLS INTERFACE s_axilite port=img_addr bundle=control
	#pragma HLS INTERFACE s_axilite port=mode bundle=control
	#pragma HLS INTERFACE s_axilite port=disc_id bundle=control
	#pragma HLS INTERFACE s_axilite port=res bundle=control
	#pragma HLS INTERFACE s_axilite port=return bundle=control

	static bool discs[NUM_OF_DISCS*NUM_OF_RAMS*RAM_SIZE];
	int counters[NUM_OF_DISCS]={0};

	int input[NUM_OF_RAMS];
	int start=0,index=0,i,j,best_id;

	memcpy(input, (const int*) img_addr, NUM_OF_RAMS * sizeof(int));


	if (!mode) //training
	{
		start = disc_id * NUM_OF_RAMS * RAM_SIZE; // starting position of discriminator with disc_id
		for(i =0; i < NUM_OF_RAMS; i++)
		{
			index = (start+(RAM_SIZE*i))+input[i];
			discs[index] = 1;
		}

		*res=1; //returns true after completion
	}
	else // classification
	{

		for(j=0;j<NUM_OF_RAMS;j++)
		{
			for(i=0;i<NUM_OF_DISCS;i++)
			{
				start = i*NUM_OF_RAMS*RAM_SIZE; // jumps from disc to disc
				counters[i] += discs[(start+(RAM_SIZE*j))+input[j]];
			}
		}

		//finds the best response
		best_id=0;
		for(i=1;i<NUM_OF_DISCS;i++)
		{
			best_id = counters[best_id]<counters[i] ? i:best_id;
		}

		*res = best_id;
	}

}
