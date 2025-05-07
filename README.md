# TranquilTrackAI: AI-Powered Autonomous Wildlife Tranquilization Drone

**Status:** Actively Under Development (Core detection & tracking operational, refining targeting & flight control)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
## Table of Contents

- [Overview](#overview)
- [Problem Statement](#problem-statement)
- [Solution](#solution)
- [Key Features](#key-features)
- [System Architecture](#system-architecture)
- [Tech Stack](#tech-stack)
- [Current Progress](#current-progress)
- [Roadmap & Future Work](#roadmap--future-work)
- [Installation](#installation)
- [Usage](#usage)
  - [Training the Model](#training-the-model)
  - [Running Live Detection](#running-live-detection)
- [Dataset](#dataset)
- [Challenges & Learnings](#challenges--learnings)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [Author](#author)
- [Acknowledgments](#acknowledgments)

## Overview

TranquilTrackAI is an innovative project focused on developing an autonomous drone system for the safe, humane, and efficient tranquilization of wildlife. Leveraging cutting-edge AI for object detection and tracking, the drone aims to accurately identify animals and specific tranquilization zones, navigate autonomously, and (in future iterations) precisely deploy tranquilizer darts. This project explores the intersection of robotics, artificial intelligence, and wildlife management.

## Problem Statement

Traditional wildlife tranquilization can be risky for both animals and personnel, often requiring close proximity in challenging terrains. It can be time-consuming, stressful for the animals, and sometimes inaccurate. There's a need for a solution that enhances safety, precision, and efficiency.

## Solution

TranquilTrackAI proposes a drone equipped with an HD camera (ESP32-CAM) and an onboard companion computer (e.g., Raspberry Pi/Jetson Nano - *specify if you have one or plan for one*) running a YOLOv8 object detection model. The drone will:
1.  Stream live video.
2.  Detect and track target animals and pre-defined tranquilization zones in real-time.
3.  Communicate telemetry and detection data to a ground control station (mother system/laptop).
4.  Enable autonomous or semi-autonomous flight to approach and align with the target.
5.  (Future) Interface with a dart deployment mechanism.

## Key Features

* **Real-Time Animal Detection:** Utilizes YOLOv8 to detect various animal species.
* **Target Zone Identification:** Aims to identify specific, safe tranquilization areas on the animal.
* **Autonomous Navigation & Alignment:** Employs MAVLink for drone control, enabling basic autonomous flight patterns and target alignment.
* **Live Video Streaming:** ESP32-CAM provides a live video feed to the ground station.
* **Object Tracking:** Implements multi-object tracking to maintain a consistent view of targets.
* **Modular Design:** Python-based software with distinct modules for data processing, detection, drone control, and communication.
* **Ground Control Interface (Mother System):** (Describe what it does/will do - e.g., displays video, tracking data, allows for high-level commands).

## System Architecture

*(Create a diagram using draw.io or similar and save it as `docs/architecture_overview.png`)*
![System Architecture Diagram](docs/architecture_overview.png)

**Briefly describe the flow:**
1.  ESP32-CAM captures video.
2.  Video streamed to Companion Computer / Ground Station.
3.  YOLO model on Companion Computer / Ground Station processes video for detection.
4.  Tracking module updates target states.
5.  Detection/tracking data sent to Ground Station (if processed on drone) or used directly (if processed on Ground Station).
6.  Flight commands sent to Flight Controller (Pixhawk) via MAVLink.

## Tech Stack

* **Programming Languages:** Python 3.x, C++ (for ESP32)
* **AI/ML:** Ultralytics YOLOv8, OpenCV, PyTorch
* **Drone Control:** PyMavlink, MAVLink Protocol
* **Embedded System:** ESP32-CAM
* **Hardware (Examples - BE SPECIFIC TO YOUR SETUP):**
    * Drone Frame: [e.g., F450 Quadcopter Frame]
    * Flight Controller: [e.g., Pixhawk 2.4.8]
    * Companion Computer (if used): [e.g., Raspberry Pi 4, Jetson Nano]
    * Camera: ESP32-CAM (AI Thinker)
* **Development Tools:** VS Code, PlatformIO (for ESP32), Git, GitHub
* **Data Annotation:** [e.g., LabelImg, CVAT, Roboflow - Be honest about what you used]

## Current Progress

* ✅ Initial dataset collected and annotated for 'animal' and 'tranquil_zone' classes.
* ✅ YOLOv8n model trained, achieving ~40% mAP (actively working to improve this to 90%+).
* ✅ Basic drone control scripts for takeoff, landing, and guided movements developed.
* ✅ ESP32-CAM firmware for video streaming functional.
* ✅ Initial object tracking script using OpenCV implemented.
* 🚧 Improving model accuracy through data augmentation and hyperparameter tuning.
* 🚧 Developing a robust mother system for real-time data display and control.
* 🚧 Refining drone alignment logic based on detection.

## Roadmap & Future Work

* **Short Term (Next 1-3 Months):**
    * 🚀 Achieve >90% mAP for animal and tranquil_zone detection.
    * ⚙️ Implement robust real-time communication between drone (or companion computer) and mother system.
    * 🖥️ Develop a functional mother system dashboard (OpenCV GUI or Web-based).
    * 🛰️ Refine autonomous alignment and hovering based on target lock.
* **Medium Term (3-6 Months):**
    * 🎯 Integrate a simulated tranquilizer deployment trigger and feedback.
    * 🌬️ Investigate sensor fusion (e.g., GPS, IMU, Lidar) for improved state estimation and targeting.
    * 🛡️ Implement advanced failsafe mechanisms.
* **Long Term:**
    * 🌍 Expand dataset for more diverse animals and environments.
    * 🤝 Explore collaborative drone operations (swarming).
    * ⚖️ Rigorous testing in controlled, ethical environments.

## Installation

**Prerequisites:**
* Python 3.8+
* Git
* PlatformIO Core (for ESP32-CAM firmware)
* [Any other specific software like CUDA for GPU training]

**1. Clone the Repository:**
   ```bash
   git clone [https://github.com/YourUsername/TranquilTrackAI.git](https://github.com/YourUsername/TranquilTrackAI.git)
   cd TranquilTrackAI
