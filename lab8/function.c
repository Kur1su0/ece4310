
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "function.h"


#define ROWS	128
#define COLS	128
#define SQR(x) ((x)*(x))
#define WINDOW 5
//#define ANGLE MI_PI/4
//#define ANGLE 25

extern int _ROWS;
extern int _COLS;
extern int _BYTES;
extern char _header[320];

extern int DISTANCE;
extern int ANGLE;
extern int THRE;
extern int isCLEAR;

double average[3];


//Private func
double magnitude(double v[3]);
double angle(double a[3], double b[3]);



/*Read image from the .ppm and save in a array.
 *
 */
unsigned char* read_image(unsigned char* image, char image_path[100])
{
    FILE *fpt;
    if ((fpt=fopen(image_path,"rb")) == NULL) {
        printf("Unable to open text.ppm for reading\n");
        exit(0);
    }
    fscanf(fpt,"%s %d %d %d\n",_header,&_COLS,&_ROWS,&_BYTES);
    if (strcmp(_header,"P5") != 0  ||  _BYTES != 255) {
        printf("Not a greyscale 8-bit PPM image\n");
        exit(0);
    }

    image = (unsigned char *)calloc(_ROWS*_COLS,sizeof(unsigned char));

    fread(image,1,_COLS*_ROWS,fpt);
    fclose(fpt);
    fpt = NULL;

    return image;

}

/*
 * Save image to the out.ppm
 */
void save_image(unsigned char* image_chr,float* image_int, char name[300])
{

    FILE* pFile;
    pFile = fopen(name,"w");
    fprintf(pFile,"P5 %d %d 255\n",COLS,ROWS);
    assert((image_chr==NULL&&image_int!=NULL) || (image_chr!=NULL&&image_int==NULL));
    if(image_chr == NULL) {
        image_chr = (unsigned char*)calloc(ROWS*COLS,sizeof(unsigned char));
        for(int i=0; i<ROWS*COLS; i++) image_chr[i] = (unsigned char)image_int[i];
        fwrite((unsigned char*)image_chr,COLS*ROWS,1,pFile);
        free(image_chr);
    }

    else {
        fwrite((unsigned char*)image_chr,COLS*ROWS,1,pFile);
    }

    fclose(pFile);

}


/* cross product:
 * cx = aybz - azby;
 * cy = azbx - axbz;
 * cz = axby - aybx;
 */

void cross_product(double p[3][128*128], double cross[3][128*128])
{
    int r = 0, c = 0;
    for(r = 0; r < _ROWS; r++) {
        for(c = 0; c < _COLS; c++) {
            if(r >= (_ROWS - DISTANCE) || (c >= _COLS - DISTANCE) ) {
                cross[0][r*_COLS+c] = 0.0;
                cross[1][r*_COLS+c] = 0.0;
                cross[2][r*_COLS+c] = 0.0;
            } else {
                //TODO:Cal Cross product.
                /* cross product:
                 * cx = aybz - azby;
                 * cy = azbx - axbz;
                 * cz = axby - aybx;
                 */
                //set up a (down by Distance) and b (right by distance)
                double a[3],b[3];
                a[0] = p[0][r*_COLS + c + DISTANCE] - p[0][r*_COLS+c];
                a[1] = p[1][r*_COLS + c + DISTANCE] - p[1][r*_COLS+c];
                a[2] = p[2][r*_COLS + c + DISTANCE] - p[2][r*_COLS+c];

                b[0] = p[0][r*_COLS + c + DISTANCE*_COLS] - p[0][r*_COLS+c];
                b[1] = p[1][r*_COLS + c + DISTANCE*_COLS] - p[1][r*_COLS+c];
                b[2] = p[2][r*_COLS + c + DISTANCE*_COLS] - p[2][r*_COLS+c];
                // 0 : x
                // 1 : y
                // 2 : z
                cross[0][r*_COLS+c] = a[1] * b[2] - a[2] * b[1];
                cross[1][r*_COLS+c] = a[2] * b[0] - a[0] * b[2];
                cross[2][r*_COLS+c] = a[0] * b[1] - a[1] * b[0];

            }

        }

    }


}


/*
**	This routine converts the data in an Odetics range image into 3D
**	cartesian coordinate data.  The range image is 8-bit, and comes
**	already separated from the intensity image.
*/

