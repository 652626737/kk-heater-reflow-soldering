#include "general_mqtt.h"
#include <Arduino.h>

#if MQTT


#include <WiFi.h>
#include <PubSubClient.h>//MQTT
#include <ArduinoJson.h>
WiFiClient espClient;// 定义wifiClient实例
PubSubClient mqttClient(espClient);


void(*mqttCallback)(int,int) ;
void(*mqttConnectedCallBack)(void) ;

#define CLIENT_ID "%s-%06X"
//boolean saveConfigToFile = false;
int restarted = 0;

char topic_availability[64];
char topic_command[64];
char topic_order[64];
char topic_version[64];
char topic_log[64];
char topic_attr[64];
char topic_state[64];
char device_name[20] = "kk-heater";

char clientId[50]; 
char mqtt_server[20] = "ha.cn";

/**
 * 根据统一的前缀组装topic
*/
void createTopic(char* topic, char *topic_suffix) {
  strcpy(topic, TOPIC_PREFIX);
  strcat(topic, clientId);
  strcat(topic, "/");
  strcat(topic, topic_suffix);
}

// <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
#define HA_DESCOVERY_TOPIC "homeassistant/%s/%s-%s/config"

#define HA_SENSOR_PAYLOAD "{\"name\": \"%s\", \
\"device_class\": \"%s\", \
\"unique_id\": \"%s-%s\", \
\"availability_topic\": \"%s\", \
\"json_attributes_topic\": \"%s\", \
\"unit_of_measurement\": \"%s\", \
\"device\": {\"name\": \"%s\",\"model\": \"%s\",\"manufacturer\": \"%s\",\"sw_version\": \"%s\",\"identifiers\": [\"%s\"]}, \
\"state_topic\": \"%s\",\
\"value_template\": \"%s\"}"

#define HA_SWITCH_PAYLOAD "{\"name\": \"%s\", \
\"icon\":\"%s\",\
\"device_class\": \"%s\", \
\"unique_id\": \"%s-%s\", \
\"availability_topic\": \"%s\", \
\"json_attributes_topic\": \"%s\", \
\"device\": {\"name\": \"%s\",\"model\": \"%s\",\"manufacturer\": \"%s\",\"sw_version\": \"%s\",\"identifiers\": [\"%s\"]}, \
\"state_topic\": \"%s\",\
\"state_off\": \"%s\",\
\"state_on\": \"%s\", \
\"command_topic\": \"%s\",\
\"payload_off\": \"%s\",\
\"payload_on\": \"%s\"}"
#define HA_SELECT_PAYLOAD "{\"name\": \"%s\", \
\"icon\":\"%s\",\
\"unique_id\": \"%s-%s\", \
\"availability_topic\": \"%s\", \
\"json_attributes_topic\": \"%s\", \
\"device\": {\"name\": \"%s\",\"model\": \"%s\",\"manufacturer\": \"%s\",\"sw_version\": \"%s\",\"identifiers\": [\"%s\"]}, \
\"state_topic\": \"%s\",\
\"value_template\": \"%s\",\
\"command_topic\": \"%s\",\
\"command_template\": \"%s\"\
\"options\": \"%s\"}"
#define HA_NUMBER_PAYLOAD "{\"name\": \"%s\", \
\"icon\":\"%s\",\
\"unique_id\": \"%s-%s\", \
\"availability_topic\": \"%s\", \
\"json_attributes_topic\": \"%s\", \
\"device\": {\"name\": \"%s\",\"model\": \"%s\",\"manufacturer\": \"%s\",\"sw_version\": \"%s\",\"identifiers\": [\"%s\"]}, \
\"state_topic\": \"%s\",\
\"value_template\": \"%s\",\
\"command_topic\": \"%s\",\
\"command_template\": \"%s\"\
\"max\": \"%f\"\
\"min\": \"%f\"\
\"payload_reset\": \"%s\", \
\"unit_of_measurement\": \"%s\"}"

