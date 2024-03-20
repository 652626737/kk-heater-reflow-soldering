#include "Temp.h"
// #include <Ticker.h>
#include "EC11.h"
#include <Thermocouple.h>
#include <MAX6675_Thermocouple.h>
#include "general.h"

Temp temp;
Ticker adc_max_temp_tic;
    Thermocouple* thermocouple;

inline float getTemp(float vol) {
    float resNTC = 10000 * vol/(3.3 - vol);
    return resistanceToTemperatureDefault(resNTC);
}
Temp::Temp()
{
    // pinMode(temp_platform_io, IN);
    // digitalWrite(switch_io, channel_low_temp);
    thermocouple = new MAX6675_Thermocouple(SCK_PIN, CS_PIN, SO_PIN);
}

void Temp::get()
{
    uint8_t i;
    for (i = 0; i < 7; i++)
        adc_buf[i] = adc_buf[i + 1];
    adc_buf[i] = analogReadMilliVolts(temp_environmental_io);

}

void Temp::get_voltage()
{
    uint8_t i;
    uint32_t mvol = 0;
    for (i = 0; i < 8; i++)
        mvol += adc_buf[i];
    mvol >>= 3;
    vol = float(mvol) / 1000;
}
// 51  150`
void Temp::get_temp_task()
{
    get_voltage();
    environmental_temp = getTemp(vol);
    now_temp_d = thermocouple->readCelsius();
    now_temp = now_temp_d; //获取K型电偶的温度
    // Serial.print("now_temp:");
    // Serial.println(now_temp);
    if (now_temp < 151)
        return;

}

static uint16_t last_temp = 0;

void adc_max_temp_auto_feed()
{
    if (last_temp != temp.now_temp)
    {
        last_temp = temp.now_temp;
    }
    else
    {
        temp.adc_max_temp_auto_flg = 1;
        if (temp.now_temp != 38)
        {
            temp.adc_max_temp = temp.now_temp;
            ec11.int_close();
            EEPROM.begin(eeprom_size);
            EEPROM.write(adc_max_temp_low_add, temp.adc_max_temp & 0xff);
            EEPROM.write(adc_max_temp_high_add, temp.adc_max_temp >> 8);
            EEPROM.commit();
            EEPROM.end();
            ec11.int_work();
        }
        adc_max_temp_tic.detach();
    }
}

bool Temp::adc_max_temp_auto()
{
    return adc_max_temp_auto_flg;
}