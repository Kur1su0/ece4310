/* Purpose: data segement.
 * lab7.c
 *
 *
 */



 #include <stdio.h>
 #include <stdlib.h>
 #include <float.h>

#define PATH "acc_gyro.txt"
#define DATA_SIZE 1250

#define WINDOW 20
#define VARIANCE_WINDOW 15 //odd number required.
#define SQR(X) ((X) * (X))
// Super structure.
typedef struct table{
    double time ;
    double accX ;
    double accY ;
    double accZ ;
    double pitch;
    double roll ;
    double yaw  ;

}table;

table thres_small;



//prototype
table* get_variance(table* data, int size);
table* get_period(table* data_smooth, table* variance, int new_size);
void  get_result(table* data_smooth, table* isMotion, int size);
table whosWorking(table* isMotion, int i);
void check_rest(table* isMotion,int size);
void check_acc(table* data_smooth,table* isMotion,int size);
void check_gyo(table* data_smooth,table* isMotion,int size);

void print_table(table* data, int size){
    int i = 0;
    for(i=0; i < size; i++){
        //if(data[i].accX == 0 ) continue;
        //printf("%d %lf %lf %lf %lf %lf %lf %lf\n",i,data[i].time, data[i].accX, data[i].accY, data[i].accZ
        //                                          ,data[i].pitch, data[i].roll, data[i].yaw);
        printf("%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n",data[i].time, data[i].accX, data[i].accY, data[i].accZ
                                                  ,data[i].pitch, data[i].roll, data[i].yaw);
    }
}

int main(){
    FILE* fpt = NULL;
    table* data = (table*)calloc(DATA_SIZE,sizeof(table));
    if((fpt=fopen(PATH,"rb"))==NULL){
        printf("Unable to open %s",PATH);
        exit(1);
    }


    /*
        thres_small.accX = 0.015;
        thres_small.accY = 0.015;
        thres_small.accZ = 0.015;*/
    thres_small.time = -1;

    thres_small.accX = 0.000147;
    thres_small.accY = 0.000157;
    thres_small.accZ = 0.0048;
    thres_small.pitch = 0.005;
    thres_small.roll = 0.035;
    thres_small.yaw =  0.009;

    //skip first line
    char junk[100];
    fscanf(fpt,"%s %s %s %s %s %s %s\n",junk,junk,junk,junk,junk,junk,junk);
    int i= 0;
    while(!feof(fpt)){

        fscanf(fpt,"%lf %lf %lf %lf %lf %lf %lf\n", &data[i].time
                                          , &data[i].accX, &data[i].accY, &data[i].accZ
                                           , &data[i].pitch, &data[i].roll, &data[i].yaw);
        //fscanf(fpt,"%lf", &data[i].time);
        i++;


    }
    fclose(fpt);
    //print_table(data, 1250);

    table* data_smooth = (table*)calloc(DATA_SIZE,sizeof(table));

    for(int j= WINDOW/2; j < DATA_SIZE - WINDOW/2; j++){
        table sum;

        sum.time = 0;
        sum.accX = 0;
        sum.accY = 0;
        sum.accZ = 0;
        sum.pitch= 0;
        sum.roll = 0;
        sum.yaw  = 0;


        for( int k = -WINDOW/2; k < WINDOW/2; k++){
            sum.time += data[j+k].time;
            sum.accX += data[j+k].accX;
            sum.accY += data[j+k].accY;
            sum.accZ += data[j+k].accZ;
            sum.pitch+= data[j+k].pitch;
            sum.roll += data[j+k].roll;
            sum.yaw  += data[j+k].yaw;


        }
           data_smooth[j].time  = sum.time / WINDOW;
           data_smooth[j].accX  = sum.accX / WINDOW;
           data_smooth[j].accY  = sum.accY / WINDOW;
           data_smooth[j].accZ  = sum.accZ / WINDOW;
           data_smooth[j].pitch = sum.pitch/ WINDOW;
           data_smooth[j].roll  = sum.roll / WINDOW;
           data_smooth[j].yaw   = sum.yaw  / WINDOW;

    }
    //print_table(data_smooth, 1250);
    int less = (WINDOW/2)*2;
    //less = less *2;
    printf("less%d\n",less);
    int new_size = DATA_SIZE - less;
    table* data_updated = (table*)calloc( (new_size), sizeof(table) );
    int j = 0;
    for(int i=0;i<DATA_SIZE;i++){
        if(data_smooth[i].time!=0.0){
            data_updated[j] = data_smooth[i];
            j++;
        }

    }

    free(data_smooth);
    data_smooth = NULL;
    data_smooth = data_updated;
    data_updated = NULL;

  //  print_table(data_smooth, new_size);


    table* variance = NULL;
    variance = get_variance(data_smooth, new_size);
    //print_table(variance, new_size-VARIANCE_WINDOW/2);
    table* isMotion = get_period(data_smooth, variance, new_size);
    //print_table(isMotion, new_size);

    get_result(data_smooth, isMotion, new_size);


return 0;
}

