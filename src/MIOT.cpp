/**
 * 不使用blinker，使用本地MQTT
*/
#include "MIOT.h"
// #include "Blinker.h"
#include "general.h"

#define BLINKER_CMD_ON 1
#define BLINKER_CMD_OFF 0

MIOT miot;

char power_topic[60] = "state";
char temp_mode_topic[60] = "temp_mode";
char temp_buf_topic[60] = "temp_buf";
char temp_env_topic[60] = "temp_env";
char temp_now_topic[60] = "temp_now";

bool power = -1;
bool fan = OFF;//风扇状态

int16_t temp_buf = 0;//设置温度
double temp_env = 0;//当前环境温度
double temp_now = 0;//当前温度

uint8_t temp_mode = -1; //0 回流 1：恒温，初始-1保证启动时发送状态

// BlinkerButton Button1(BUTTON_1);
// BlinkerButton Button2(BUTTON_2);
// BlinkerButton Button3(BUTTON_3);
// BlinkerButton Button4(BUTTON_4);
// BlinkerButton Button5(BUTTON_5);
// BlinkerSlider Slider1(Slider_1);

void button1_callback(int &state) //按键事件
{
    if (!pwm.power)
    {
        pwm.begin();
        ui.oled_display_set();
        power = ON;
    }
}

void button2_callback(int &state) //按键事件
{
    if (pwm.power)
    {
        pwm.end();
        ui.oled_display_set();
        power = OFF;
    }
}
void button3_callback(int &state) //按键事件
{
    if (!pwm.temp_mode)
    {
        pwm.temp_mode = 1;
        ui.temp_mode_flg = 1;
        if (pwm.power)
        {
            pwm.end();
            eeprom.write_flg = 1;
            eeprom.write_t = 0;
        }
        ui.oled_display_set();
        temp_mode =1;
    }
}

void button4_callback(int &state) //按键事件
{
    if (pwm.temp_mode)
    {
        pwm.temp_mode = 0;
        ui.temp_mode_flg = 1;
        if (pwm.power)
        {
            pwm.end();
            eeprom.write_flg = 1;
            eeprom.write_t = 0;
        }
        ui.oled_display_set();
        temp_mode =0;
    }
}

void button5_callback(int &state) //按键事件
{
    pwm.fan();
}

void slider1_callback(int32_t value) //滑块处理
{
    Serial.printf("slider1_callback,value:%d, pwm.temp_mode:%d\n", value, pwm.temp_mode);
    if (pwm.temp_mode)
    {
        if (value < 40)
            value = 40;
        else if (value > 250)
            value = 250;
        pwm.temp_buf = value;

        ui.temp_move_flg = 1;
        eeprom.write_t = 0;
        eeprom.write_flg = 1;
        ui.show_temp_mode = show_set_temp;
        ui.oled_display_set();
        temp_buf = value;
    }
    else 
    {
        temp_buf = 0;//发送一次温度
    }
}

void miotPowerState(int &state) //电源事件
{

    if (state == BLINKER_CMD_ON)
    {
        if (!pwm.power)
        {
            pwm.begin();
            ui.oled_display_set();
        }
    }
    else if (state == BLINKER_CMD_OFF)
    {
        if (pwm.power)
        {
            pwm.end();
            ui.oled_display_set();
        }
    }
}

void miotColor(int32_t color)
{
    if (pwm.temp_mode)
    {
        color &= 0xffffff;

        switch (color)
        {
        case 9498256:
            pwm.temp_buf = qianlvse;
            break;
        case 16711935:
            pwm.temp_buf = zihongse;
            break;
        case 16761035:
            pwm.temp_buf = fenhongse;
            break;
        case 16738740:
            pwm.temp_buf = fense;
            break;
        case 10494192:
            pwm.temp_buf = zise;
            break;
        case 8900331:
            pwm.temp_buf = tianlanse;
            break;
        case 255:
            pwm.temp_buf = lanse;
            break;
        case 65535:
            pwm.temp_buf = qingse;
            break;
        case 65280:
            pwm.temp_buf = lvse;
            break;
        case 16753920:
            pwm.temp_buf = juhuangse;
            break;
        case 16776960:
            pwm.temp_buf = huangse;
            break;
        case 16711680:
            pwm.temp_buf = hongse;
            break;
        }
        ui.temp_move_flg = 1;
        eeprom.write_t = 0;
        eeprom.write_flg = 1;
        ui.show_temp_mode = show_set_temp;
        ui.oled_display_set();
    }
}

