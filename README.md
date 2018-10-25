# HLS Wisard with Dictionary

This contains a simple HLS Wisard implementation with the use of dictionary to decrease memory consumption.
If a collision is detect it searches for the next position below. 
It uses a H3 matrix as it's hash function. This function is randomly generated through the H3_Matrix_Generator.py file. The value is then hardcoded to the HLS version.

It uses Axi-Full and Axi-Lite for transfer. The testbench file(Outdated) should be used with Vivado HLS.

The folder "xSDK" contains the code to be executed on the ARM microprocessor with the MNIST Database. The hardcoded file names are based on the "MNIST" folder.
The only change made to the ldscript is the increase in heap_size.
The "MNIST" folder should be copied to the SD Card.

The folder "Without Dictionary" consists of a Wisard version in HLS without the use of a dictionary. ( Precursor to the dictionary version )


