#include "mnist_operations.h"
#include "xstatus.h"
#include <stdlib.h>
#include <xil_printf.h>
#include <ff.h>
#include <string.h>

//Swap byte for unsigned int
unsigned int swap_uint32( unsigned int val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | (val >> 16);
}

unsigned char *
read_label_idx(TCHAR * filename, unsigned int * n)
{
	static FIL fil; /* File object */
	FRESULT Res;
	UINT * br;

	Res = f_open(&fil, filename, FA_READ); //reads file
	if (Res) {
		xil_printf("Error fopen %s\n\r",filename);
		return XST_FAILURE;
	}

    unsigned short zero;
    unsigned char data_type, dims;

    f_read(&fil,&zero,2,br);
    f_read(&fil,&data_type,1,br);
    f_read(&fil,&dims,1,br);
    f_read(&fil,n,4,br);

   // fread(&zero, 2, 1, fd);
   // fread(&data_type, 1, 1, fd);
   // fread(&dims, 1, 1, fd);
   // fread(n, 4, 1, fd);

    //Convert to Big-endian
    *n  = swap_uint32(*n); //Number of images

    unsigned char * data = (unsigned char *) malloc(sizeof(unsigned char) * *n);
    unsigned int i;

    for (i = 0; i < *n; i++) {
       // fread(&data[i], 1, 1, fd);
    	f_read(&fil,&data[i],1,br);
    }

    xil_printf("Read file %s\n",filename);
    f_close(&fil);

    return data;
}

unsigned char ***
read_data_idx(TCHAR * filename, unsigned int * n, unsigned int * rows, unsigned int * cols)
{
	static FIL fil; /* File object */
	FRESULT Res;
	UINT * br;

	Res = f_open(&fil, filename, FA_READ); //reads file
	if (Res) {
		xil_printf("Error fopen %s\n\r",filename);
		return XST_FAILURE;
	}

    unsigned short zero;
    unsigned char data_type, dims;
    f_read(&fil,&zero,2,br);
    f_read(&fil,&data_type,1,br);
    f_read(&fil,&dims,1,br);
    f_read(&fil,n,4,br);
    f_read(&fil,rows,4,br);
    f_read(&fil,cols,4,br);
    /*fread(&zero, 2, 1, fd);
    fread(&data_type, 1, 1, fd);
    fread(&dims, 1, 1, fd);
    fread(n, 4, 1, fd);
    fread(rows, 4, 1, fd);
    fread(cols, 4, 1, fd);*/

    //Convert to Big-endian
    *n  = swap_uint32(*n); //Number of images
    *rows  = swap_uint32(*rows); //Number of rows
    *cols  = swap_uint32(*cols); //Number of columns

    unsigned char *** data = (unsigned char ***) malloc(sizeof(unsigned char **) * *n);
    unsigned int i, j, k;

    //xil_printf("N:%d ROWS:%d COLS:%d\n",*n,*rows,*cols);
    for (i = 0; i < *n; i++) {
    	//xil_printf("%d\n",i);
        data[i] = (unsigned char **) malloc(sizeof(unsigned char *) * *rows);

        for (j = 0; j < *rows; j++) {
           data[i][j] = (unsigned char *) malloc(sizeof(unsigned char) * *cols);

            for (k = 0; k < *cols; k++) {
            	f_read(&fil,&data[i][j][k],1,br);
                //fread(&data[i][j][k], 1, 1, fd);
            }
        }
    }

    f_close(&fil);

    xil_printf("Read file %s\n",filename);
    return data;
}

/********************************************************/
/***************BitArray Operations**********************/
/********************************************************/
void
binarize_image_bitarray(unsigned char ** image, bitarray_t * bin_image, unsigned int rows, unsigned int cols)
{
    unsigned int i,j, k, sum = 0, total = rows * cols;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += image[i][j];
        }
    }

    float mean = ((float)sum)/((float)total);
    //printf("MEAN = %.f\n", mean);
    k=0;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
        	bitarray_setBit(bin_image,k++,image[i][j] > mean ? 1:0);

        }
    }
}

void bitarray_new(bitarray_t *self,unsigned int n_bit)
{
	self->num_bits = n_bit;
	self->bitarray_size = (n_bit & 0xFF) == 0 ? n_bit >> 3:(n_bit >>3)+1;
	self->bitarray = (char*) calloc(self->bitarray_size,sizeof(char));
    /*bitarray_t * ba = (bitarray_t*) malloc(sizeof(bitarray_t));
	ba->num_bits = n_bit;

	unsigned int n_int = (n_bit >> 6); //divide by 64 bits
	n_int += ((n_bit & 0x3F) > 0); //ceil quotient. If remainder > 0 then sum by 1

	ba->bitarray = (uint64_t *) calloc(n_int, sizeof(uint64_t));
	ba->bitarray_size = n_int;

	return ba;*/
}

void bitarray_setBit(bitarray_t *self, int index, int value)
{
	const int i1 = index >>3;
	const int i2 = index & 0x07;

	if (value)
		self->bitarray[i1] = self->bitarray[i1] | (1<<i2);
	else
		self->bitarray[i1] = self->bitarray[i1] & ~(1<<i2);
}

void bitarray_free(bitarray_t *self)
{
	free(self->bitarray);
}

int bitarray_getBit(bitarray_t *self, int index)
{
	const int i1 = index >>3;
	const int i2 = index & 0x07;

	return (self->bitarray[i1] & (1<<i2)) >>i2;

}






