#ifndef MAIN_DEFINE__H_

#define _DSP_ADC_SCALE_    (3.3f/4095.0f)
#define _REF165_    1.65

#define do_TOGGLE(A)   A = (A!=0)? 0 : 1;
#define do_LPF_GAMMA(y, x, GAMMA)    y = GAMMA * y + (1.0f - GAMMA) * (x)
#define _GAMMA_11p52Ksample_10HzLPF     0.994560853    // = Exp(-6.283*10./11.52e3), 11.52KHz sample
#define _GAMMA_1ksample_10HzLPF         0.939103108    // = Exp(-6.283*10./1.e3), 1.0KHz sample
#define _GAMMA_20ksample_10HzLPF         0.996863429    // = Exp(-6.283*10./20e3)
#define _GAMMA_50ksample_10HzLPF         0.998744273    // = Exp(-6.283*10./50e3)
#define _GAMMA_1ksample_1HzLPF         0.993736697    // = Exp(-6.283*1/1e3)
#endif // MAIN_DEFINE__H_