void setupTopics() {
#if ESP32
  sprintf(clientId, CLIENT_ID, device_name, ESP.getEfuseMac());
#endif
#if ESP8266
  sprintf(clientId, CLIENT_ID, device_name, ESP.getChipId());
#endif

  strcpy(topic_availability, TOPIC_PREFIX);
  strcat(topic_availability, clientId);
  strcat(topic_availability, TOPIC_SUFFIX_AVAILABILITY);
  
  strcpy(topic_command, TOPIC_PREFIX);
  strcat(topic_command, clientId);
  strcat(topic_command, TOPIC_SUFFIX_COMMAND);
  
  strcpy(topic_order, TOPIC_PREFIX);
  strcat(topic_order, clientId);
  strcat(topic_order, "/order");
  
  strcpy(topic_version, TOPIC_PREFIX);
  strcat(topic_version, clientId);
  strcat(topic_version, TOPIC_SUFFIX_VERSION);
  
  strcpy(topic_log, TOPIC_PREFIX);
  strcat(topic_log, clientId);
  strcat(topic_log, TOPIC_SUFFIX_LOG);
  
  strcpy(topic_attr, TOPIC_PREFIX);
  strcat(topic_attr, clientId);
  strcat(topic_attr, "/attrs");

  strcpy(topic_state, TOPIC_PREFIX);
  strcat(topic_state, clientId);
  strcat(topic_state, "/state");
}

/**
 * 
 * eg:
 * 
{\"name\": \"电量\", \
\"device_class\": \"energy\", \
\"unique_id\": \"sensor.%s.energy_wh\", \
\"availability_topic\": \"%s\", \
\"json_attributes_topic\": \"%s\", \
\"unit_of_measurement\": \"Wh\", \
\"device\": {\"name\": \"功率计\",\"model\": \"PowerKing\",\"manufacturer\": \"KK\",\"sw_version\": \"1.1\",\"identifiers\": [\"%s\"]}}\", \
\"state_topic\": \"%s\",\
\"value_template\": \"{{ value_json.wh }}\"}"

 * @param sensor_type: sensor/switch/binary_sensor等等
 * @param unique_id_surfix: unique_id={device_identifier}-{unique_id_surfix}
 * 
*/
void createDiscovery(char* sensor_type, char* json_attributes_topic,
                            char* state_topic, char* s_name, char* unique_id_surfix,
                            char* availability_topic, char* device_class, char* value_template,
                            char* payload_on, char* payload_off, char* unit_of_meas,
                            int off_delay,
                            char* payload_available, char* payload_not_avalaible, bool gateway_entity, char* command_topic,
                            char* device_name_display, char* device_manufacturer, char* device_model, char* sw_version, char* device_identifier, 
                            char* device_mac, 
                            bool retainCmd,
                            char* icon,
                            char* state_on, char* state_off,
                            char* command_teplate,
                            char* options,
                            float max,
                            float min,
                            char* payload_reset) {
    char topic_config[100];
    char payload_config[500];
    sprintf(topic_config, HA_DESCOVERY_TOPIC, sensor_type, clientId,unique_id_surfix);
    if (0 == strcmp(sensor_type, "sensor"))
    {
      sprintf(payload_config, HA_SENSOR_PAYLOAD, s_name, 
                                                device_class, 
                                                device_identifier, unique_id_surfix, 
                                                availability_topic,
                                                json_attributes_topic,
                                                unit_of_meas,
                                                device_name_display, device_model, device_manufacturer, sw_version, device_identifier,
                                                state_topic,
                                                value_template);
    } else if (0 == strcmp(sensor_type, "switch")) {
      sprintf(payload_config, HA_SWITCH_PAYLOAD, s_name, 
                                                icon,
                                                device_class, 
                                                device_identifier, unique_id_surfix, 
                                                availability_topic,
                                                json_attributes_topic,
                                                device_name_display, device_model, device_manufacturer, sw_version, device_identifier,
                                                state_topic,
                                                state_off, 
                                                state_on,
                                                command_topic,
                                                payload_off,
                                                payload_on);
    } else if (0 == strcmp(sensor_type, "select")) {
      sprintf(payload_config, HA_SELECT_PAYLOAD, s_name, 
                                                icon,
                                                device_identifier, unique_id_surfix, 
                                                availability_topic,
                                                json_attributes_topic,
                                                device_name_display, device_model, device_manufacturer, sw_version, device_identifier,
                                                state_topic,
                                                value_template, 
                                                command_topic,
                                                command_teplate,
                                                options);
    }else if (0 == strcmp(sensor_type, "number")) {
      sprintf(payload_config, HA_NUMBER_PAYLOAD, s_name, 
                                                icon,
                                                device_identifier, unique_id_surfix, 
                                                availability_topic,
                                                json_attributes_topic,
                                                device_name_display, device_model, device_manufacturer, sw_version, device_identifier,
                                                state_topic,
                                                value_template, 
                                                command_topic,
                                                command_teplate,
                                                max,
                                                min,
                                                payload_reset,
                                                unit_of_meas);
    }
    
    mqttClient.setBufferSize(1000);
    boolean ret = mqttClient.publish(topic_config, payload_config, true);
    Serial.println("-------------");
    Serial.println(ret);
    Serial.println(topic_config);
    Serial.println(payload_config);
    }