void get_result(table* data_smooth, table* isMotion, int size){

    printf("%s\t%s\t\t%s\t\t%s\t\t%s\n","Type","start","end","Perid","length/angle");
    //check_rest(isMotion, size);
    check_gyo(data_smooth, isMotion, size);
    //check_acc(data_smooth, isMotion, size);

//    for


}
void check_acc(table* data_smooth, table* isMotion, int size){
  int chunk = 0;
  int isStart = 0;
  int start=0, end=0;
  double Vavg=0,Vfin=0,Vinit=0;
  double dist_accX=0, dist_accY=0, dist_accZ=0;
    //accX
    for(int i = 0; i<size;i++){
        if( whosWorking(isMotion,i).accX == 1){
            if(isStart == 0 ){
                start = i;
                isStart = 1;
            }
            Vinit = Vfin;
            Vfin += data_smooth[i].accX * 0.05;
            Vavg = (Vinit + Vfin) / (double)2;
            dist_accX += Vavg * 0.05;
            chunk++;
        }
        else{
            if(isStart == 1){
                end = i;
                printf("accX\t%f\t%f\t%f\t%f\n",
                                start*0.05, end*0.05, (end-start)*0.05,dist_accX);
                isStart = 0;
                start = 0;
                end = 0;
                chunk = 0;
                dist_accX = 0;
                Vavg=0,Vfin=0,Vinit=0;
            }
      }

  }

  //dist_accYfor(int i = 0; i<size;i++){
   for(int i = 0; i<size;i++){
      if( whosWorking(isMotion,i).accY == 1){
          if(isStart == 0 ){
              start = i;
              isStart = 1;
          }
          Vinit = Vfin;
          Vfin += data_smooth[i].accY * 0.05;
          Vavg = (Vinit + Vfin) / (double)2;
          dist_accY += Vavg * 0.05;
          chunk++;
      }
      else{
          if(isStart == 1){
              end = i;
              printf("accY\t%f\t%f\t%f\t%f\n",
                              start*0.05, end*0.05, (end-start)*0.05,dist_accY);
              isStart = 0;
              start = 0;
              end = 0;
              chunk = 0;
              dist_accY = 0;
              Vavg=0,Vfin=0,Vinit=0;
          }
    }

}
//accZ

for(int i = 0; i<size;i++){
   if( whosWorking(isMotion,i).accZ == 1){
       if(isStart == 0 ){
           start = i;
           isStart = 1;
       }
       Vinit = Vfin;
       Vfin += data_smooth[i].accZ * 0.05;
       Vavg = (Vinit + Vfin) / (double)2;
       dist_accZ += Vavg * 0.05;
       chunk++;
   }
   else{
       if(isStart == 1){
           end = i;
           printf("accZ\t%f\t%f\t%f\t%f\n",
                           start*0.05, end*0.05, (end-start)*0.05,dist_accZ);
           isStart = 0;
           start = 0;
           end = 0;
           chunk = 0;
           dist_accZ = 0;
           Vavg=0,Vfin=0,Vinit=0;
       }
 }

}




}