void miotMode(uint8_t mode)
{

    // if (mode == BLINKER_CMD_MIOT_DAY)
    // { //日光模式
    //     if (pwm.temp_mode)
    //     {
    //         pwm.temp_buf = MIOT_DAY;
    //         ui.temp_move_flg = 1;
    //         ui.show_temp_mode = show_set_temp;
    //         eeprom.write_t = 0;
    //         eeprom.write_flg = 1;
    //         ui.oled_display_set();
    //     }
    // }
    // else if (mode == BLINKER_CMD_MIOT_NIGHT)
    // { //夜光模式
    //     if (pwm.temp_mode)
    //     {
    //         pwm.temp_mode = 0;
    //         ui.temp_mode_flg = 1;
    //         ui.oled_display_set();
    //     }
    // }
    // else if (mode == BLINKER_CMD_MIOT_COLOR)
    // { //彩光模式
    //     if (!pwm.temp_mode)
    //     {
    //         pwm.temp_mode = 1;
    //         ui.temp_mode_flg = 1;
    //         ui.oled_display_set();
    //     }
    // }
    // else if (mode == BLINKER_CMD_MIOT_WARMTH)
    // { //温馨模式
    // }
    // else if (mode == BLINKER_CMD_MIOT_TV)
    // { //电视模式
    // }
    // else if (mode == BLINKER_CMD_MIOT_READING)
    // { //阅读模式
    // }
    // else if (mode == BLINKER_CMD_MIOT_COMPUTER)
    // { //电脑模式
    // }
}
void mqttCommand(int cmd, int param) {
    Serial.printf("mqttCommand,cmd:%d, para:%d\n", cmd, param);
    switch (cmd)
    {
    case 1://开始加热
        button1_callback(param);
        break;
    case 2://停止加热
        button2_callback(param);
        break;
    case 3://恒温模式
        button3_callback(param);
        break;
    case 4://回流模式
        button4_callback(param);
        break;
    case 5://风扇
        button5_callback(param);
        break;
    case 6://温度
        slider1_callback(param);
        break;
    
    default:
        break;
    }
}

void sendHADiscovery() {

  char topic[64];
  createTopic(topic, temp_buf_topic);
  char macAddress[13];
#if ESP32
  sprintf(macAddress, "%012X", ESP.getEfuseMac());
#elif ESP8266
  sprintf(macAddress, "%012X", ESP.getChipId());
#endif
  createDiscovery("switch", topic_attr,
                topic_state,"加热", "switch",
                topic_availability,"switch", "{{}}",
                "{\"cmd\":1}", "{\"cmd\":0}", NULL,
                NULL,
                NULL, NULL, NULL, topic_command,
                "加热板", "KK","KK-Heater","1.1",clientId,
                macAddress,
                true,
                "mdi:soldering-iron",
                "1",
                "0" ,
                NULL,
                NULL,
                0,0,NULL
                );
  createTopic(topic, temp_now_topic);
  createDiscovery("sensor",topic_attr,
                topic,"当前温度", "temp_platform",
                topic_availability, "temperature", "{{value}}",
                NULL, NULL, "℃",
                NULL,
                NULL, NULL, NULL, NULL,
                "加热板", "KK","KK-Heater", "1.1", clientId, 
                macAddress,
                true ,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                0,0,NULL);

  createTopic(topic, temp_env_topic);
  createDiscovery("sensor",topic_attr,
                topic,"环境温度", "temp_environment",
                topic_availability, "temperature", "{{ value}}",
                NULL, NULL, "℃",
                NULL,
                NULL, NULL, NULL, NULL,
                "加热板", "KK","KK-Heater", "1.1", clientId, 
                macAddress,
                true ,
                NULL,
                NULL,
                NULL  ,NULL,NULL ,0,0,NULL);
  createTopic(topic, temp_mode_topic);
  createDiscovery("select",topic_attr,
                topic,"加热模式", "temp_mode",
                topic_availability, NULL, " >\n\
            {% set mapper = { 0:'回流', 1:'恒温'} %}\n\
            {% set digit = value_json %}\n\
            {% set word = mapper[digit] %}\n\
            {{ word }}",
                NULL, NULL, "W",
                NULL,
                NULL, NULL, NULL,topic_command,
                "加热板", "KK","KK-Heater", "1.1", clientId, 
                macAddress,
                true,
                NULL,
                NULL, NULL,
                "{% if value == \"回流\" %}\n\
                    {\"cmd\":4}\n\
                {% else %}\n\
                    {\"cmd\":3}\n\
                {% endif %}" ,
                "[\"回流\",\"恒温\"]"
                ,0,0,NULL);

  createTopic(topic, temp_buf_topic);
  createDiscovery("number",topic_attr,
                topic,"温度设定", "temp_buf",
                topic_availability, "temperature", "{{ value }}",
                NULL, NULL, "℃",
                NULL,
                NULL, NULL, NULL, NULL,
                "功率计", "KK","KK-Heater", "1.1", clientId, 
                macAddress,
                true,
                NULL,
                NULL, NULL,
                ">\n\
                {\"cmd\":6, \"para\": {{value}} }" ,
                NULL,
                250,20,NULL );

}

