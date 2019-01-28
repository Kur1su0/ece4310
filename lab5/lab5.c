#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#define SQR(x) ((x)*(x))
#define txt "ini.txt"
#define image_path "hawk.ppm"
#define out_image_path "hawk_out.ppm"
#define sobel_image_path "hawk_out_sobel.ppm"

#define BLACK 0
#define WHITE 255

#define TRUE 1
#define FALSE 0
typedef int bool;
// Struct design
struct _pixel{
    int x;
    int y;
};
typedef struct _pixel PIXEL;
typedef char* FILE_PATH;
//prototype
unsigned char* read_image(unsigned char*);
void save_image(unsigned char*,float*, char* );
unsigned char* show_crosshair(unsigned char*, PIXEL, int);
PIXEL* active_contour(unsigned char*, PIXEL*);
float* sobel(unsigned char*);
float* conv(unsigned char* a, float* b, int array[]);
//1st int* for larger image, 2nd int* for smaller image(7*7) 
float* normalized(float*,float*, int, int);
// ENERGY
float* clear_energy_buffer(float*);
float contour_avg_dis(PIXEL*);
float InternalEnergy1(PIXEL,PIXEL,int,int);
float InternalEnergy2(float, PIXEL,PIXEL,int,int);
float ExternalEnergy(PIXEL, float *, int, int);    
//gloal val.
int COLS = 0, ROWS = 0, BYTES = 0;
int line=0;
bool show_process_flag = TRUE;
bool show_final_contour = TRUE;
char header[320];


int main(){
    PIXEL* pixel= NULL;
    PIXEL* itered_pixel= NULL;
    unsigned char* image = NULL;
    unsigned char* ini_image = NULL;
    unsigned char* out_image = NULL;
    int ch=0;
    FILE *fp = fopen(txt,"r");

    image = read_image(image);
    ini_image = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    out_image = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

    if (fp==NULL){
        printf("txt not found");
        exit(1);
    }
    //find the total #lines.
    while(!feof(fp)){
        ch = fgetc(fp);
        if (ch == '\n')
            line++;
    }
    fseek(fp, 0, SEEK_SET);
    pixel = (PIXEL*)calloc(line,sizeof(PIXEL));
    int i = 0;

    //get ini contour info.
    while(!feof(fp)){
        fscanf(fp, "%d %d\n",&pixel[i].x, &pixel[i].y);
        i++;
    }

   for(i=0;i<ROWS*COLS;i++){
       out_image[i] = image[i];
       ini_image[i] = image[i];
   }
   for (i=0;i<line;i++){
       ini_image = show_crosshair(ini_image, pixel[i],BLACK);
   }
   //save init contour.
   save_image(ini_image,NULL,"init.ppm");

  //contour manipulation,
  itered_pixel = active_contour(image, pixel);

  //show crosshair
   for (i=0;i<line;i++){
       out_image = show_crosshair(out_image, itered_pixel[i],BLACK);
   }
  save_image(out_image,NULL,"itered.ppm");
  
  
  
  
  save_image(NULL,sobel(out_image),"sobel_final.ppm");
  
  if(show_final_contour){
      for (i=0;i<line;i++) printf("%d %d\n",itered_pixel[i].y,itered_pixel[i].x);
  }
 free(ini_image);
 free(out_image);
 return 0;
}




/*Read image from the .ppm and save in a array.
 *
 */
unsigned char* read_image(unsigned char* image){
    FILE *fpt;
    if ((fpt=fopen(image_path,"rb")) == NULL) {
        printf("Unable to open text.ppm for reading\n");
        exit(0);
    }
    fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
    if (strcmp(header,"P5") != 0  ||  BYTES != 255) {
        printf("Not a greyscale 8-bit PPM image\n");
        exit(0);
    }

    image = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

    fread(image,1,COLS*ROWS,fpt);
    fclose(fpt);
    fpt = NULL;

    return image;

}

/*
 * Save image to the out.ppm
 */
void save_image(unsigned char* image_chr,float* image_int, char name[300]){

    FILE* pFile;
    pFile = fopen(name,"w");
    fprintf(pFile,"P5 %d %d 255\n",COLS,ROWS);
    assert((image_chr==NULL&&image_int!=NULL) || (image_chr!=NULL&&image_int==NULL));   
    if(image_chr == NULL){
        image_chr = (unsigned char*)calloc(ROWS*COLS,sizeof(unsigned char));   
        for(int i=0;i<ROWS*COLS;i++) image_chr[i] = (unsigned char)image_int[i];
        fwrite((unsigned char*)image_chr,COLS*ROWS,1,pFile);
        free(image_chr);
    }
    
    else{
        fwrite((unsigned char*)image_chr,COLS*ROWS,1,pFile);
    }
    
    fclose(pFile);

}

