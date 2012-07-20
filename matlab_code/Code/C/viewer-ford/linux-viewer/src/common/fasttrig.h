#ifndef _FASTTRIG_H
#define _FASTTRIG_H

void fasttrig_init();
void fasttrig_sincos(double theta, double *s, double *c);
double fasttrig_atan2(double y, double x);

#endif
