#include <stdio.h>
#include <ff.h>

typedef struct {
	unsigned int num_bits;
	unsigned int bitarray_size;
	char * bitarray;
} bitarray_t;

unsigned char * read_label_idx(TCHAR * filename, unsigned int * n);
unsigned char *** read_data_idx(TCHAR * filename, unsigned int * n, unsigned int * rows, unsigned int * cols);

//void binarize_image(unsigned char ** image, unsigned char ** bin_image, unsigned int rows, unsigned int cols);
//void binarize_image_array(unsigned char ** image, unsigned char * bin_image, unsigned int rows, unsigned int cols);
void binarize_image_bitarray(unsigned char ** image, bitarray_t * bin_image, unsigned int rows, unsigned int cols);
void bitarray_new(bitarray_t *self,unsigned int n_bit);
void bitarray_setBit(bitarray_t *self, int index, int value);
void bitarray_free(bitarray_t *self);
int bitarray_getBit(bitarray_t *self, int index);


