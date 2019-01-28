
/*
** lab3.c
** Zice Wei
** zicew
** ECE 4310 FALL 2018
** letter detection.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define image_path "src/parenthood.ppm"
#define e_path     "src/parenthood_e_template.ppm"
#define msf_path   "src/msf.ppm"
#define gt_path   "src/gt"
#define w_r         7
#define w_c         4
int thre;
//prototype
/*
typedef struct _PIXEL{
    unsigned char pixel;
    int           mark = 0;
}_PIXEL; */

// unsigned char* readimage(unsigned char*, int);
void image_saver(int, unsigned char*,int,int,int,int);
unsigned char* thin(unsigned char*, int, int);
void get_end_branch_point(unsigned char*, int*,int*,int,int);


int main(int argc, char const *argv[])
{
    FILE	             	*fpt;
    FILE	             	*pFile;
    unsigned char	        *image;
    unsigned char         *msf;
    unsigned char	        *e;

    char	             	header[320];
    char		            header_e[320];
    char		            header_msf[320];

    int		            ROWS, COLS, BYTES;
    int		            ROWS_e, COLS_e, BYTES_e;
    int		            ROWS_msf, COLS_msf, BYTES_msf;

    int		            r, c, r2, c2, sum;
    int                   threshold = -1;


    if(argc!=2) {
        printf("too few argument\n");
        exit(1);
    }
    threshold = atoi(argv[1]);
    thre = threshold;
    if (threshold <= 0) {
        printf("error threshold\n");
        exit(1);
    }

    if ((fpt=fopen(image_path,"rb")) == NULL) {
        printf("Unable to open text.ppm for reading\n");
        exit(0);
    }
    fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
    if (strcmp(header,"P5") != 0  ||  BYTES != 255) {
        printf("Not a greyscale 8-bit PPM image\n");
        exit(0);
    }

    image     =    (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

    fread(image,1,COLS*ROWS,fpt);
    fclose(fpt);
    fpt = NULL;

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
    e         =    (unsigned char *)calloc(ROWS_e*COLS_e,sizeof(unsigned char));


    fread(e,1,COLS_e*ROWS_e,fpt);
    fclose(fpt);
    fpt = NULL;

    //Read msf
    if ((fpt=fopen(msf_path,"rb")) == NULL) {
        printf("Unable to open msf.ppm for reading\n");
        exit(0);
    }
    fscanf(fpt,"%s %d %d %d\n",header_msf,&COLS_msf,&ROWS_msf,&BYTES_msf);
    if (strcmp(header_msf,"P5") != 0  ||  BYTES_msf != 255) {
        printf("Not a greyscale 8-bit PPM image\n");
        exit(0);
    }

    msf         =    (unsigned char *)calloc(ROWS_msf*COLS_msf,sizeof(unsigned char));
    unsigned char* temp;
    unsigned char* temp2;
    temp         =    (unsigned char *)calloc(ROWS_e*COLS_e,sizeof(unsigned char));
    temp2         =    (unsigned char *)calloc(ROWS_e*COLS_e,sizeof(unsigned char));


    fread(msf,1,COLS_msf*ROWS_msf,fpt);
    fclose(fpt);
    fpt = NULL;

    int flag = 0;




    if ((pFile=fopen(gt_path,"rb")) == NULL) {
        printf("Unable to open gt for reading\n");
        exit(0);
    }
    char letter;
    int  gt_cols = 0, gt_rows = 0;
    int  count = 0;
    flag = -1;

    int TP=0,FP=0, TN=0, FN=0;
    float TPR=0, FPR=0;
    int i = 0;
    int branch=0,end=0;
    while(1) {
        flag = 0;
         //branch=0;end=0;
        //read data
        fscanf(pFile, "%c %d %d\n",&letter, &gt_cols, &gt_rows);
      //  printf("%c: %d %d\n",letter, gt_cols, gt_rows );

        //#define w_r         7
        //#define w_c         4

        //a. iii copy letter to temp
        for(r=-w_r;r<=w_r;r++){
         for(c=-w_c;c<=w_c;c++){
             if(msf[(r+gt_rows)*COLS_msf+c+gt_cols] > (unsigned char)threshold)
                 flag = 1;
             temp[i] = image[(r+gt_rows)*COLS +c+gt_cols];
             i++;

         }

     }

        //a. iv thre the image to 128
        for(i=0; i<ROWS_e*COLS_e; i++) {
            temp[i] = temp[i]>(unsigned char)128?(unsigned char)0:(unsigned char)255;
        }
          i=0;

      //e 164 341
      //r 164 75
     //f 447 25 
        if(gt_cols==164 && gt_rows==75 ){
          printf("before\n");
          for(r=0;r<ROWS_e;r++){
               for(c=0;c<COLS_e;c++){
                   printf("%4d",temp[r*COLS_e+c]);
               }printf("\n");
           }printf("\n\n");

           image_saver(0,temp, COLS_e,ROWS_e, gt_cols,gt_rows);
           temp2 = thin(temp, COLS_e, ROWS_e);



          image_saver(1,temp2, COLS_e,ROWS_e, gt_cols,gt_rows);
          printf("after\n");

        for(r=0;r<ROWS_e;r++){
             for(c=0;c<COLS_e;c++){
                 printf("%4d",temp2[r*COLS_e+c]);
             }printf("\n");
         }printf("\n\n");
         get_end_branch_point(temp2,&branch,&end, COLS_e, ROWS_e);
         printf("branch:%d ,end:%d\n",branch,end);
          branch =0; end =0;
         exit(1);

        }

        //image_saver(0,temp, COLS_e,ROWS_e, gt_cols,gt_rows);

        //a. v. Thin
        //image_saver(1,temp2, COLS_e,ROWS_e, gt_cols,gt_rows);

        //printf("%c %d %d\n",letter, gt_cols, gt_rows);

        temp2 = thin(temp, COLS_e, ROWS_e);
        get_end_branch_point(temp2,&branch, &end,COLS_e, ROWS_e);
        //printf("%c: %d %d\n",letter, gt_cols, gt_rows );

        //printf("branch:%d ,end:%d\n",branch,end);

        //b. count up FPs and TPs

        if(flag==1) {
          if(letter=='e'){
            image_saver(1,temp2, COLS_e,ROWS_e, gt_cols,gt_rows);
            TP++;
          }else{
            FP++;
          }

        }else{
             if(end==1&&branch==1){

               if(letter=='e'){
              // image_saver(1,temp2, COLS_e,ROWS_e, gt_cols,gt_rows);
                  TP++;
               }
               else{
                 FP++;
               }
             }
             else{
               if(letter=='e'){
                  FN++;
               }
               else{
                 TN++;
               }

             }

        }


        flag = 0;
         branch =0; end =0;
        //Set the pFile back.
        if (feof(pFile)) {

            fseek(pFile,0,SEEK_SET);
            break;
        }



    }
    //calculate
    TPR = (float)TP /(TP+FN);
    FPR = (float)FP /(FP+TN);
    printf("%d %d %f %f\n", TP, FP, TPR, FPR);

    return 0;
}


//save image.
void image_saver(int thre_flag,unsigned char* image,
                 int cols_e,int rows_e,
                 int pixel_x , int pixel_y)
{
    FILE* pFile;
    char path[30];
    if(thre_flag==1){
        snprintf(path,sizeof(path),"out/letter/%d/%d_%d_thre.ppm",thre,pixel_x, pixel_y);
    }
    else{
        snprintf(path,sizeof(path),"out/letter/%d/%d_%d.ppm",thre,pixel_x, pixel_y);
    }
    pFile = fopen(path,"w");
    fprintf(pFile,"P5 %d %d 255\n",cols_e,rows_e);
    fwrite(image,rows_e*cols_e,1,pFile);
    fclose(pFile);

}

int get_egde( unsigned char* letter, int COLS_e, int ROWS_e,
              int* p0, int* p1, int* p2, int* p3,
              int* p4, int* p5, int* p6, int* p7, int*p8,int r,int c){
              int i=0 ,j=0;
              int e_to_ne=0;
              int new_col = COLS_e+2;
              int new_row = COLS_e+2;
              int real_pixel_head = new_col+1; //12
              int edge_array[9];
              int r2=0,c2=0;


                  //------------------------
                  // p1 p2 p3
                  // p8 p0 p4
                  // p7 p6 p5
                  //------------------------


                 if( letter[r*(COLS_e)+c]!=0) {

                    *p0 = letter[r*(COLS_e) +  c            ];
                    *p1 = letter[(r-1)*(COLS_e) +  c - 1    ];
                    *p2 = letter[(r-1)*(COLS_e) +  c        ];
                    *p3 = letter[(r-1)*(COLS_e) +  c + 1    ];
                    *p4 = letter[r*(COLS_e) +  c     + 1    ];
                    *p5 = letter[(r+1)*(COLS_e) +  c + 1    ];
                    *p6 = letter[(r+1)*(COLS_e) +  c        ];
                    *p7 = letter[(r+1)*(COLS_e) +  c - 1    ];
                    *p8 = letter[r*(COLS_e) +  c     - 1    ];

                      if(r==0){
                        *p1=0;
                        *p2=0;
                        *p3=0;
                      }
                      if(c==0){
                        *p1=0;
                        *p8=0;
                        *p7=0;

                      }
                      if(c==COLS_e-1){
                        *p3=0;
                        *p4=0;
                        *p5=0;

                      }
                      if(r==ROWS_e-1){
                        *p5=0;
                        *p6=0;
                        *p7=0;
                      }

                      edge_array[0] = *p1;
                      edge_array[1] = *p2;
                      edge_array[2] = *p3;
                      edge_array[3] = *p4;
                      edge_array[4] = *p5;
                      edge_array[5] = *p6;
                      edge_array[6] = *p7;
                      edge_array[7] = *p8;
                      edge_array[8] = *p1;


                      i=0;
                      while(i!=8){
                        if(edge_array[i]==255&&edge_array[i+1]==0) e_to_ne++;
                        i++;
                      }

                      return e_to_ne;

                  } else {
                      e_to_ne = 0;
                      //          continue;
                      return e_to_ne;

                  }

}
int get_neighbour(int p0,int p1,int p2,int p3,int p4,int p5,int p6,int p7,int p8){
    int neighbour=0;

  if(p0==0) {
      neighbour = 0;
  } else {
      if(p1==255) neighbour++;
      if(p2==255) neighbour++;
      if(p3==255) neighbour++;
      if(p4==255) neighbour++;
      if(p5==255) neighbour++;
      if(p6==255) neighbour++;
      if(p7==255) neighbour++;
      if(p8==255) neighbour++;
  }
  return neighbour;
}





unsigned char* thin(unsigned char* letter, int COLS_e, int ROWS_e)
{

    int mark = 1;
    int r,c, r2, c2;
    int e_to_ne=0;
    int neighbour=0;
    int i=0,j=0;
    int count=0;
    int p0,p1,p2,p3,p4,p5,p6,p7,p8;
    int real_pixel_head;

    //------------------------
    // p1 p2 p3
    // p8 p0 p4
    // p7 p6 p5
    //------------------------


    int mark_array[COLS_e*ROWS_e];

    //1. pass through image.
    while(mark > 0) {
        mark=0;
        //initialize the array,
        for(i=0; i<ROWS_e*COLS_e; i++) {
            mark_array[i] = -2;
        }

        //printf("ori:\n");
    //    for(r=0;r<ROWS_e;r++){
    //       for(c=0;c<COLS_e;c++){
    //           printf("%4d",letter[r*COLS_e+c]);
      //     }printf("\n");
    //    }printf("\n\n");

        for (r = 0; r < ROWS_e ; r++) {
            for (c = 0 ; c < COLS_e ; c++) {


              p0=0;p1=0;p2=0;p3=0;p4=0;p5=0;p6=0;p7=0;p8=0;

              e_to_ne = get_egde(letter,COLS_e,ROWS_e,
                            &p0,&p1,&p2,&p3,&p4,&p5,&p6,&p7,&p8,r,c);
               if (p0==0) continue;
                //3.checking neighbour
                neighbour =  get_neighbour(p0,p1,p2,p3,p4,p5,p6,p7,p8);

              // 4.check N, E, or(W and S) are nor edge pixels
                // p1     p2(A)  p3
                // p8(C)  p0     p4(B)
                // p7     p6(D)  p5

                int _4_pass = 0;
               if((p2==0) || (p4==0) || (p8==0&&p6==0)) {
                    _4_pass = 1;
                }
                if  (( (neighbour>=2) && (neighbour<=6)) && (e_to_ne==1)&&(_4_pass==1) ) {

                    _4_pass = 0;
                     mark_array[mark] = r*(COLS_e) +  c ;
                      mark++;
                }
            //    printf("mark %d, nei %d, edge %d\n",mark,neighbour,e_to_ne);
            }
        }  //end of row loop.




    			for (i =0; i < COLS_e*ROWS_e; i++)
    			{
    				if(mark_array[i]==-2) break;
    					// printf("c: %d  r: %d \n",c, r );
    			  letter[mark_array[i]]= (unsigned char)0;

    			}


//printf("mark %d\n",mark);
    }//end of while
/*
     printf("result:\n");
     for(r=0;r<ROWS_e;r++){
        for(c=0;c<COLS_e;c++){
            printf("%4d",letter[r*COLS_e+c]);
        }printf("\n");
     }printf("\n\n");
*/

//exit(1);
  return letter;

}


//void get_end_branch_point(unsigned char*, int*,int*,int,int,int,int);
void get_end_branch_point(unsigned char* letter,int* branch,int* end,
                           int COLS_e, int ROWS_e){
    int r=0, c=0;
    int status=0;
    int p1=1,p2=0,p3=0,p4=0,p5=0,p6=0,p7=0,p8=0,p0=0;
    int _end=0;
    int _branch=0;
    for(r=0;r < ROWS_e; r++){
        for(c=0; c < COLS_e; c++){
            if(letter[r*COLS_e+c]==0){
                //do nothing.
            }else{

                status =get_egde( letter, COLS_e, ROWS_e,&p0, &p1, &p2, &p3,&p4, &p5, &p6, &p7, &p8,r,c);
                if(status == 1) _end++;
                if(status > 2)  _branch++;
                 }

       }

    }
    *end = _end;
    *branch =_branch;

    //return (end==1&&branch==1)?1:0;



}
