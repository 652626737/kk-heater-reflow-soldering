#include "User.h"

/*
  物联网加热台固件
  版本:V1.5
  日期:2022-11-8
  IO配置：EC11.cpp、Buzzer.cpp、IIC使用默认IO、Temp.h、PWM.h
*/

void setup()
{

  delay(100);

#ifdef DEBUG
  setupSerial();
#endif

  oled.begin();

  eeprom.read_all_data();

  Ticker_init();

  miot.begin();

  ec11.begin(ui_key_callb);
  ec11.speed_up(true);
  ec11.speed_up_max(20);

  ui.page_switch_flg = true;
  
}

void loop()
{
  delay(1);
  // if (system_get_cpu_freq() != SYS_CPU_160MHZ)
  //   system_update_cpu_freq(SYS_CPU_160MHZ);
  /*
  //function takes the following frequencies as valid values:
//  240, 160, 80    <<< For all XTAL types
//  40, 20, 10      <<< For 40MHz XTAL
//  26, 13          <<< For 26MHz XTAL
//  24, 12          <<< For 24MHz XTAL
bool setCpuFrequencyMhz(uint32_t cpu_freq_mhz);
  */
  // if (getCpuFrequencyMhz() != )
  ui.run_task();
  eeprom.write_task();
   miot.run_task();
}
