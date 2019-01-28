
/*
** lab1.c
** ECE 4310 FALL 2018
** This program reads bridge.ppm, a 512 x 512 PPM image.
** It smooths it using a standard 3x3 mean filter.
** The program also demonstrates how to time a piece of code.
**
** To compile, must link using -lrt  (man clock_gettime() function).
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW 3
#define UNIT 1000000000.0
int main()
{
    FILE		*fpt;
    unsigned char	*image;
    
    unsigned char	*smoothed;
    unsigned char       *smoothed2;
    int                 *smoothed1;
    char		header[320];
    int		ROWS,COLS,BYTES;
    int		r,c,r2,c2,sum;
    struct timespec	tp1,tp2;

    /* read image */
    if ((fpt=fopen("bridge.ppm","rb")) == NULL) {
        printf("Unable to open bridge.ppm for reading\n");
        exit(0);
    }
    fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
    if (strcmp(header,"P5") != 0  ||  BYTES != 255) {
        printf("Not a greyscale 8-bit PPM image\n");
        exit(0);
    }
    //image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    header[0]=fgetc(fpt);	/* read white-space character that separates header */
    fread(image,1,COLS*ROWS,fpt);
    fclose(fpt);
    //1. Conv.
    /* allocate memory for smoothed version of image */
    //smoothed=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    smoothed=(unsigned char*)calloc(ROWS*COLS,sizeof(unsigned char));

    /* query timer */
    clock_gettime(CLOCK_REALTIME,&tp1);
    //printf("tp1: %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

    /* smooth image, skipping the border points */
    for (r=3; r<ROWS-3; r++)
        for (c=3; c<COLS-3; c++) {
            sum=0;
            for (r2=-3; r2<=3; r2++)
                for (c2=-3; c2<=3; c2++)
                    sum+=image[(r+r2)*COLS+(c+c2)];
            smoothed[r*COLS+c]=sum/49;
        }

    /* query timer */
    clock_gettime(CLOCK_REALTIME,&tp2);
    //printf("tp2: %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

    /* report how long it took to smooth */
    printf("conv %f ",(double)(tp2.tv_nsec-tp1.tv_nsec)/UNIT);

    /* write out smoothed image to see result */
    fpt=fopen("smoothed_conv.ppm","w");
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(smoothed,COLS*ROWS,1,fpt);
    fclose(fpt);
    free(smoothed);
    smoothed = NULL;

    //------------------------------------------------
    //2.Seperable filters
    /* allocate memory for smoothed version of image */
    smoothed1=(int *)calloc(ROWS*COLS,sizeof(int));
    smoothed2=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    
    /* query timer */
    clock_gettime(CLOCK_REALTIME,&tp1);
    //printf("tp1: %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

    /* smooth image, skipping the border points */
    //horizental.
    for (r=0; r<ROWS; r++)
        for (c=3; c<COLS-3; c++) {
            sum=0;
            for (c2=-3; c2<=3; c2++)
                sum+=(int)image[(c+c2)+r*COLS];
            smoothed1[(r)*COLS+c]=sum;
          }
    //vertical
    sum = 0;
    for (r=3; r<ROWS-3; r++)
        for (c=0; c<COLS; c++) {
            sum = 0;
            for (r2=-3; r2<=3; r2++)
                 sum+=(int)smoothed1[(r+r2)*COLS+c];
            smoothed2[(r)*COLS+c]=(unsigned char)(sum/49);
        }

    /* query timer */
    clock_gettime(CLOCK_REALTIME,&tp2);
    //printf("tp2: %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

    /* report how long it took to smooth */
    printf("sep %f ",(double)(tp2.tv_nsec-tp1.tv_nsec)/UNIT);

    /* write out smoothed image to see result */
    fpt=fopen("smoothed_sep.ppm","w");
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(smoothed2,COLS*ROWS,1,fpt);
    fclose(fpt);
    
    free(smoothed1);
    free(smoothed2);
    smoothed = NULL;

 //-----------------------------------------------------------
 //Sliding Window.
    /* allocate memory for smoothed version of image */
    smoothed1=(int *)calloc(ROWS*COLS,sizeof(int));
    smoothed2=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
   
    /* query timer */
    clock_gettime(CLOCK_REALTIME,&tp1);

    //printf("tp1: %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

    /* smooth image, skipping the border points */
    //horizental.
    for (r=0; r<ROWS; r++){
        sum = 0;
        for (c=3; c<COLS-3; c++) {

            for (c2=-3; c2<=3; c2++){
                if (c == 3){
                    sum+=(int)image[(c+c2)+r*COLS];
                //    printf("yes. sum %d\n",sum);
                }else{
                    sum = sum - (int)image[(c2+c-1)+r*COLS] + (int)image[(c+c2+6)+r*COLS];
                 //   printf("not. sum %d, Old %d, new %d\n",sum, image[(c2+c-1)+r*COLS],image[(c+c2+6)+r*COLS]);

                //    exit(1);
                    break;
                }

            }
            smoothed1[(r)*COLS+c]=sum;
          }
     }
    //vertical
    sum = 0;

    for (c=0; c<COLS; c++){
        sum = 0;
        for (r=3; r<ROWS-3; r++) {
            for (r2=-3; r2<=3; r2++){
                if(r==3){
                    sum+=smoothed1[(r+r2)*COLS+c];
                }else{
                    sum = sum - smoothed1[(r+r2-1)*COLS+c] + smoothed1[(r+r2+6)*COLS+c];
                    //continue;
                    break;
                }

            }
            smoothed2[(r)*COLS+c]=(unsigned char)(sum/49);
        }
    }
    /* query timer */
    clock_gettime(CLOCK_REALTIME,&tp2);
   // printf("tp2: %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

    /* report how long it took to smooth */
    printf("sw %f\n",(double)(tp2.tv_nsec-tp1.tv_nsec)/UNIT);

    /* write out smoothed image to see result */
   // printf("sliding window. completed.\n");
    fpt=fopen("smoothed_sw.ppm","w");
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(smoothed2,COLS*ROWS,1,fpt);
    fclose(fpt);
    free(smoothed1);
    free(smoothed2);
    smoothed = NULL;

    free(image);
    image = NULL;


}
