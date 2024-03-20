/*****************************************************************************
 * 
 * ****************************************************************************/

/**
 * @file general.h
 * 
 * 本身包含flash存储设置、NTC、SCREEN背光、BT_HOME、SERIAL_OUTPUT
 * UDP_DEBUG
 */
#ifndef __GENERAL_H__
#define __GENERAL_H__

#define ESP32 1
// #define ESP8266 1

//启用NTC测温
#define NTC 1
//启用屏幕亮度调节
#define SCREEN_BL 1

//启用WiFi
#define WIFI 0
#if WIFI

#include "general_wifi.h"
#endif
//启用MQTT
#ifndef MQTT
#define MQTT 1
#endif
#if MQTT
#include "general_mqtt.h"
#endif

//启用OTA
#ifndef OTA
#define OTA 0
#endif
#if OTA
#include "general_ota.h"
#endif

//启用蓝牙输出
#define BT_HOME 0
//启动串口输出
#define SERIAL_OUTPUT 1
//启动串输出log_info函数
#ifndef LOG_ENABLE
#define LOG_ENABLE 1
#endif

//启用flash存储设置
#ifndef USING_FLASH_CONFIG
#define USING_FLASH_CONFIG 0
#endif


//启用udp debug
#ifndef UDP_DEBUG
#define UDP_DEBUG 1
#endif


void setupPins();
void setupSerial();
#ifndef __LOG_H__
#define __LOG_H__


#if LOG_ENABLE
#include <Arduino.h>

#define LOG_NONE    0
#define LOG_ERROR   10
#define LOG_WARNING 20
#define LOG_INFO    30
#define LOG_DEBUG   40

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

#if LOG_LEVEL >= LOG_INFO

#if UDP_DEBUG
#define log_info(...) udpPrintf(__VA_ARGS__)
#else
#define log_info(...) Serial.printf(__VA_ARGS__)
#endif
#else
#define log_info(...)
#endif

#if LOG_LEVEL >= LOG_ERROR

#if UDP_DEBUG
#define log_error(...) udpPrintf(__VA_ARGS__)
#else
#define log_error(...) Serial.printf(__VA_ARGS__)
#endif
#else
#define log_error(...)
#endif

#if LOG_LEVEL >= LOG_WARNING
#if UDP_DEBUG
#define log_warning(...) udpPrintf(__VA_ARGS__)
#else
#define log_warning(...) Serial.printf(__VA_ARGS__)
#endif
#else
#define log_warning(...)
#endif

#if LOG_LEVEL >= LOG_DEBUG
#if UDP_DEBUG
#define log_debug(...) udpPrintf(__VA_ARGS__)
#else
#define log_debug(...) Serial.printf(__VA_ARGS__)
#endif

#else
#define log_debug(...)
#endif

// void log_init(void);

#else /* !LOG_ENABLE */

#define log_info(...)
#define log_warning(...)
#define log_error(...)
#define log_debug(...)
#define log_init()

#endif

#define log_func() log_debug("call %s\r\n", __FUNCTION__)

#if UDP_DEBUG
void udpPrintf(const char *format, ...);
#endif



#endif /* __LOG_H__ */
void turnonLED() ;
void turnoffLED() ;
void flashLed(int , int , int );

#if NTC
#include <math.h> //需要使用标准数学库中函数
/**
 * @brief
 *
 * @param _R1       当前温度下的电阻
 * @param _B        所使用的NTC电阻B值(datasheet里面有,例如3950)
 * @param _R2       T2温度下的电阻
 * @param _T2       一般是25℃
 * @return float    返回的就是当前温度(℃)
 */
// inline float resistanceToTemperature(float _R1, float B, float _R2, float _T2);
float resistanceToTemperatureDefault(float _R1);
float readTempDefault(int8_t ipPort);
#endif

#if SCREEN_BL
void setScreenBrightness(int8_t bright);
#endif


#if USING_FLASH_CONFIG

void loadConfig();
void saveConfig(char* ssid, char* pass, char* deviceName, char* mqttServer) ;
#endif
void advertisePowerMeter(int mvol, int ma, int temp,float w, int wh);


#endif //__GENERAL_H__