/*
 * Show crossing for the matched initial coordinatte:w
 *
 *
 */
unsigned char* show_crosshair(unsigned char* image, PIXEL point, int color){
    //         p1
    //         p2
    //         p3
    //p6 p7 p8 p0 p9 p10 011
    // *       p4
    // *       p5
    // *       p6
    int i = 0;
   // "-" case
   for(i=-3; i<=3; i++){
       image[(point.y-i)*(COLS)+point.x] = color;
   }
   // "|" case
   for(i=-3; i<=3; i++){
       image[point.y*COLS+point.x+i ] = color;
   }
   return image;

}


/* Active_contour for contour interation.
 *
 *
 */
PIXEL* active_contour(unsigned char*image, PIXEL* new_pixel){
    //PIXEL* new_pixel     = NULL;
    float* sobel_template  = NULL;
    unsigned char* s = (unsigned char*)calloc(ROWS*COLS,sizeof(unsigned char));
    float* internalenergy1 = (float*)calloc(7*7,sizeof(float));
    float* internalenergy2 = (float*)calloc(7*7,sizeof(float));
    float* externalenergy  = (float*)calloc(7*7,sizeof(float));
    float* total_energy  = (float*)calloc(7*7,sizeof(float));
   // new_pixel            = (PIXEL*)calloc(line,sizeof(PIXEL));
    
    //for(int i=0; i<line; i++) new_pixel[i]=ori_pixel[i];
    //1. Calculate Sobel templeate.
    sobel_template = sobel(image);
    //normalized(sobel_template,0,255);
    
    //save_image(NULL,sobel_template,"sobel.ppm");
    normalized(sobel_template,NULL,0,1);
    int c=0,r=0;
    float average_dis = 0;
    PIXEL current_contour_pix;
    PIXEL next_contour_pix;
    int iters = 0, i=0;
    while(iters<30){
        for(int i=0;i<ROWS*COLS;i++) s[i]=image[i];
        average_dis = contour_avg_dis(new_pixel);
        for(i=0;i<line;i++){
            //clear energy buffer;
            internalenergy1 = clear_energy_buffer(internalenergy1);
            internalenergy2 = clear_energy_buffer(internalenergy2);
            externalenergy  = clear_energy_buffer(externalenergy);
            total_energy    = clear_energy_buffer(total_energy);
            //setting up countour i and contour i+1.
            current_contour_pix = new_pixel[i];
            if (i==line-1){
                next_contour_pix = new_pixel[0];
            }else{
               next_contour_pix = new_pixel[i+1];
            }

            for(r=0;r<7;r++){
                for(c=0;c<7;c++){
                //Internal Energy1.

                internalenergy1[r*7+c] = InternalEnergy1(current_contour_pix, next_contour_pix,
                                                  r,c);
                //Internal Energy2.
                internalenergy2[r*7+c] = InternalEnergy2(average_dis, 
                                             current_contour_pix,next_contour_pix, r, c);
                //external energy.
                externalenergy[r*7+c] = ExternalEnergy(current_contour_pix, 
                                                                   sobel_template, r, c);    
    
                }//end of for (c)
            }//end of for (r)
        //normalized
         
         internalenergy1 = normalized(NULL,internalenergy1, 0 ,1);
          internalenergy2 = normalized(NULL,internalenergy2, 0 ,1);
        //Update contours
         int flag = 0;
         float min_x=0,min_y=0,min=0;
          for(r=0; r<7; r++){
              for(c=0; c<7; c++){
                  total_energy[r*7+c] =  internalenergy1[r*7+c]
                                       +internalenergy2[r*7+c]
                                       -externalenergy[r*7 +c]; 
                  if (flag == 0){
                      min = total_energy[0];
                      min_x = 0;
                      min_y = 0;
                      flag = 1;
                  }
                  if (total_energy[r*7+c] < min){
                      min = total_energy[r*7+c];
                      min_x = c;
                      min_y = r;

                  }
             
             }
              
          }


          for(r=0; r<7; r++){
              for(c=0; c<7; c++){

              }
              }
         new_pixel[i].x += min_x - 3; 
         new_pixel[i].y += min_y - 3;
         
        }//end of for (<line)

   for (i=0;i<line;i++){
       s = show_crosshair(s, new_pixel[i],BLACK);
   }
   if(show_process_flag){
       save_image(s,NULL,"show.ppm");
       sleep(1);
   }
   if(iters!=0){
       if(iters==0 || iters==4 || iters==9 || iters==19 || iters==29){
           char PATH[30];
           sprintf(PATH,"src/proc/iter_%d.ppm",iters+1);
           save_image(s,NULL,PATH);
       } 
   }
   //sleep(1);
    iters++;
    }//end of while (iter)
    free(internalenergy1);
    free(internalenergy2);
    free(externalenergy);
    free(total_energy);
    internalenergy1=NULL;
    internalenergy2=NULL;
    externalenergy = NULL;
    total_energy = NULL;

   return new_pixel;
}
float* clear_energy_buffer(float* buffer){
    for(int i=0; i<49; i++) buffer[i] = 0;
    return buffer;


}
float ExternalEnergy(PIXEL current, float* sobel_template, int r, int c){ 
    int R = current.y + r -3; 
    int C = current.x + c -3;
    return sobel_template[R*COLS+C];


}
float InternalEnergy2(float avg_dis, PIXEL current, PIXEL next, int r, int c){
    float res=0;
   
    res = SQR(avg_dis - sqrt(
                             SQR( (current.y + r-3 - next.y ) )+
                             SQR( (current.x + c - 3 - next.x) )
                             )
              );
    return res;
    
}


