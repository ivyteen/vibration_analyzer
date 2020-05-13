#ifndef __ADXL345_H__
#define __ADXL345_H__

typedef struct sensor_dev_t {
    double* rx;
    double* ry;
    double* rz;
    double* rt;

    double* outbuf_x;
    double* outbuf_y;
    double* outbuf_z;

    int handler;

    int bus_num;
    int cs_num;

    int sample_rate;
    int sample_time;

    int samples;
    int samples_max_spi;

    double delay;
} SENSOR_DEV_t;


#define DATA_FORMAT   0x31  // data format register address
#define DATA_FORMAT_B 0x0B  // data format bytes: +/- 16g range, 13-bit resolution (p. 26 of ADXL345 datasheet)
#define READ_BIT      0x80
#define MULTI_BIT     0x40
#define BW_RATE       0x2C
#define POWER_CTL     0x2D
#define DATAX0        0x32

#define OFS_X       0x1E
#define OFS_Y       0x1F
#define OFS_Z       0x20




extern int vibSensor_PiGpioInit(void);
extern void vibSensor_PiGpioCleanUp(void);
extern int vibSensor_SpiInit(SENSOR_DEV_t* dev);

extern int vibSensor_ReadData(SENSOR_DEV_t* dev);
extern void vibSensor_SpiCleanUp(SENSOR_DEV_t* dev);


#endif //__VIBSENSOR_H__