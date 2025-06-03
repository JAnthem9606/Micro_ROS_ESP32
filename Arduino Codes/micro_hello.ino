#include <micro_ros_arduino.h>
#include <Arduino.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>

// ROS node and publisher
rcl_node_t node;
rcl_publisher_t pub_hello;
rclc_executor_t executor;
rcl_allocator_t allocator;
rclc_support_t support;

// Message
std_msgs__msg__String hello_msg;
// Check agent connection function
bool check_agent_connection() {
    return rmw_uros_ping_agent(100, 1) == RMW_RET_OK;
}

void setup() {
    // Initialize serial transport for micro-ROS
    set_microros_transports();
    // Wait for agent
    while (!check_agent_connection()) {
        delay(1000);
    }
    allocator = rcl_get_default_allocator();
    // Initialize ROS support
    rclc_support_init(&support, 0, NULL, &allocator);
    // Create node
    rclc_node_init_default(&node, "esp32_hello_node", "", &support);

    // Create publisher for LDR sensor
    rclc_publisher_init_default(
        &pub_hello,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg,  String),
        "message"
    );
    // Create executor
    rclc_executor_init(&executor, &support.context, 1, &allocator);
}

void loop() {
    if (check_agent_connection()) {
        char* msg = "Hello from ESP32 using Micro-ROS";
        hello_msg.data.data = msg;
        rcl_publish(&pub_hello, &hello_msg, NULL);
    }
    delay(500);
}
