#include "general.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <WiFi.h>
// #define WIFI_CONNECT_TIME_MILLI 480000
#define PIN_LED 17
#define SCREEN_BL_PIN 13
#define USE_BTHOME_V1 1

void setupSerial() {
#if SERIAL_OUTPUT
	Serial.begin(115200);
	Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
#if ESP32
  Serial.printf("Chip ID: %X\n", ESP.getEfuseMac());
#endif
#if ESP8266
  Serial.printf("Chip ID: %X\n", ESP.getChipId());
#endif
  
#endif
}

#if UDP_DEBUG
// https://blog.csdn.net/Naisu_kun/article/details/86300456
#include <WiFiUdp.h> //引用以使用UDP
unsigned int localUdpPort = 2333; //本地端口号
WiFiUDP udp;
char udp_send_buff[256];
uint32_t DEFAULT_IP = 0;
uint16_t DEFAULT_PORT = 0;
IPAddress remoteIP(DEFAULT_IP);
uint16_t remotePort = DEFAULT_PORT;


void udpPrintf(const char *format, ...){
  va_list params;
  va_start(params, format);
  //如果wifi未连接或者没有客户端连接到设备，则打印到串口
  if(!WiFi.isConnected() || (int32_t)remoteIP == DEFAULT_IP || DEFAULT_PORT == remotePort) {
    Serial.printf(format, params);
  } else {
    // Serial.printf(format, params);
    udp.beginPacket(remoteIP, remotePort);
    //Udp.print("Received: ");
    udp.printf(format, params);
    // Serial.printf();
    //  udp.write((const uint8_t*)str, strlen(str)); //复制数据到发送缓存
    udp.endPacket();
  }
  va_end(params);
  
}

void udpDebugLoop() {
  if ((int32_t)remoteIP == DEFAULT_IP || DEFAULT_PORT == remotePort) {
    int size = udp.parsePacket();
    
    if (size) {
      char buf[255];
      int size = udp.read(buf, 255);
      if (size>0)
      { 
        buf[size] = 0;
      }
      
      Serial.print(buf);
      if (0 == strcmp("lkk", buf))
      {
        // log_info("origin info:%sremoteport%d\n", remoteIP.toString().c_str(), remotePort);
        // log_info("remote info:%s\nremoteport%d\n", udp.remoteIP().toString().c_str(), udp.remotePort());
        Serial.print("origin info:");
        Serial.print(remoteIP);
        Serial.print("remoteport:");
        Serial.println(remotePort);
        bool isSuc = remoteIP.fromString(udp.remoteIP().toString().c_str());
        Serial.print("remote info:");
        Serial.print(udp.remoteIP());
        Serial.println();
        Serial.print(isSuc);
        Serial.println();
        Serial.print("remoteport:");
        Serial.println(udp.remotePort());
        remotePort = udp.remotePort();

        Serial.print("origin info:");
        Serial.print(remoteIP);
        Serial.print("remoteport:");
        Serial.println(remotePort);
        udp.println("Connected!");
      }
      udp.endPacket();
    }
  }else {
    log_info("test%d",remotePort);
  }
}
#endif

void setupPins() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(SCREEN_BL_PIN, OUTPUT);
}
void turnonLED() {
  digitalWrite(PIN_LED, HIGH);
}
void turnoffLED() {
  digitalWrite(PIN_LED, LOW);
}
void flashLed(int times, int highInterval, int lowInterval) {

    for (int i = 0; i < times; i++)
    {
      digitalWrite(PIN_LED, HIGH);
      delay(highInterval);
      digitalWrite(PIN_LED, LOW);
      if (lowInterval > 0)
      {
        delay(lowInterval);
      }
      
      
    }
    
}


#if NTC
/**
 * @brief
 *
 * @param _R1       当前温度下的电阻
 * @param _B        所使用的NTC电阻B值(datasheet里面有,例如3950)
 * @param _R2       T2温度下的电阻
 * @param _T2       一般是25℃
 * @return float    返回的就是当前温度(℃)
 */
float resistanceToTemperature(float _R1, float B, float _R2, float _T2 = 25)
{
    return (1.0 / ((1.0 / B) * log(_R1 / _R2) + (1.0 / (_T2 + 273.15))) - 273.15);
}
#define NTC_B 3950
#define NTC_Rp 10000
#define NTC_RES_PULLUP 10000

float resistanceToTemperatureDefault(float _R1)
{
    return (1.0 / ((1.0 / NTC_B) * log(_R1 / NTC_Rp) + (1.0 / (25 + 273.15))) - 273.15);
}
/**
 * 10K分压电阻，10K NTC
*/
float readTempDefault(int8_t ipPort) {
  uint8_t i;
  uint32_t mvol = 0;
  for (i = 0; i < 4; i++)
    mvol += analogReadMilliVolts(ipPort);
  mvol >>= 2;
  // vol_low = vol_low * 1000 / 1024;
  float vol = float(mvol) / 1000;
  float resNTC = NTC_RES_PULLUP * vol/(3.3 - vol);
  return resistanceToTemperatureDefault(resNTC);
  
}
#endif