void convert_to_coord( double P[3][128*128], unsigned char* RangeImage)
{

    int	r,c;
    double	cp[7];
    double	xangle,yangle,dist;
    double	ScanDirectionFlag,SlantCorrection;
    //unsigned char	RangeImage[128*128];
    //double		P[3][128*128];
    int             ImageTypeFlag;
    //char	Filename[160];
    char Outfile[160];
    FILE	*fpt;
    /*
    printf("Enter range image file name:");
    scanf("%s",Filename);
    if ((fpt=fopen(Filename,"r")) == NULL) {
        printf("Couldn't open %s\n",Filename);
        exit(1);
    }
    fread(RangeImage,1,128*128,fpt);
    fclose(fpt);
    */
    //printf("Up(-1), Down(1) or Neither(0)? ");
    //scanf("%d",&ImageTypeFlag);
    //printf("you input: %d\n",ImageTypeFlag);

    cp[0]=1220.7;		/* horizontal mirror angular velocity in rpm */
    cp[1]=32.0;		/* scan time per single pixel in microseconds */
    cp[2]=(COLS/2)-0.5;		/* middle value of columns */
    cp[3]=1220.7/192.0;	/* vertical mirror angular velocity in rpm */
    cp[4]=6.14;		/* scan time (with retrace) per line in milliseconds */
    cp[5]=(ROWS/2)-0.5;		/* middle value of rows */
    cp[6]=10.0;		/* standoff distance in range units (3.66cm per r.u.) */

    cp[0]=cp[0]*3.1415927/30.0;	/* convert rpm to rad/sec */
    cp[3]=cp[3]*3.1415927/30.0;	/* convert rpm to rad/sec */
    cp[0]=2.0*cp[0];		/* beam ang. vel. is twice mirror ang. vel. */
    cp[3]=2.0*cp[3];		/* beam ang. vel. is twice mirror ang. vel. */
    cp[1]/=1000000.0;		/* units are microseconds : 10^-6 */
    cp[4]/=1000.0;			/* units are milliseconds : 10^-3 */

    ScanDirectionFlag=1;

    /* start with semi-spherical coordinates from laser-range-finder: */
    /*			(r,c,RangeImage[r*COLS+c])		  */
    /* convert those to axis-independant spherical coordinates:	  */
    /*			(xangle,yangle,dist)			  */
    /* then convert the spherical coordinates to cartesian:           */
    /*			(P => X[] Y[] Z[])			  */

    if (ImageTypeFlag != 3) {
        for (r=0; r<ROWS; r++) {
            for (c=0; c<COLS; c++) {
                if(RangeImage[r*COLS+c]==0){
                    P[0][r*COLS+c]=0;
                    P[1][r*COLS+c]=0;
                    P[2][r*COLS+c]=0;

                }else{
                    SlantCorrection=cp[3]*cp[1]*((double)c-cp[2]);
                    xangle=cp[0]*cp[1]*((double)c-cp[2]);
                    yangle=(cp[3]*cp[4]*(cp[5]-(double)r))+	/* Standard Transform Part */
                           SlantCorrection*ScanDirectionFlag;	/*  + slant correction */
                    dist=(double)RangeImage[r*COLS+c]+cp[6];
                    P[2][r*COLS+c]=sqrt((dist*dist)/(1.0+(tan(xangle)*tan(xangle))
                                                     +(tan(yangle)*tan(yangle))));
                    P[0][r*COLS+c]=tan(xangle)*P[2][r*COLS+c];
                    P[1][r*COLS+c]=tan(yangle)*P[2][r*COLS+c];
                }
            }
        }
    }

    sprintf(Outfile,"%s.coords","chair-range.ppm");
    fpt=fopen(Outfile,"w");
    fwrite(P[0],8,128*128,fpt);
    fwrite(P[1],8,128*128,fpt);
    fwrite(P[2],8,128*128,fpt);
    fclose(fpt);


}




void cal_region_grow(double cross[3][128*128], unsigned char* labels) {
    int		r,c;//,r2,c2;
    int		*indices,i;
    int		RegionSize,*RegionPixels,TotalRegions;
    //double		avg,var;
    FILE* fpt = NULL;
    int seed = 0;
    //labels=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    /* used to quickly erase small grown regions */
    indices=(int *)calloc(_ROWS*_COLS,sizeof(int));
    unsigned char* copy = (unsigned char*)calloc(128*128, sizeof(unsigned char));
    for(int i =0; i < 128*128; i++) copy[i] = labels[i];
    TotalRegions=0;
    for (r=WINDOW/2; r<ROWS-WINDOW/2; r++) {
        for (c=WINDOW/2; c<COLS-WINDOW/2; c++) {
                seed = 1;

               for(int r2=-WINDOW/2; r2<=WINDOW/2;r2++){
                    for(int c2=-WINDOW/2; c2<=WINDOW/2;c2++){
                        if(labels[(r+r2)*_COLS+(c+c2)]!=255){
                            seed = 0;
                        }

                    }

                }

                if(seed ==1){
                TotalRegions++;

                //TotalRegions+=50;
                if (TotalRegions >= 255) {
                    printf("Segmentation incomplete.  Ran out of labels.\n");
                    break;
                }
                RegionGrow(cross,labels,r,c,255,255-TotalRegions*25,
                           indices,&RegionSize, average);

                if (RegionSize < 13) {  //FIXME:how?
                    // erase region (relabel pixels back to 0)
                    for (i=0; i<RegionSize; i++)
                        if(copy[indices[i]] == 255) labels[indices[i]] = 255;
                    TotalRegions--;
                } else{
                    printf("Region:\t%d(%d)\tsize:\t%4d\t",TotalRegions,255-TotalRegions*25, RegionSize);
                    printf("average: %f\t%f\t%f\n",average[0],average[1],average[2]);
                  }
         }

        }
        //if (c < COLS-WINDOW/2)
        //    break;	/* ran out of labels -- break both loops */
    }
    if(isCLEAR==1){
         for(int i=0; i<128*128;i++) labels[i] = labels[i]==255?0:labels[i];
    }
    printf("%d total regions were found\n",TotalRegions);
    char out_name[100];
    sprintf(out_name,"src/seg_%d_%d_%d_%d.ppm",THRE,DISTANCE,ANGLE,isCLEAR);

    if ((fpt=fopen(out_name,"wb")) == NULL) {
        printf("Unable to open file for writing\n");
        exit(0);
    }
    fprintf(fpt,"P5 %d %d 255\n",_COLS,_ROWS);
    fwrite(labels,1,_ROWS*_COLS,fpt);
    fclose(fpt);
}


