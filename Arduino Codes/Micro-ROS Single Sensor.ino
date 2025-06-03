#include <micro_ros_arduino.h>
#include <Arduino.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>
#include <WiFi.h>

// WiFi credentials
char* ssid = "*******";
char* password = "********";

// micro-ROS agent IP and port
char* ip = "192.168.220.75";
const uint32_t agent_port = 8888;

// ROS node and publisher
rcl_node_t node;
rcl_publisher_t pub_ldr;
rclc_executor_t executor;
rcl_allocator_t allocator;
rclc_support_t support;

// LDR Pin
#define LDR_PIN 34

// Message
std_msgs__msg__Int32 ldr_msg;

// Check agent connection function
bool check_agent_connection() {
    return rmw_uros_ping_agent(100, 1) == RMW_RET_OK;
}

void setup() {
    pinMode(LDR_PIN, INPUT);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }

    // Setup micro-ROS WiFi transport
    set_microros_wifi_transports(ssid, password, ip, agent_port);
    delay(2000);

    // Wait for agent
    while (!check_agent_connection()) {
        delay(1000);
    }

    allocator = rcl_get_default_allocator();

    // Initialize ROS support
    rclc_support_init(&support, 0, NULL, &allocator);

    // Create node
    rclc_node_init_default(&node, "esp32_single_sensor_node", "", &support);

    // Create publisher for LDR sensor
    rclc_publisher_init_default(&pub_ldr, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "ldr_value");

    // Create executor
    rclc_executor_init(&executor, &support.context, 1, &allocator);
}

void loop() {
    if (check_agent_connection()) {
        ldr_msg.data = analogRead(LDR_PIN);
        rcl_publish(&pub_ldr, &ldr_msg, NULL);
    }
    delay(500);
}