#if SCREEN_BL
void setScreenBrightness(int8_t bright) {
  digitalWrite(SCREEN_BL_PIN, (bright == 0) ? LOW : HIGH);
}
#endif

#if BT_HOME
//Use lib: h2zero/NimBLE-Arduino@^1.4.0
#include "NimBLEDevice.h"
#include "NimBLEBeacon.h"
// #include "esp_sleep.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
BLEAdvertising *pAdvertising;
/**
 * 根据BT规范，消息由3部分组成：header+payload+rssi
    header：消息长度，mac，advertising payload（AD）长度
    payload：1个或者多个AD，
    AD：长度，type，数据。至少要包含flags和servicedata（16位UUID），也可以添加local name
    BTHome的data包含：16位uuid，设备信息和测量值
HEADER                           AD 
043E 消息长度 02010000 Mac     AD长度   Flags  长度+local name           长度+16UUID  UUID    V2设备信息   V1的数据前缀 packetID    sensor数据      
043E 27 02010000 A5808FE64854 1B      020106 0B094449592D73656E736F72 0B16         1C18                23                     02 C409    03  03    BF13  CC    V1
043E 26 02010000 A5808FE64854 1A      020106 0B094449592D73656E736F72 0A16         FCD2    40                      0009       02 C409        03    BF13  CC    V2
043E 19 02010000 A5808FE64854 0D      020106                          0916         1C18                            020009                02  01    61    CC    V1
Flags：020106=
Flags data: 020106
0x02 = length (2 bytes)
0x01 = Flags
0x06 = in bits, this is 00000110.
bit 1 and bit 2 are 1, meaning:
bit 1: “LE General Discoverable Mode”
bit 2: “BR/EDR Not Supported”
  */

#define BEACON_UUID "4c65931d-a9ba-4462-976a-6b971acebcf7" // UUID 1 128-Bit (may use linux tool uuidgen or random numbers via https://www.uuidgenerator.net/)

/**
 * 
 * mvol   mV
 * power  0.01w
 * temp   0.01°C
 * mwh    0.001 KwH
 */
void setBeaconV1(int mvol, int power, int temp, int wh, uint8_t packetId) {

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  
  oAdvertisementData.setFlags(0x06); // this is 00000110. Bit 1 and bit 2 are 1, meaning: Bit 1: “LE General Discoverable Mode” Bit 2: “BR/EDR Not Supported”
  
  std::string strServiceData = "";
  std::string strServiceData2 = "";
  strServiceData += (char)11;     // Length
  strServiceData += (char)0x09;   // Type = Complete local name
  // String device_name = String("power_king"); //needs to only be 10 characters long
  char str_buf[11]; // Actually 11 in size as its 0 indexed
  // device_name.toCharArray(str_buf,11); // Length is 11 here as we need to capture the null terminator
  sprintf(str_buf, "power_king");
  strServiceData += str_buf;


//  uint8_t length = packetId < 0 ? 21 : 24;
  strServiceData2 += (char)15;     // Length
  strServiceData2 += 0x16;      // Type = Service Data - 16-bit UUID

  strServiceData2 += 0x1C;    // Use 0x1C18 for non-encrypted messages and 0x1E18 for encrypted data.
  strServiceData2 += 0x18;
  
  //13字节，See https://bthome.io/ 
  
//   if (packetId >= 0) {
//    // Count packet
//    strServiceData2 += (char)0x02;  
//    strServiceData2 += (char)0x00;

//    // Count value
//    strServiceData2 += (char)(packetId & 0xFF);  // bootcount lower byte
//  }

//   电压 packet,0.001V
  strServiceData2 += 0x03;
  strServiceData2 += 0x0C;

  // 电压 Value (little endian)
  strServiceData2 += (char)(mvol & 0xFF);  // bootcount lower byte
  strServiceData2 += (char)((mvol & 0xFF00)>>8); // bootcount upper byte
//
//  // 功率 packet,0.01W 这里如果是04（5个字节）不成功。。。
  strServiceData2 += 0x03;
  strServiceData2 += 0x0B;
  strServiceData2 += (char)(power & 0xFF);  // bootcount lower byte
  strServiceData2 += (char)((power & 0xFF00)>>8); // bootcount upper byte
  // strServiceData2 += (char)((power & 0xFF0000)>>16); // bootcount upper byte
//  strServiceData2 += 0x01;
  // 功率 Value  
//  strServiceData2 += (char)(power & 0xFF);  //  lower byte
//  strServiceData2 += (char)(0x11); //  middle byte
//  strServiceData2 += (char)(0x00); //  upper byte

  // Temperature packet,2位小数温度
  strServiceData2 += 0x23;
  strServiceData2 += 0x02;
  // Temperature Value (little endian)
  strServiceData2 += (char)(temp & 0xFF);  // bootcount lower byte
  strServiceData2 += (char)((temp & 0xFF00)>>8); // bootcount upper byte

  // 电量 packet,0.01 KwH
//  strServiceData2 += 0x04;
//  strServiceData2 += 0x0A;
//  // 电量 Value (little endian)
//  strServiceData2 += (char)(wh & 0xFF);  //  lower byte
//  strServiceData2 += (char)((wh & 0xFF00)>>8); //  middle byte
//  strServiceData2 += (char)((wh & 0xFF0000)>>16); //  upper byte



  oAdvertisementData.addData(strServiceData);
  oAdvertisementData.addData(strServiceData2);
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
  /**  pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
  *    Advertising mode. Can be one of following constants:
  *  - BLE_GAP_CONN_MODE_NON (non-connectable; 3.C.9.3.2).
  *  - BLE_GAP_CONN_MODE_DIR (directed-connectable; 3.C.9.3.3).
  *  - BLE_GAP_CONN_MODE_UND (undirected-connectable; 3.C.9.3.4).
  */
  pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);

}
/**
 * 
 * mvol   mV
 * current  1 mA
 * temp   0.1°C
 * mwh    0.001 KwH
 */
