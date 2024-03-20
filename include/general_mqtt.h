/*****************************************************************************
 * 
 * ****************************************************************************/

/**
 * @file general.h
 * 
 * OTA依赖WiFi，
 * MQTT依赖WiFi、
 * WiFi设置依赖WiFi&flash存储、
 * 
 */
#ifndef __GENERAL_MQTT_H__
#define __GENERAL_MQTT_H__

#include "general.h"


#if MQTT

#define MQTT_CONNECT_TIME_MILLI 30000

#define TOPIC_PREFIX "home/"
#define TOPIC_SUFFIX_AVAILABILITY "/availability"
#define TOPIC_SUFFIX_COMMAND "/command"
#define TOPIC_SUFFIX_VERSION "/version"
#define TOPIC_SUFFIX_LOG "/log"
#define VERSION "1.0"//1.0
extern char topic_availability[64];
extern char topic_command[64];
extern char topic_order[64];
extern char topic_version[64];
extern char topic_log[64];
extern char topic_attr[64];
extern char topic_state[64];

extern char clientId[50]; 
extern char mqtt_server[20];

void createTopic(char* topic, char *topic_suffix);
bool publishStateToMQTT(char *payload);
bool publishMQTT(char *topic, char *payload);
void setupMqttCallback(void(*cb)(int,int));
void mqttLoop();
void setupMQTT();
void setupMQTTWithConnectedCallback(void(*cb)(void));
void createDiscovery(char* sensor_type, char* json_attributes_topic,
                        char* state_topic, char* s_name, char* unique_id_surfix,
                        char* availability_topic, char* device_class, char* value_template,
                        char* payload_on, char* payload_off, char* unit_of_meas,
                        int off_delay,
                        char* payload_available, char* payload_not_avalaible, bool gateway_entity, char* command_topic,
                        char* device_name, char* device_manufacturer, char* device_model, char* sw_version, char* device_identifier, 
                        char* device_mac, 
                        bool retainCmd,
                        char* icon,
                        char* state_on, char* state_off,
                        char* command_teplate,
                        char* options,
                        float max,
                        float min,
                        char* payload_reset);

#endif


#endif //__GENERAL_MQTT_H__