/*
** Given an image, a starting point, and a label, this routine
** paint-fills (8-connected) the area with the given new label
** according to the given criteria (pixels close to the average
** intensity of the growing region are allowed to join).
*/

#define MAX_QUEUE 10000	/* max perimeter size (pixels) of border wavefront */

void RegionGrow(double cross[0][128*128],	/* image data */
                unsigned char *labels,	/* segmentation labels */
                //int ROWS,int COLS,	/* size of image */
                int r,int c,		/* pixel to paint from */
                int paint_over_label,	/* image label to paint over */
                int new_label,		/* image label for painting */
                int *indices,		/* output:  indices of pixels painted */
                int *count, double average[3])		/* output:  count of pixels painted */
{
    int	r2,c2;
    int	queue[MAX_QUEUE],qh,qt;
 //   double average[3];
    double total[3];	/* average and total intensity in growing region */
    int total_r=0,total_c=0,centroid_r=0,centroid_c=0,d_r=0,d_c=0;
    int p_r=0,p_c=0,distance=0;
    int cal_avg_flag = 0;
    *count=0;
    if (labels[r*COLS+c] != paint_over_label)
        return;
    labels[r*COLS+c]=new_label;
    average[0]=total[0]=cross[0][r*COLS+c];
    average[1]=total[1]=cross[1][r*COLS+c];
    average[2]=total[2]=cross[2][r*COLS+c];

    centroid_r = r;
    centroid_c = c;
    total_r = r;
    total_c = c;

    if (indices != NULL)
        indices[0]=r*COLS+c;
    queue[0]=r*COLS+c;
    qh=1;	/* queue head */
    qt=0;	/* queue tail */
    (*count)=1;
    while (qt != qh) {
        for (r2=-1; r2<=1; r2++)
            for (c2=-1; c2<=1; c2++) {
                double vector[3];
                if (r2 == 0  &&  c2 == 0)
                    continue;
                if ((queue[qt]/COLS+r2) < 0  ||  (queue[qt]/COLS+r2) >= ROWS  ||
                        (queue[qt]%COLS+c2) < 0  ||  (queue[qt]%COLS+c2) >= COLS)
                    continue;
                if (labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=paint_over_label) //XXX: chair-255
                    continue;
                /* test criteria to join region */

                vector[0] = cross[0][(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];
                vector[1] = cross[1][(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];
                vector[2] = cross[2][(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];

                double res = angle(average, vector)*(180.0/3.1415);
                if(res > ANGLE){
                    cal_avg_flag = 0;
                    labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2] = 0;
                    continue;
                }
                //printf("ANGLE: %f\n", res);
                labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]=new_label;
                if (indices != NULL)
                    indices[*count]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
                //total+=image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];

                total_r+=queue[qt]/COLS+r2;
                total_c+=queue[qt]%COLS+c2;
                (*count)++;
                cal_avg_flag = 1;

                average[0] = (average[0] + vector[0])/2.0;
                average[1] = (average[1] + vector[1])/2.0;
                average[2] = (average[2] + vector[2])/2.0;

                //printf("%f %f %f\n",vector[0],vector[1],vector[2]);
                //printf("%f %f %f\n",average[0],average[1],average[2]);

                queue[qh]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
                qh=(qh+1)%MAX_QUEUE;
                if (qh == qt) {
                    printf("Max queue size exceeded\n");
                    exit(0);
                }
            }
        qt=(qt+1)%MAX_QUEUE;
    }
    //printf("average: %-12f%-12f%-12f\n",average[0],average[1],average[2]);
}

double magnitude(double v[3]){
    return sqrt( SQR(v[0]) + SQR(v[1]) + SQR(v[2]) );

}


double angle(double a[3], double b[3]){

    double Angle, dot_product, mag_product;
    //Vector * Vector
    dot_product = (a[0]*b[0] + a[1]*b[1] + a[2]*b[2] );
    //magnitude
    mag_product = magnitude(a) * magnitude(b);
    // angle
    Angle = acos(dot_product / mag_product);

    return Angle;
}