void MIOT::begin()
{

    if (miot_able)
    {
        open_flg = 1;
        if (WiFi.status() != WL_CONNECTED)
        {
            wifi_conect_flg = setwifi.power_on_conect();
            if (!wifi_conect_flg)
            {
                Serial.println("connect wifi error!");
                return;
            }
            Serial.println("connect wifi ok!");
        }
        // setupMQTT();
        setupMQTTWithConnectedCallback(sendHADiscovery);
        setupMqttCallback(&mqttCommand);
    }
    else
        return;

        

    // if (strlen(&wifima.blinker_id[0]) == 12)
    // {
    //     Serial.println("blinker start!");
    //     Blinker.begin(&wifima.blinker_id[0],(const char*)&WiFi.SSID()[0],(const char*)&WiFi.psk()[0]);
    // }
    // else
    // {
    //     Serial.println("blinker id error!");
    //     return;
    // }
        

// #ifdef DEBUG
//     {
//         BLINKER_DEBUG.stream(Serial);
//     }
// #else
//     {
//         const char *p = &wifima.blinker_id[0];
//         Blinker.begin(p);
//     }
// #endif

    // Button1.attach(button1_callback);
    // Button2.attach(button2_callback);
    // Button3.attach(button3_callback);
    // Button4.attach(button4_callback);
    // Button5.attach(button5_callback);
    // BlinkerMIOT.attachPowerState(miotPowerState);
    // BlinkerMIOT.attachColor(miotColor);
    // Slider1.attach(slider1_callback);
    // BlinkerMIOT.attachMode(miotMode);
}


void MIOT::run_task()
{
    if (miot_able && wifi_conect_flg)
    {
        // Blinker.run();
        mqttLoop();
        if (pwm.power != power) {
            char payload[4];
            sprintf(payload, "%d", (pwm.power) ? 1 : 0);

            if (publishMQTT(power_topic, payload))
                power = pwm.power;
        }
        if (pwm.temp_mode != temp_mode)
        {
            char payload[4];
            sprintf(payload, "%d", pwm.temp_mode);
            if (publishMQTT(temp_mode_topic, payload))
                temp_mode = pwm.temp_mode;
            
        }
        if (pwm.temp_buf != temp_buf)
        {
            char payload[4];
            sprintf(payload, "%d", pwm.temp_buf);
            if (publishMQTT(temp_buf_topic, payload) )
                temp_buf = pwm.temp_buf;
        }
        if (temp.environmental_temp != temp_env)
        {
            char payload[4];
            sprintf(payload, "%f", temp.environmental_temp);
            if (publishMQTT(temp_env_topic, payload) )
                temp_env = temp.environmental_temp;
        }
        if (temp.now_temp_d != temp_now)
        {
            char payload[4];
            sprintf(payload, "%f", temp.now_temp_d);
            if (publishMQTT(temp_now_topic, payload) )
                temp_now = temp.now_temp_d;
        }
        
        
    }
}