void setBeaconV2(int mvol, int current, int temp, int wh, int packetId) {

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  
  oAdvertisementData.setFlags(0x06); // this is 00000110. Bit 1 and bit 2 are 1, meaning: Bit 1: “LE General Discoverable Mode” Bit 2: “BR/EDR Not Supported”
  
  std::string strServiceData = "";
  std::string strServiceData2 = "";
  strServiceData += (char)11;     // Length
  strServiceData += (char)0x09;   // Type = Complete local name
  String device_name = "power_king"; //needs to only be 10 characters long
  char str_buf[11]; // Actually 11 in size as its 0 indexed
  // device_name.toCharArray(str_buf,11); // Length is 11 here as we need to capture the null terminator
  sprintf(str_buf, "power_king");
  strServiceData += str_buf;


  int length = packetId < 0 ? 17 : 19;
  strServiceData2 += (char)length;     // Length
  strServiceData2 += 0x16;      // Type = Service Data - 16-bit UUID

  strServiceData2 += 0xFC;    // 0xFCD2-公共的UUID
  strServiceData2 += 0xD2;

  strServiceData2 += 0x40;    //The first byte after the UUID 0x40 is the BTHome device info byte, which has several bits indicating the capabilities of the device.
  
  //13字节，See https://bthome.io/

  // 电压 packet,0.001V
  strServiceData2 += 0x0C;

  // 电压 Value (little endian)
  strServiceData2 += (char)(mvol & 0xFF);  // bootcount lower byte
  strServiceData2 += (char)((mvol & 0xFF00)>>8); // bootcount upper byte

  // 电流 packet,0.001A
  strServiceData2 += 0x43;
  // 电流 Value (little endian)
  strServiceData2 += (char)(current & 0xFF);  // bootcount lower byte
  strServiceData2 += (char)((current & 0xFF00)>>8); // bootcount upper byte

  // Temperature packet,1位小数温度
  strServiceData2 += 0x45;
  // Temperature Value (little endian)
  strServiceData2 += (char)(temp & 0xFF);  // bootcount lower byte
  strServiceData2 += (char)((temp & 0xFF00)>>8); // bootcount upper byte

  // 电量 energy packet,0.001kWh
  strServiceData2 += 0x0A;
  // 电量 Value (little endian)
  strServiceData2 += (char)(wh & 0xFF);  //  lower byte
  strServiceData2 += (char)((wh & 0xFF00)>>8); //  middle byte
  strServiceData2 += (char)((wh & 0xFF0000)>>16); //  upper byte

  if (packetId >=0) {
    // packet id
    strServiceData2 += (char)0x00;

    // packet id value
    strServiceData2 += (char)(packetId & 0xFF);  // bootcount lower byte
  }

  oAdvertisementData.addData(strServiceData);
  oAdvertisementData.addData(strServiceData2);
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
  /**  pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
  *    Advertising mode. Can be one of following constants:
  *  - BLE_GAP_CONN_MODE_NON (non-connectable; 3.C.9.3.2).
  *  - BLE_GAP_CONN_MODE_DIR (directed-connectable; 3.C.9.3.3).
  *  - BLE_GAP_CONN_MODE_UND (undirected-connectable; 3.C.9.3.4).
  */
  pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);

}

uint8_t packetId = 0;
/**
 *
 * mvol:
 * current:毫安
 * temp:1位小数的温度 
 * wh:
 * 
 */
void advertisePowerMeter(int mvol, int ma, int temp,float w, int wh) {
  // Create the BLE Device
  BLEDevice::init("meter");

  pAdvertising = BLEDevice::getAdvertising();
#if USE_BTHOME_V1
  setBeaconV1(mvol, w * 100, temp * 100, wh, packetId++);
#else
  setBeaconV2(mvol, ma, temp, wh, -1);
#endif
   // Start advertising
  pAdvertising->start();
  Serial.println("Advertizing started...");
  delay(100);
  pAdvertising->stop();
}
void advertise() {
  // Create the BLE Device
  BLEDevice::init("");

  pAdvertising = BLEDevice::getAdvertising();
  
  setBeaconV1(11,22,33,44, -1);
   // Start advertising
  pAdvertising->start();
  Serial.println("Advertizing started...");
  delay(100);
  pAdvertising->stop();
}
#endif