float contour_avg_dis(PIXEL* new_pixel){
    float avg_dis = 0;
    for(int i=0;i< line;i++){
        if( i == line-1){
            avg_dis += sqrt(SQR(new_pixel[i].y - new_pixel[0].y)
                           +SQR(new_pixel[i].x - new_pixel[0].x));
        }else{
            avg_dis += sqrt( SQR(new_pixel[i].y - new_pixel[i+1].y)
                            +SQR(new_pixel[i].x - new_pixel[i+1].x));
        }


    }

    
    return avg_dis/(line);


}
float InternalEnergy1(PIXEL current_con, PIXEL next_con,int r,int c ){
    float Distance=0;
    Distance =  SQR(current_con.y + r-3 - next_con.y )
               +SQR(current_con.x + c - 3 - next_con.x);
    return Distance;
    
}
float* sobel(unsigned char* image){
    int c=0, r=0;
    float* Gx_template       = (float *)calloc(ROWS*COLS,sizeof(float));
    float* Gy_template       = (float *)calloc(ROWS*COLS,sizeof(float));
    float* gradient_template = (float *)calloc(ROWS*COLS,sizeof(float));

    //horizental changes
    
    int Gx[9] = {  1, 0, -1,
                   2, 0, -2,   
                   1, 0, -1 };  
    //vertical changes.              
    int Gy[9] = {  1, 2,  1,
                   0, 0,  0,
                  -1, -2, -1 };
    Gx_template = conv(image,Gx_template,Gx);
    Gy_template = conv(image,Gy_template,Gy);
    for(int i=0; i<ROWS*COLS; i++){
         gradient_template[i] = (int)ceil( sqrt( SQR(Gx_template[i]) + SQR(Gy_template[i])) );
         if(gradient_template[i] > 255) gradient_template[i] = 255;
    }


    free(Gx_template);
    free(Gy_template);

    return gradient_template;
}

float* conv(unsigned char* image, float* template, int G[9] ){
    int r,c,r2,c2;
    float sum;
    int COLS_g = 3;
    int ROWS_g = 3;
    for (r = 1; r < ROWS - 1; r++){
        for (c = 1; c < COLS - 1; c++) {
            sum=0;
            for (r2 =-1; r2<=1; r2++){
                 for (c2 = -1; c2 <= 1; c2++){
                    sum += (int)G[(r2+1)*3 + c2+1  ] * (int)image[(r+r2)*COLS+(c+c2)];
                }
            }
          template[r*COLS+c] = sum;
            
         }
    }
     return template;
}

//Normalized to 0-255 : MIN = 0, MAX = 255
//Normalized to 0-1 : MIN = 0, MAX = 1.

float* normalized(float* ori, float* small, int MIN, int MAX){
  assert((ori==NULL&&small!=NULL) || (ori!=NULL&&small==NULL));   
  int r=0,c=0;
  float* buffer= ori==NULL?small:ori;
  assert(buffer!=NULL);
  float min = buffer[0];
  float max = buffer[0];
  int cols=0,rows=0;
  if( small == NULL){
      cols = COLS;
      rows = ROWS;
  } else{
      cols = 7;
      rows = 7;
  }

     for (r=0; r<rows; r++){
         for (c=0; c<cols; c++){
             min = buffer[(r)*cols+(c)]<=min?buffer[(r)*cols+(c)]:min;
             max = buffer[(r)*cols+(c)]>=max?buffer[(r)*cols+(c)]:max;
         }
     }
     for (r=0; r<rows; r++){
         for (c=0; c<cols; c++){
             buffer[(r)*cols+(c)] =  (buffer[(r)*cols+(c)] - min)*MAX / (max - min  );
         }

     }
     
    return buffer;
}