void check_gyo(table* data_smooth, table* isMotion, int size){
  int chunk = 0;
  int isStart = 0;
  int start=0, end=0;
  double angle_pitch=0, angle_roll=0, angle_yaw=0;

  double VavgX=0,VfinX=0,VinitX=0;
  double VavgY=0,VfinY=0,VinitY=0;
  double VavgZ=0,VfinZ=0,VinitZ=0;

  double dist_accX=0, dist_accY=0, dist_accZ=0;
    //pitch
    for(int i = 0; i<size;i++){
    //time means rest
        if( whosWorking(isMotion,i).accZ == 1){

            if(isStart == 0 ){
                start = i;
                isStart = 1;
            }
            angle_pitch+=data_smooth[i].pitch ;
            angle_roll+=data_smooth[i].roll ;
            angle_yaw+=data_smooth[i].yaw ;

            VinitX = VfinX;
            VfinX += data_smooth[i].accX * 0.05;
            VavgX = (VinitX + VfinX) / (double)2;
            dist_accX += VavgX * 0.05;


            VinitY = VfinY;
            VfinY += data_smooth[i].accY * 0.05;
            VavgY = (VinitY + VfinY) / (double)2;
            dist_accY += VavgY * 0.05;


            VinitZ = VfinZ;
            VfinZ += data_smooth[i].accZ * 0.05;
            VavgZ = (VinitZ + VfinZ) / (double)2;
            dist_accZ += VavgZ * 0.05;



        }
        else{

        if(isStart == 1){
            end = i;

            printf("accZ\n");
            printf("accX\t%f\t%f\t%f\t%f\n",start*0.05, end*0.05, (end-start)*0.05,dist_accX);
            printf("accY\t%f\t%f\t%f\t%f\n",start*0.05, end*0.05, (end-start)*0.05,dist_accY);
            printf("accZ\t%f\t%f\t%f\t%f\n",start*0.05, end*0.05, (end-start)*0.05,dist_accZ);
            printf("Pitch\t%f\t%f\t%f\t%f\n", start*0.05, end*0.05, (end-start)*0.05,angle_pitch*0.05);
            printf("Roll\t%f\t%f\t%f\t%f\n",  start*0.05, end*0.05, (end-start)*0.05,angle_roll*0.05);
            printf("yaw\t%f\t%f\t%f\t%f\n",start*0.05, end*0.05, (end-start)*0.05,angle_yaw*0.05);
            printf("\n\n\n");

            isStart = 0;
            start = 0;
            end = 0;
            chunk = 0;

            angle_pitch = 0;

            angle_pitch=0; angle_roll=0; angle_yaw=0;

            VavgX=0;VfinX=0;VinitX=0;
            VavgY=0;VfinY=0;VinitY=0;
            VavgZ=0;VfinZ=0;VinitZ=0;

            dist_accX=0; dist_accY=0; dist_accZ=0;
        }
      }

  }
/*
      //roll
      chunk = 0;
      for(int i = 0; i<size;i++){
          if( whosWorking(isMotion,i).roll == 1){
              if(isStart == 0 ){
                  start = i;
                  isStart = 1;
              }
              angle_roll+=data_smooth[i].roll;
          chunk++;

          }
          else{

          if(isStart == 1){
              end = i;
              printf("Roll\t%f\t%f\t%f\t%f\n",  start*0.05, end*0.05, (end-start)*0.05,angle_roll*0.05);
              isStart = 0;
              start = 0;
              end = 0;
              chunk = 0;

              angle_roll = 0;
          }
        }
    }

    chunk = 0;
    for(int i = 0; i<size;i++){
        if( whosWorking(isMotion,i).yaw == 1){
            if(isStart == 0 ){
                start = i;
                isStart = 1;
            }
            angle_yaw+=data_smooth[i].yaw;
        chunk++;

        }
        else{

        if(isStart == 1){
            end = i;
            printf("yaw\t%f\t%f\t%f\t%f\n",start*0.05, end*0.05, (end-start)*0.05,angle_yaw*0.05);
            isStart = 0;
            start = 0;
            end = 0;
            chunk = 0;

            angle_yaw = 0;
        }
      }
  }


*/




}

void check_rest(table* isMotion,int size){
  int chunk = 0;
  int isStart = 0;
  int start=0, end=0;

    for(int i = 0; i<size;i++){
    //time means rest
        if( whosWorking(isMotion,i).time == 1){
            if(isStart == 0 ){
                start = i;
                isStart = 1;
            }
        chunk++;
        }
        else{

        if(isStart == 1){
            end = i;
            printf("Rest\t%f\t%f\t%f\t%s\n",
                            start*0.05, end*0.05, (end-start)*0.05," - ");
            isStart = 0;
            start = 0;
            end = 0;
            chunk = 0;
        }
    }

    }
}

table whosWorking(table* isMotion,int i){
    table result = {
           0,0,0,0,0,0,0
    };


    if(   isMotion[i].accX==0 && isMotion[i].accY==0 && isMotion[i].accZ==0
       && isMotion[i].pitch==0 && isMotion[i].roll==0 && isMotion[i].yaw==0) {
         result.time = 1;
         return result;
   }
   if(isMotion[i].accX != 0) result.accX = 1;
   if(isMotion[i].accY != 0) result.accY = 1;
   if(isMotion[i].accZ != 0) result.accZ = 1;
   if(isMotion[i].pitch != 0) result.pitch = 1;
   if(isMotion[i].roll != 0) result.roll = 1;
   if(isMotion[i].yaw != 0) result.yaw = 1;

return result;


}

