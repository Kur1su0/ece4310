#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "function.h"

#define PATH "src/chair-range.ppm"

/* Custom args */
//#define THRE 130
//#define THRE 102
int DISTANCE = 4;
int ANGLE = 20;
int THRE = 130;
int isCLEAR = 1;

int _COLS = 128, _ROWS = 128, _BYTES = 0;
char _header[320];
char image_path[160];
char out_name[100];

int main(int argc, char *argv[]){
    //printf("start operation\n");
    THRE     =atoi(argv[1]);
    DISTANCE =atoi(argv[2]);
    ANGLE    =atoi(argv[3]);
    isCLEAR  =atoi(argv[4]);
    printf("THRE\t%d\t",THRE     );
    printf("DISTANCE\t%d\t",DISTANCE );
    printf("ANGLE\t%d\t",ANGLE    );
    printf("isCLEAR\t%d\n",isCLEAR   );

    unsigned char* range_image = NULL;
    unsigned char* labels = NULL;
    labels = (unsigned char*)calloc(_COLS*_ROWS, sizeof(unsigned char));
    range_image = read_image(range_image, PATH);


    for(int i=0;i<128*128;i++){
        //labels[i] =  range_image[i]>=THRE?0:255;
        if(range_image[i] < THRE) labels[i] =  255;
        range_image[i]   =  range_image[i]>=THRE?0:range_image[i];

    }
    sprintf(out_name,"src/backgroundRM_%d_%d_%d_%d.ppm",THRE,DISTANCE,ANGLE,isCLEAR);
    save_image(range_image, NULL, out_name);
    sprintf(out_name,"src/label_%d_%d_%d_%d.ppm",THRE,DISTANCE,ANGLE,isCLEAR);
    save_image(labels, NULL, out_name);
    double P[3][128*128];
    double cross[3][128*128];
    convert_to_coord( P, range_image );
    //for(int i =0 ; i< 128*128; i++){
    //    printf("%f\%f\%f\n",P[0][i],P[1][i],P[2][i]);
   // }
    //exit(0);

    /* Cross Product:
     * Cx = AyBz - AzBy;
     * Cy = AzBx - AxBz;
     * Cz = AxBy - AyBx;
     */

    cross_product(P, cross);
    //debug print function.
    int debug = 0;
    if (debug==1){
        for(int i=0;i<128*128;i++){
            printf("%f\t",cross[0][i]);
        }
        printf("\n");
        for(int i=0;i<128*128;i++){
            printf("%f\t",cross[1][i]);
        }
        printf("\n");
        for(int i=0;i<128*128;i++){
            printf("%f\t",cross[2][i]);
         }
        printf("\n");
        exit(0);
    }

    cal_region_grow(cross, labels);




}
