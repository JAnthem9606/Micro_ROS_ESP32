#include <micro_ros_arduino.h>
#include <Arduino.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/bool.h>
#include <NewPing.h>
#include <WiFi.h>

// WiFi credentials
char* ssid = "BLOCK-5";
char* password = "786ABV@5";

// micro-ROS agent IP and port
IPAddress agent_ip(192, 168, 220, 75);  // Change to your agent's IP
const uint32_t agent_port = 8888;
char* ip = "192.168.220.75";
// Node, publisher, subscriber, and executor
rcl_node_t node;
rcl_publisher_t pub_led_status, pub_ldr_value, pub_tch_value, pub_ult_value;
rcl_subscription_t sub_led_cmd;
rclc_executor_t executor;
rcl_allocator_t allocator;
rclc_support_t support;

// LED Pin
#define LED_PIN 2
#define LDR_PIN 34
#define TCH_PIN 5
#define trigger 22
#define echo 23
#define max_distance 200

NewPing sonar(trigger, echo, max_distance);
// Message
std_msgs__msg__Bool led_msg, tch_msg;
std_msgs__msg__Int32 ldr_msg, ult_msg;

// Agent Status
typedef enum { WAITING_AGENT, AGENT_AVAILABLE } agent_status_t;
agent_status_t state = WAITING_AGENT;

// Callback function for LED command
void led_callback(const void *msg_in) {
    const std_msgs__msg__Bool *msg = (const std_msgs__msg__Bool *)msg_in;
    digitalWrite(LED_PIN, msg->data ? HIGH : LOW);
}

// Function to check micro-ROS agent connection
bool check_agent_connection() {
    return rmw_uros_ping_agent(100, 1) == RMW_RET_OK;
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(TCH_PIN, OUTPUT);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
    // Initialize micro-ROS UDP transport
    set_microros_wifi_transports(ssid, password,ip, agent_port);
    delay(2000);  // Wait before checking agent
    
    // Check agent connectivity before initializing ROS
    while (!check_agent_connection()) {
        delay(1000);
    }

    // Allocator
    allocator = rcl_get_default_allocator();
    
    // Initialize ROS support
    rclc_support_init(&support, 0, NULL, &allocator);

    // Create node
    rclc_node_init_default(&node, "esp32_node", "", &support);

    // Create publishers
    rclc_publisher_init_default(&pub_led_status, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool), "led_status");
    rclc_publisher_init_default(&pub_tch_value, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool), "touch");
    rclc_publisher_init_default(&pub_ult_value, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "distance");
    rclc_publisher_init_default(&pub_ldr_value, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "ldr_value");

    // Create subscriber
    rclc_subscription_init_default(&sub_led_cmd, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool), "led_cmd");

    // Create executor
    rclc_executor_init(&executor, &support.context, 1, &allocator);
    rclc_executor_add_subscription(&executor, &sub_led_cmd, &led_msg, &led_callback, ON_NEW_DATA);
}

void loop() {
    // Check agent connectivity every 5 seconds
    static unsigned long last_check = 0;
    if (millis() - last_check > 5000) {
        last_check = millis();
        state = check_agent_connection() ? AGENT_AVAILABLE : WAITING_AGENT;
    }

    // Execute ROS loop only if agent is available
    if (state == AGENT_AVAILABLE) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
        ldr_msg.data = analogRead(LDR_PIN);
        tch_msg.data = digitalRead(TCH_PIN);
        ult_msg.data = sonar.ping_cm();

        rcl_publish(&pub_ldr_value, &ldr_msg, NULL);
        rcl_publish(&pub_tch_value, &tch_msg, NULL);
        rcl_publish(&pub_ult_value, &ult_msg, NULL);
    }
}
