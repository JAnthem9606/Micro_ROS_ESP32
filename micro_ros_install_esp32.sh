#!/bin/bash

################################################################################
# Script: setup_microros_esp32.sh
# Purpose: Automate micro-ROS setup for ESP32 using ROS 2 Humble
# Author: Bilal and OpenAI ChatGPT
#
# Prerequisites (must be installed BEFORE running this script):
#   ✓ Ubuntu 20.04/22.04
#   ✓ ROS 2 Humble installed and sourced in /opt/ros/humble
#   ✓ Colcon build tool (`sudo apt install ros-dev-tools`)
#   ✓ Git and rosdep (`sudo apt install git python3-rosdep`)
#   ✓ Python3-pip (`sudo apt install python3-pip`)
#   ✓ Internet connection to clone and download packages
#   ✓ Permissions to install packages with `sudo`
################################################################################

set -e  # Exit immediately on error

ROS_DISTRO=humble

echo "=== [1/9] Sourcing ROS 2 ($ROS_DISTRO)... ==="
source /opt/ros/$ROS_DISTRO/setup.bash

echo "=== [2/9] Creating micro-ROS workspace... ==="
mkdir -p ~/microros_ws/src
cd ~/microros_ws

echo "=== [3/9] Cloning micro_ros_setup repository... ==="
git clone -b $ROS_DISTRO https://github.com/micro-ROS/micro_ros_setup.git src/micro_ros_setup

echo "=== [4/9] Updating and installing dependencies... ==="
sudo apt update
sudo apt install -y python3-pip
sudo rosdep init || true
rosdep update
rosdep install --from-path src --ignore-src -y

echo "=== [5/9] Building micro-ROS setup tools... ==="
colcon build
source install/local_setup.bash

echo "=== [6/9] Creating and building firmware workspace for ESP32... ==="
ros2 run micro_ros_setup create_firmware_ws.sh freertos esp32
ros2 run micro_ros_setup build_firmware.sh
source install/local_setup.bash

echo "=== [7/9] Creating micro-ROS agent workspace... ==="
ros2 run micro_ros_setup create_agent_ws.sh

echo "=== [8/9] Building micro-ROS agent... ==="
ros2 run micro_ros_setup build_agent.sh
source install/local_setup.bash

echo "✅ micro-ROS setup for ESP32 with ROS 2 $ROS_DISTRO completed successfully."
