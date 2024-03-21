#ifndef _PWM_H_
#define _PWM_H_

#include <Arduino.h>
#include <Ticker.h>
// #include "ADC.h"
#include "Temp.h"
#include "Tick_IRQ.h"

#define PWM_IO 27
#define FAN_IO 32


#define kp 8.5
#define ki 1.4
#define ki_high 1.4
#define kd 27.0


#define OFF 0
#define ON 1

#define Re_So 0
#define Co_Temp 1





class PWM{
    public:
    PWM();
    void begin();
    void end();
    void temp_set();
    void fan();
    bool power = OFF;
    bool sleep_flg = OFF;
    
    int16_t temp_buf = 0;//设置温度

    uint8_t temp_mode = 0; //0 回流 1：恒温

    int16_t temp_mode1_time = 0;//恒温时间

    uint8_t percent = 0;

    bool temp_reached_flg = false;

    uint8_t backflow_working_state = 0;
    // int16_t backflow_temp_buf = 0;
    int16_t backflow_temp_tmp = 0;

    

    
    private:

    bool fan_state = OFF;

    int16_t need_set_temp = 50;
    
    uint8_t high_time = 0;

    int16_t pwm_buf = 0;
    float pwm_buf_f = 0;
    friend void pwm_irq();
    


    
};



extern PWM pwm;



#endif