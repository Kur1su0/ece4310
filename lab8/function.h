
//#define DISTANCE 4
//#define ANGLE 20
//#define THRE 130
//#define isCLEAR 1


unsigned char* read_image(unsigned char*, char*);
void save_image(unsigned char*,float*, char* );
void convert_to_coord(double P[3][128*128], unsigned char* range_image);
void cross_product(double p[3][128*128], double cross[3][128*128]);
void cal_region_grow(double cross[3][128*128], unsigned char* labels);
void RegionGrow(double cross[3][128*128],
                unsigned char *labels,	/* segmentation labels */
                int r,int c,		/* pixel to paint from */
                int paint_over_label,	/* image label to paint over */
                int new_label,		/* image label for painting */
                int *indices,		/* output:  indices of pixels painted */
                int *count, double avg[3]);