void reconnect() {
  int timeConsumed = 0;
  while (timeConsumed < MQTT_CONNECT_TIME_MILLI && !mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
      
    // Attempt to connect
    if (mqttClient.connect(clientId, topic_availability, 0, true, "offline")) {
      Serial.println("connected");
  
      // 连接成功时订阅主题
      mqttConnectedCallBack();
      mqttClient.publish(topic_availability, "online", true);
      mqttClient.subscribe(topic_order);
      char attrs[200];
      sprintf(attrs, "{\"ip\":\"%s\", \"version\":\"%s\", \"client_id\":\"%s\", \"rssi\":\"%d dB\"}", WiFi.localIP().toString().c_str(), VERSION, clientId, WiFi.RSSI());
      
      mqttClient.publish(topic_attr, attrs, true);
      mqttClient.publish(topic_version, VERSION, true);
      if (restarted == 0) {
        Serial.println("First time restarted");
        restarted = 1;
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      timeConsumed += 5000;
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);   // 打印主题信息
  Serial.print("] ");
  Serial.print("Payload [");
  char message[length];
  for(int i=0; i< length; i++) {
    message[i] = (char)payload[i];
    Serial.print(message[i]); 
  }
//  message[length] = '\0';
  Serial.println("] ");
  if (0 == strcmp(topic, topic_order)) {
//    char value = (char)payload[0];
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(message);
    if (json.success()) {
      Serial.println("Json deseiralization successfully!");
      if (json.containsKey("cmd")) {
        Serial.println("Json content legal !");
        int command = json["cmd"];
        int parameter = json.containsKey("para") ? json["para"] : 0;
        mqttCallback(command, parameter);
        
      }
      
    } 
  }
  
}
void setupMQTT() {
#if USING_FLASH_CONFIG
  loadConfig();
#endif
  setupTopics();
  log_info(clientId);
  mqttClient.setServer(mqtt_server, 1883);                              //设定MQTT服务器与使用的端口，1883是默认的MQTT端口
  mqttClient.setCallback(callback); 
  
  
}
void setupMQTTWithConnectedCallback(void(*cb)(void)) {
  setupMQTT();
  mqttConnectedCallBack = cb;
  
}
void mqttLoop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}
bool publishStateToMQTT(char *payload) {
  if (mqttClient.connected()) {
    return mqttClient.publish(topic_state, payload);
  } else {
    log_error("未连接MQTT，不发送……");
    return false;
  }
}
bool publishMQTT(char *topic, char *payload) {
  if (mqttClient.connected()) {
    char tp[60];
    createTopic(tp, topic);

    return mqttClient.publish(tp, payload);
  } else {
    log_error("未连接MQTT，不发送……");
    return false;
  }
}

void setupMqttCallback(void(*cb)(int,int)) {
  mqttCallback = cb;
}
#endif

