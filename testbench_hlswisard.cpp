#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "hls-wisard.hpp"


int main(int argc, char **argv)
{

	//int map[112]={5,0,6,6,1,4,4,0,0,2,2,3,0,2,5,0,0,1,3,0,2,7,1,0,1,2,4,0,4,0,1,0,2,1,4,0,5,0,4,0,5,4,0,4,0,7,0,1,6,1,4,2,0,4,4,2,6,5,3,3,2,5,1,0,4,2,3,1,2,1,4,2,5,0,4,4,4,6,2,2,4,0,1,6,0,6,2,6,2,3,4,7,0,4,0,2,1,6,0,2,3,0,4,6,2,0,0,0,5,4,4,4};
	int map[5] = {0,3,1,1,2};
	int res_id=0,res_sum=0;

	//wisard(bool mode, int disc_id, volatile int *img_addr, int *res)

	wisard(0,0,map,&res_id);//training disc 0

	printf("After Train-> ID: %d\n",res_id);

	wisard(1,0,map,&res_id);

	printf("After Test-> ID: %d\n",res_id); //should output 0;

	for(int i=0;i<5;i=i+1)
		map[i] = 1;

	wisard(0,1,map,&res_id);//training disc 1

	printf("After Train-> ID: %d\n",res_id);

	wisard(1,0,map,&res_id);

	printf("After Test-> ID: %d\n",res_id); //should output 1;

	for(int i=0;i<5;i=i+1)
		map[i] = 2;

	wisard(0,0,map,&res_id);//training disc 0

	printf("After Train-> ID: %d\n",res_id);

	wisard(1,0,map,&res_id);

	printf("After Test-> ID: %d\n",res_id); //should output 0;

	return 0;
}

