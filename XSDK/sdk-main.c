#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xil_printf.h"
#include <xwisard.h>
#include <string.h>
#include <xil_cache.h>
#include "mnist_operations.h"
#include <ff.h>
#include <xtime_l.h>

#define NUM_OF_RAMS 28
#define TUPLE_SIZE 28

XWisard_Config* xw_cfg;
XWisard xw;
static DIR dir;
static FATFS fatfs;
XTime tStart,tEnd,tws,twe;


void map_array(int * tuple_map, bitarray_t * data, int * map)
{
    int i, j, k = 0;
    int addr_pos=0;
    int addr;

	for (i = 0; i < NUM_OF_RAMS; i++)
	{
		addr = 0;

		for (j = 0; j < TUPLE_SIZE; j++)
		{
			addr = (addr<<1) | bitarray_getBit(data, tuple_map[addr_pos++]);
		}

		map[i]=addr;
		//printf("Map:%d, Addr:%llu\n",map[i],addr);
	}
}


int main()
{
	int i,j;
	FRESULT Res;
	TCHAR * Path="0:/"; //for mounting
	TCHAR * train_label_filename = "0:/MNIST/TRAINL";
	TCHAR * train_data_filename = "0:/MNIST/TRAINI";
	TCHAR * test_label_filename = "0:/MNIST/T10KL";
	TCHAR * test_data_filename = "0:/MNIST/T10KI";
	long long int counter=0;

	unsigned int n_train_label, n_train_data, n_test_label, n_test_data;
	unsigned int rows_train, cols_train, rows_test, cols_test;
	int mode=0;

	/**********************Initialization********************************/
	init_platform();
	XTime_GetTime(&tStart);
	xil_printf("Initializing ...\n");

	xw_cfg = XWisard_LookupConfig(0);
	if(xw_cfg==NULL)
	{
		xil_printf("Error Wisard LookupConfig\n");
		return XST_FAILURE;
	}
	if(XWisard_CfgInitialize(&xw,xw_cfg) != XST_SUCCESS)
	{
		xil_printf("Error Wisard Initialization\n");
		return XST_FAILURE;
	}

	XWisard_DisableAutoRestart(&xw);
	XWisard_InterruptGlobalDisable(&xw);
	Xil_DCacheDisable();

	//mounting SDcard
	Res = f_mount(&fatfs, Path, 0);
	if (Res != FR_OK) {
		xil_printf("Error mounting SD Card\n\r");
		return XST_FAILURE;
	}

	Res = f_opendir(&dir, "0:/MNIST");
	if (Res) {
		xil_printf("Error opening MNIST dir\n\r");
		return XST_FAILURE;
	}

	/*Res = f_readdir(&dir, &finfo);
	while (strcmp(finfo.fname, "\0") != 0)
	{
		xil_printf("%s\n",finfo.fname);
		f_readdir(&dir, &finfo);
	}*/
	/****************Ended Initialization********************************/

	//Read mnist dataset files
	xil_printf("Reading Files...\n");
	unsigned char * train_labels = read_label_idx(train_label_filename, &n_train_label);
	unsigned char *** train_data = read_data_idx(train_data_filename, &n_train_data, &rows_train, &cols_train);
	unsigned char * test_labels = read_label_idx(test_label_filename, &n_test_label);
	unsigned char *** test_data = read_data_idx(test_data_filename, &n_test_data, &rows_test, &cols_test);

	//Initialize aux matrix to store binarized images
	int map[NUM_OF_RAMS];
    int entry_size = rows_train * cols_train; //784 for MNIST
	bitarray_t bin_data;
	bitarray_new(&bin_data,entry_size);
	int tuple_map[784] = { 598 , 470 , 123 , 694 , 320 , 395 , 684 , 772 , 534 , 54 , 542 , 384 , 295 , 454 , 161 , 512 , 131 , 697 , 228 , 428 , 740 , 647 , 781 , 567 , 453 , 67 , 334 , 463 , 82 , 717 , 661 , 669 , 378 , 558 , 500 , 390 , 303 , 457 , 735 , 730 , 237 , 307 , 399 , 529 , 731 , 493 , 641 , 562 , 637 , 505 , 706 , 700 , 667 , 593 , 302 , 282 , 84 , 69 , 679 , 646 , 533 , 711 , 29 , 329 , 642 , 712 , 655 , 235 , 707 , 704 , 154 , 342 , 128 , 124 , 404 , 143 , 589 , 770 , 118 , 208 , 549 , 398 , 117 , 622 , 331 , 708 , 745 , 373 , 155 , 210 , 104 , 736 , 561 , 201 , 506 , 678 , 301 , 318 , 204 , 170 , 311 , 300 , 213 , 682 , 743 , 12 , 403 , 762 , 332 , 257 , 732 , 643 , 256 , 379 , 401 , 374 , 421 , 741 , 426 , 739 , 18 , 249 , 207 , 715 , 415 , 220 , 758 , 701 , 422 , 614 , 386 , 668 , 518 , 286 , 368 , 27 , 675 , 630 , 58 , 101 , 748 , 387 , 414 , 234 , 383 , 260 , 50 , 156 , 617 , 92 , 603 , 699 , 609 , 75 , 183 , 574 , 339 , 160 , 179 , 397 , 193 , 169 , 615 , 23 , 531 , 80 , 423 , 53 , 22 , 333 , 7 , 186 , 308 , 71 , 5 , 323 , 166 , 703 , 716 , 236 , 35 , 361 , 709 , 599 , 122 , 782 , 420 , 469 , 724 , 324 , 654 , 640 , 247 , 528 , 502 , 757 , 445 , 526 , 153 , 588 , 107 , 718 , 164 , 78 , 129 , 546 , 478 , 413 , 259 , 436 , 258 , 755 , 284 , 705 , 775 , 377 , 304 , 288 , 406 , 651 , 62 , 37 , 728 , 230 , 560 , 635 , 134 , 372 , 290 , 517 , 349 , 148 , 540 , 492 , 721 , 431 , 507 , 656 , 508 , 356 , 119 , 763 , 77 , 590 , 765 , 72 , 516 , 211 , 634 , 587 , 698 , 255 , 109 , 187 , 366 , 660 , 359 , 348 , 447 , 242 , 167 , 266 , 163 , 190 , 409 , 3 , 39 , 93 , 250 , 231 , 243 , 199 , 149 , 565 , 479 , 36 , 459 , 60 , 530 , 722 , 341 , 171 , 480 , 408 , 465 , 317 , 215 , 391 , 192 , 281 , 710 , 312 , 151 , 367 , 212 , 9 , 491 , 485 , 335 , 25 , 683 , 162 , 313 , 631 , 535 , 217 , 623 , 747 , 276 , 575 , 139 , 419 , 494 , 173 , 203 , 269 , 725 , 389 , 498 , 252 , 172 , 2 , 515 , 466 , 452 , 695 , 152 , 274 , 216 , 267 , 776 , 435 , 430 , 375 , 543 , 293 , 106 , 369 , 473 , 396 , 52 , 233 , 569 , 177 , 316 , 760 , 489 , 214 , 461 , 26 , 330 , 229 , 670 , 472 , 664 , 63 , 363 , 0 , 382 , 713 , 200 , 536 , 40 , 319 , 168 , 681 , 602 , 753 , 425 , 496 , 388 , 141 , 31 , 686 , 298 , 738 , 362 , 4 , 181 , 719 , 596 , 297 , 585 , 746 , 188 , 449 , 696 , 275 , 11 , 365 , 191 , 30 , 527 , 10 , 345 , 601 , 325 , 538 , 524 , 251 , 381 , 326 , 175 , 76 , 240 , 271 , 737 , 476 , 370 , 571 , 688 , 46 , 557 , 380 , 769 , 402 , 628 , 262 , 223 , 353 , 600 , 338 , 336 , 510 , 539 , 443 , 85 , 548 , 147 , 44 , 270 , 238 , 1 , 176 , 677 , 504 , 32 , 322 , 350 , 289 , 189 , 299 , 114 , 272 , 579 , 16 , 636 , 756 , 581 , 687 , 503 , 633 , 222 , 159 , 462 , 433 , 689 , 279 , 429 , 752 , 8 , 239 , 90 , 606 , 15 , 49 , 41 , 653 , 663 , 112 , 86 , 263 , 482 , 692 , 347 , 246 , 774 , 434 , 98 , 268 , 649 , 612 , 658 , 103 , 583 , 194 , 734 , 744 , 412 , 541 , 226 , 639 , 55 , 554 , 13 , 442 , 65 , 265 , 102 , 607 , 411 , 59 , 467 , 45 , 509 , 723 , 100 , 456 , 750 , 38 , 287 , 110 , 652 , 573 , 385 , 672 , 174 , 550 , 182 , 519 , 245 , 248 , 294 , 547 , 136 , 532 , 24 , 68 , 61 , 43 , 553 , 432 , 95 , 253 , 424 , 691 , 597 , 659 , 221 , 778 , 487 , 144 , 64 , 254 , 280 , 142 , 371 , 471 , 283 , 89 , 79 , 206 , 605 , 88 , 132 , 232 , 458 , 592 , 180 , 407 , 241 , 94 , 727 , 742 , 749 , 364 , 484 , 48 , 671 , 477 , 440 , 354 , 97 , 650 , 178 , 202 , 702 , 83 , 568 , 416 , 662 , 620 , 296 , 648 , 244 , 564 , 140 , 357 , 427 , 644 , 497 , 51 , 392 , 111 , 96 , 754 , 99 , 486 , 309 , 520 , 676 , 511 , 555 , 438 , 158 , 584 , 264 , 355 , 610 , 133 , 105 , 613 , 28 , 563 , 360 , 468 , 490 , 674 , 225 , 495 , 195 , 273 , 145 , 780 , 552 , 448 , 751 , 576 , 328 , 513 , 196 , 771 , 57 , 759 , 310 , 566 , 481 , 594 , 21 , 113 , 358 , 657 , 572 , 394 , 544 , 638 , 376 , 291 , 6 , 629 , 352 , 618 , 578 , 522 , 165 , 314 , 766 , 305 , 783 , 91 , 444 , 125 , 198 , 418 , 450 , 523 , 146 , 545 , 537 , 455 , 665 , 261 , 577 , 137 , 720 , 595 , 437 , 219 , 608 , 666 , 346 , 767 , 127 , 624 , 439 , 340 , 733 , 777 , 277 , 343 , 115 , 621 , 582 , 604 , 464 , 47 , 475 , 56 , 87 , 121 , 514 , 611 , 680 , 693 , 321 , 779 , 499 , 501 , 570 , 764 , 197 , 559 , 400 , 74 , 344 , 227 , 488 , 351 , 405 , 627 , 690 , 625 , 616 , 645 , 116 , 218 , 474 , 451 , 184 , 580 , 551 , 441 , 460 , 42 , 685 , 278 , 209 , 157 , 525 , 20 , 521 , 66 , 586 , 285 , 410 , 205 , 327 , 315 , 773 , 768 , 726 , 135 , 185 , 337 , 130 , 17 , 34 , 81 , 673 , 70 , 108 , 626 , 138 , 761 , 19 , 483 , 306 , 632 , 224 , 393 , 14 , 73 , 120 , 150 , 446 , 729 , 556 , 619 , 591 , 33 , 292 , 714 , 417 , 126};

	//training
	xil_printf("Training the set ...\n");
	mode=0;
    for (i = 0; i < n_train_label; i++)
    {
        binarize_image_bitarray(train_data[i], &bin_data, rows_train, cols_train);
        map_array(tuple_map,&bin_data,map);
        //wisard_train(wisard, bin_data, train_labels[i]);

        XTime_GetTime(&tws);
    	XWisard_Set_mode(&xw,mode);
    	XWisard_Set_disc_id(&xw, train_labels[i]);
    	XWisard_Set_input_addr(&xw,&map[0]);

    	XWisard_Start(&xw);
    	while(!XWisard_IsDone(&xw));
    	XTime_GetTime(&twe);

    	counter+=2*(twe - tws);
    	//xil_printf("Best_sum:%d\n",XWisard_Get_res_cc(&xw));
    }
    printf("Train Clock Cycles: %llu\n", counter);
    counter=0;
    //classifying
    int label, count_correct_label = 0;
	xil_printf("Classifying images...\n");
	mode=1;
	for (i = 0; i < n_test_label; i++) {
		binarize_image_bitarray(test_data[i], &bin_data, rows_test, cols_test);
		map_array(tuple_map,&bin_data,map);

		XTime_GetTime(&tws);
    	XWisard_Set_mode(&xw,mode);
    	//XWisard_Set_disc_id(&xw, train_labels[i]); //XX
    	XWisard_Set_input_addr(&xw, &map[0]);

    	XWisard_Start(&xw);
    	while(!XWisard_IsDone(&xw));


    	label = XWisard_Get_res(&xw); //response
    	XTime_GetTime(&twe);
    	counter+=2*(twe - tws);
       //	xil_printf("Best_sum:%d Label:%d\n",XWisard_Get_res_cc(&xw),XWisard_Get_res(&xw));
		if (label == test_labels[i]) {
			count_correct_label++;
		}
	}
	printf("Classify Clock Cycles: %llu\n", counter);

    //Calculate accuracy
    float acc = ((float)count_correct_label)/((float)n_test_label);
    printf("Total = %d, Correct predicted images = %d, Accuracy = %f\n", n_test_label, count_correct_label, acc);

    //Freeing Memory
    free(train_labels);
    free(test_labels);

    for (i = 0; i < n_train_data; i++) {
        for (j = 0; j < rows_train; j++) {
            free(train_data[i][j]);
        }
        free(train_data[i]);
    }
    free(train_data);

    for (i = 0; i < n_test_data; i++) {
        for (j = 0; j < rows_test; j++) {
            free(test_data[i][j]);
        }
        free(test_data[i]);
    }
    free(test_data);

    bitarray_free(&bin_data);

    XTime_GetTime(&tEnd);

    printf("Total Clock Cycles: %llu\n", 2*(tEnd - tStart));

    xil_printf("-The End-\n");

    cleanup_platform();
    return XST_SUCCESS;
}
