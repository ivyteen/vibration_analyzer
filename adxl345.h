#ifndef __VIBSENSOR_H__
#define __VIBSENSOR_H__

extern int vibSensor_Init(double* outbuf_x, double* outbuf_y, double* outbuf_z, int sample_rate, int sample_time);
extern int vibSensor_ReadData(void);
extern void vibSensor_CleanUp(void);


#endif //__VIBSENSOR_H__