table* get_period(table* data_smooth, table* variance, int new_size){
    int w = VARIANCE_WINDOW;
    table* isMotion  = (table*)calloc(new_size, sizeof(table));
    int j = 0;
    //accX
    for(int i=0; i < new_size - w/2; i++){
        j = 0;
        if(variance[i].accX >= thres_small.accX){
            while(j <= w){
                isMotion[i+j].accX = 1;
                j++;
            }
            i += w;
        }
    }

    //accY
    for(int i=0; i < new_size - w/2; i++){
        j = 0;
        if(variance[i].accY >= thres_small.accY){
            while(j <= w){
                isMotion[i+j].accY = 1;
                j++;
            }
            i += w;
        }
    }
    //accZ
    for(int i=0; i < new_size - w/2; i++){
        j = 0;
        if(variance[i].accZ >= thres_small.accZ){
            while(j <= w){
                isMotion[i+j].accZ = 1;
                j++;
            }
            i += w;
        }
    }

    //pitch
    for(int i=0; i < new_size - w/2; i++){
        j = 0;
        if(variance[i].pitch >= thres_small.pitch){
            while(j <= w){
                isMotion[i+j].pitch = 1;
                j++;
            }
            i += w;
        }
    }
    //roll
    for(int i=0; i < new_size - w/2; i++){
        j = 0;
        if(variance[i].roll >= thres_small.roll){
            while(j <= w){
                isMotion[i+j].roll = 1;
                j++;
            }
            i += w;
        }
    }
    //yaw
    for(int i=0; i < new_size - w/2; i++){
        j = 0;
        if(variance[i].yaw >= thres_small.yaw){
            while(j <= w){
                isMotion[i+j].yaw = 1;
                j++;
            }
            i += w;
        }
    }



  return isMotion;
}





table* get_variance(table* data, int size){
    int w = VARIANCE_WINDOW;
    table* variance = (table*)calloc(size, sizeof(table));
    table* avg = (table*)calloc(size, sizeof(table));
    for (int i=0; i<size; i++){
        avg[i].time = 0;
        avg[i].accX = 0;
        avg[i].accY = 0;
        avg[i].accZ = 0;
        avg[i].pitch= 0;
        avg[i].roll = 0;
        avg[i].yaw  = 0;

    }


     //test
    int i = 0;
    //get avg.
    for(int j= w/2; j < size - w/2; j++){

     //    int special = w/2;
       // < special || > size - special.
        for( int k = -w/2; k < w/2; k++){
           //calculate total in a window.
            avg[i].time += data[j+k].time;
            avg[i].accX += data[j+k].accX;
            avg[i].accY += data[j+k].accY;
            avg[i].accZ += data[j+k].accZ;
            avg[i].pitch+= data[j+k].pitch;
            avg[i].roll += data[j+k].roll;
            avg[i].yaw  += data[j+k].yaw;
        }
            avg[i].time  /= w;
            avg[i].accX  /= w;
            avg[i].accY  /= w;
            avg[i].accZ  /= w;
            avg[i].pitch /= w;
            avg[i].roll  /= w;
            avg[i].yaw   /= w;
            //printf("time %f\n",avg[i].time);
            i++;
    }
    i = 0;

    for(int j= w/2; j < size - w/2; j++){
     //    int special = w/2;
       // < special || > size - special.
        for( int k = -w/2; k < w/2; k++){
            variance[i].time  += SQR (data[j+k].time  - avg[i].time) / (double)(w);
            variance[i].accX  += SQR (data[j+k].accX  - avg[i].accX) / (double)(w) ;
            variance[i].accY  += SQR (data[j+k].accY  - avg[i].accY) / (double)(w);
            variance[i].accZ  += SQR (data[j+k].accZ  - avg[i].accZ) / (double)(w) ;
            variance[i].pitch += SQR (data[j+k].pitch - avg[i].pitch) / (double)(w);
            variance[i].roll  += SQR (data[j+k].roll  - avg[i].roll) / (double)(w) ;
            variance[i].yaw   += SQR (data[j+k].yaw  - avg[i].yaw) / (double)(w);
        }
        //printf("%d : time %f\n",i,variance[i].time  );
        i++;
    }

    free(avg);

    return variance;
}
