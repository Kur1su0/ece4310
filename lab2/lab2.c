
/*
** lab2.c
** ECE 4310 FALL 2018
** letter detection.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#define image_path "src/parenthood.ppm"
#define e_path     "src/parenthood_e_template.ppm"
#define w_r         7
#define w_c         4


void main(int argc, char const *argv[]){
  FILE	             	*fpt;
  unsigned char	        *image;
  unsigned char	        *e;
  int	                *e_msf;
  int                   *msf_image;
  unsigned char         *image_8;

  char	             	header[320];
  char		            header_e[320];
  int		            ROWS, COLS, BYTES;
  int		            ROWS_e, COLS_e, BYTES_e;
  int		            r, c, r2, c2, sum;
  int                   e_total=0, e_avg=0;
  int                   threshold = -1;
  if(argc!=2) {printf("too few argument\n");exit(1);}
  threshold = atoi(argv[1]);
  if (threshold <= 0){printf("error threshold\n"); exit(1);}
  
  if ((fpt=fopen(image_path,"rb")) == NULL) {
      printf("Unable to open text.ppm for reading\n");
      exit(0);
  }
  fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
  if (strcmp(header,"P5") != 0  ||  BYTES != 255) {
      printf("Not a greyscale 8-bit PPM image\n");
      exit(0);
  }
  image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

  fread(image,1,COLS*ROWS,fpt);
  fclose(fpt);


  //Read "e"
  if ((fpt=fopen(e_path,"rb")) == NULL) {
      printf("Unable to open e.ppm for reading\n");
      exit(0);
  }
  fscanf(fpt,"%s %d %d %d\n",header_e,&COLS_e,&ROWS_e,&BYTES_e);
  if (strcmp(header_e,"P5") != 0  ||  BYTES_e != 255) {
      printf("Not a greyscale 8-bit PPM image\n");
      exit(0);
  }
  //image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
  e = (unsigned char *)calloc(ROWS_e*COLS_e,sizeof(unsigned char));
  //fseek(fpt, 0, SEEK_SET);
  //header[0]=fgetc(fpt);
  fread(e,1,COLS_e*ROWS_e,fpt);
  fclose(fpt);
  for (r=0; r<ROWS_e; r++){
      for (c=0; c<COLS_e; c++){
  //        printf("%8d", (int)e[(r)*COLS_e+(c)]);
          e_total+=(int)e[(r)*COLS_e+(c)];

      }
  //    printf("\n");
  }
  e_avg  = e_total/(COLS_e*ROWS_e);
  //printf("total = %d, avg = %d\n", e_total, e_avg);
  //printf("row e: %d, col e:%d\n",ROWS_e, COLS_e);
  //printf("row  : %d, col  :%d\n",ROWS, COLS);


  e_msf =(int *)calloc(ROWS_e*COLS_e,sizeof(int));

  for (r=0; r<ROWS_e; r++){
      for (c=0; c<COLS_e; c++){
          e_msf[(r)*COLS_e+(c)] = (int)e[(r)*COLS_e+(c)] - e_avg;
        //  min = e_msf[(r)*COLS_e+(c)]<=min?e_msf[(r)*COLS_e+(c)]:min;
    //      printf("%8d",(int)e_msf[(r)*COLS_e+(c)]);
      }
  //    printf("\n");
  }
  //printf("min = %d\n", min);
  //Convert to 8 bit MSF.

// multiple
    msf_image =(int *)calloc(ROWS*COLS,sizeof(int));
    for (r = w_r; r < ROWS - w_r; r++){
        for (c = w_c; c < COLS - w_c; c++) {
            sum=0;
            for (r2 =-w_r; r2<=w_r; r2++){
                for (c2 = -w_c; c2 <= w_c; c2++){
                    sum += (int)e_msf[(r2+w_r)*COLS_e+(c2+w_c)] * (int)image[(r+r2)*COLS+(c+c2)];
                }
            }msf_image[r*COLS+c] = (int) (sum);

        }
      //  printf("\n");
      }
     int min = msf_image[0];
     int max = msf_image[0];

        for (r=0; r<ROWS; r++){
            for (c=0; c<COLS; c++){
                min = msf_image[(r)*COLS+(c)]<=min?msf_image[(r)*COLS+(c)]:min;
                max = msf_image[(r)*COLS+(c)]>=max?msf_image[(r)*COLS+(c)]:max;
            }
        }
//printf("max: %d\n", max);
//printf("min: %d\n", min);
//convert to 8bit
        image_8 =(unsigned char*)calloc(ROWS*COLS,sizeof(unsigned char));
        for (r=0; r<ROWS; r++){
            for (c=0; c<COLS; c++){
                
                image_8[(r)*COLS+(c)] =(unsigned char) ( (msf_image[(r)*COLS+(c)] - min)*255 / (max - min  );
                
            }

        }
        unsigned char* image_out = NULL;
        int flag = 0;
        FILE* pFile;
  //      pFile = fopen("log","w");
        image_out =(unsigned char*)calloc(ROWS*COLS,sizeof(unsigned char));
        /*for (r=0; r<ROWS; r++){
            for (c=0; c<COLS; c++){
                if(image_8[(r)*COLS+(c)] >= threshold){
                    flag = 1;
                    
                }
            }
        }*/
        
       for (r=w_r; r<ROWS-w_r; r+=2*w_r+1){
            for (c=w_c; c<COLS-w_c; c+=2*w_c+1){
              for (r2 =-w_r; r2<=w_r; r2++){
                  for (c2 = -w_c; c2 <= w_c; c2++){
                      if((int)image_8[(r+r2)*COLS+(c+c2)]>= threshold){
                        flag = 1;
                      
                      }
                  }
               }
                  image_out[r*COLS+c] = flag==1?(unsigned char)255:(unsigned char)0;
                  //if (flag==1) fprintf(pFile,"e %d %d\n",(r*COLS+c)%COLS,(r*COLS+c)/COLS);
                  if (flag==1) printf("%d %d\n",(r*COLS+c)%COLS,(r*COLS+c)/COLS);
                  flag = 0;
              
            }

        }
        
       // fclose(pFile);
       //thres output
        char path[100];
        //snprintf(path,sizeof(path), "ppm/out_%d.ppm",threshold);
        //fpt=fopen("out_.ppm","w");
        fpt=fopen("0_test","w");
        fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
        fwrite(image_8,COLS*ROWS,1,fpt);
        fclose(fpt);
      
        free(image);
        free(e);
        free(e_msf);
        free(msf_image);
        free(image_8);
        free(image_out);
}
