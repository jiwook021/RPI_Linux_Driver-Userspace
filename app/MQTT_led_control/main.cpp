/**
 * MQTT LED Controller for Raspberry Pi - Robust Version
 * 
 * This program controls an LED connected to a GPIO pin on Raspberry Pi
 * via MQTT messages. Enhanced with robust error handling and connection management.
 * 
 * Dependencies:
 * - Eclipse Paho MQTT C++ Client Library
 * - C++11 or later
 * 
 * Compilation:
 * g++ -std=c++11 mqtt_led_controller_robust.cpp -o mqtt_led_controller_robust -lpaho-mqttpp3 -lpaho-mqtt3as
 * http://169.254.50.163:8080/data/app/MQTT_led_control/
 * python3 -m http.server 8080
 */

 #include <iostream>
 #include <fstream>
 #include <string>
 #include <csignal>
 #include <chrono>
 #include <thread>
 #include <atomic>
 #include <memory>
 #include <mqtt/async_client.h>
 
 // Constants
 const std::string MQTT_SERVER_ADDRESS = "tcp://localhost:1883";
 const std::string CLIENT_ID = "rpi_gpio_controller";
 const std::string TOPIC_CONTROL = "rpi/led/control";
 const std::string TOPIC_STATUS = "rpi/led/status";
 const int QOS = 1;
 const int GPIO_PIN = 17;  // Change to your actual GPIO pin number
 const int PUBLISH_INTERVAL_MS = 5000;   // Status publishing interval in milliseconds
 const int RECONNECT_DELAY_MS = 5000;    // Reconnection delay in milliseconds
 const int CONNECTION_TIMEOUT_MS = 10000; // Connection timeout in milliseconds
 
 // Global flag for program termination
 std::atomic<bool> running{true};
 
 /**
  * Class for controlling GPIO pins via sysfs interface
  */
 class GpioController {
 private:
     int pin;
     std::atomic<bool> exported{false};
     std::string valueFilePath;
     std::string directionFilePath;
     std::string exportFilePath;
     std::string unexportFilePath;
 
 public:
     /**
      * Constructor
      * @param gpio_pin The GPIO pin number to control
      */
     GpioController(int gpio_pin) : pin(gpio_pin) {
         valueFilePath = "/sys/class/gpio/gpio" + std::to_string(pin) + "/value";
         directionFilePath = "/sys/class/gpio/gpio" + std::to_string(pin) + "/direction";
         exportFilePath = "/sys/class/gpio/export";
         unexportFilePath = "/sys/class/gpio/unexport";
     }
 
     /**
      * Destructor - unexports the pin if it was exported
      */
     ~GpioController() {
         if (exported) {
             unexport();
         }
     }
 
     /**
      * Export the GPIO pin to make it accessible via sysfs
      * @return true if successful, false otherwise
      */
     bool exportPin() {
         std::cout << "Exporting pin " << pin << std::endl;
         
         // First check if already exported
         if (fileExists(valueFilePath)) {
             std::cout << "Pin " << pin << " is already exported" << std::endl;
             exported = true;
             return true;
         }
         
         std::ofstream exportFile(exportFilePath);
         if (!exportFile.is_open()) {
             std::cerr << "Failed to open export file. Are you running as root?" << std::endl;
             return false;
         }
         
         exportFile << pin;
         exportFile.close();
         
         // Give the system time to create the direction file
         for (int i = 0; i < 20; i++) {
             std::this_thread::sleep_for(std::chrono::milliseconds(50));
             if (fileExists(directionFilePath)) {
                 exported = true;
                 std::cout << "Pin " << pin << " exported successfully" << std::endl;
                 return true;
             }
         }
         
         std::cerr << "Failed to export pin " << pin << " (timeout)" << std::endl;
         return false;
     }
 
     /**
      * Unexport the GPIO pin
      * @return true if successful, false otherwise
      */
     bool unexport() {
         std::cout << "Unexporting pin " << pin << std::endl;
         
         std::ofstream unexportFile(unexportFilePath);
         if (!unexportFile.is_open()) {
             std::cerr << "Failed to open unexport file" << std::endl;
             return false;
         }
         
         unexportFile << pin;
         unexportFile.close();
         
         // Verify the pin was unexported
         for (int i = 0; i < 10; i++) {
             std::this_thread::sleep_for(std::chrono::milliseconds(50));
             if (!fileExists(valueFilePath)) {
                 exported = false;
                 std::cout << "Pin " << pin << " unexported successfully" << std::endl;
                 return true;
             }
         }
         
         std::cerr << "Failed to unexport pin " << pin << " (timeout)" << std::endl;
         return false;
     }
 
     /**
      * Set the direction of the GPIO pin (in or out)
      * @param direction "in" for input, "out" for output
      * @return true if successful, false otherwise
      */
     bool setDirection(const std::string& direction) {
         if (!exported && !exportPin()) {
             std::cerr << "Cannot set direction on unexported pin" << std::endl;
             return false;
         }
         
         std::ofstream directionFile(directionFilePath);
         if (!directionFile.is_open()) {
             std::cerr << "Failed to open direction file" << std::endl;
             return false;
         }
         
         directionFile << direction;
         directionFile.close();
         
         // Verify the direction was set
         std::string currentDirection = getCurrentDirection();
         if (currentDirection != direction) {
             std::cerr << "Failed to set direction. Current: " << currentDirection 
                       << ", Requested: " << direction << std::endl;
             return false;
         }
         
         std::cout << "Direction set to " << direction << " successfully" << std::endl;
         return true;
     }
 
     /**
      * Get the current direction of the GPIO pin
      * @return "in", "out", or empty string on error
      */
     std::string getCurrentDirection() {
         if (!exported) {
             return "";
         }
         
         std::ifstream directionFile(directionFilePath);
         if (!directionFile.is_open()) {
             return "";
         }
         
         std::string direction;
         std::getline(directionFile, direction);
         directionFile.close();
         
         return direction;
     }
 
     /**
      * Write a value to the GPIO pin
      * @param value 0 for low, 1 for high
      * @return true if successful, false otherwise
      */
     bool writeValue(int value) {
         if (!exported && !exportPin()) {
             std::cerr << "Cannot write to unexported pin" << std::endl;
             return false;
         }
         
         // Ensure direction is "out"
         std::string direction = getCurrentDirection();
         if (direction != "out") {
             std::cout << "Direction is not 'out', setting it now..." << std::endl;
             if (!setDirection("out")) {
                 return false;
             }
         }
         
         std::ofstream valueFile(valueFilePath);
         if (!valueFile.is_open()) {
             std::cerr << "Failed to open value file for writing" << std::endl;
             return false;
         }
         
         valueFile << value;
         valueFile.close();
         
         // Verify the value was set
         int currentValue = readValue();
         if (currentValue != value) {
             std::cerr << "Failed to set value. Current: " << currentValue 
                       << ", Requested: " << value << std::endl;
             return false;
         }
         
         std::cout << "Value set to " << value << " successfully" << std::endl;
         return true;
     }
 
     /**
      * Read the current value of the GPIO pin
      * @return -1 on error, 0 for low, 1 for high
      */
     int readValue() {
         if (!exported && !exportPin()) {
             std::cerr << "Cannot read from unexported pin" << std::endl;
             return -1;
         }
         
         std::ifstream valueFile(valueFilePath);
         if (!valueFile.is_open()) {
             std::cerr << "Failed to open value file for reading" << std::endl;
             return -1;
         }
         
         std::string value;
         std::getline(valueFile, value);
         valueFile.close();
         
         try {
             return std::stoi(value);
         } catch (const std::exception& e) {
             std::cerr << "Error converting GPIO value: " << e.what() << std::endl;
             return -1;
         }
     }
 
 private:
     /**
      * Check if a file exists
      * @param path Path to the file
      * @return true if file exists, false otherwise
      */
     bool fileExists(const std::string& path) {
         std::ifstream file(path);
         return file.good();
     }
 };
 
 /**
  * MQTT callback handler class
  */
 class MqttCallback : public virtual mqtt::callback {
 private:
     mqtt::async_client& client;
     GpioController& gpio;
     std::atomic<bool> reconnection_required{false};
 
 public:
     MqttCallback(mqtt::async_client& client, GpioController& gpio) 
         : client(client), gpio(gpio) {}
 
     /**
      * Handle connection loss
      * @param cause The cause of the connection loss
      */
     void connection_lost(const std::string& cause) override {
         std::cout << "\n*** Connection lost: " << cause << " ***" << std::endl;
         reconnection_required = true;
     }
 
     /**
      * Handle reconnect
      */
     bool reconnect() {
         if (!reconnection_required || !running) {
             return true;
         }
 
         std::cout << "Attempting to reconnect..." << std::endl;
         
         // Reconnection attempt loop
         int retries = 0;
         const int MAX_RETRIES = 10;
         
         while (running && reconnection_required && retries < MAX_RETRIES) {
             try {
                 retries++;
                 std::cout << "Reconnection attempt " << retries << " of " << MAX_RETRIES << std::endl;
                 
                 auto connOpts = mqtt::connect_options_builder()
                     .clean_session(true)
                     .connect_timeout(std::chrono::seconds(10))
                     .keep_alive_interval(std::chrono::seconds(20))
                     .finalize();
                 
                 // Connect synchronously
                 mqtt::token_ptr conntok = client.connect(connOpts);
                 conntok->wait();
                 
                 std::cout << "Reconnected successfully" << std::endl;
                 
                 // Resubscribe to topics
                 std::cout << "Resubscribing to topics..." << std::endl;
                 client.subscribe(TOPIC_CONTROL, QOS)->wait();
                 std::cout << "Resubscribed to: " << TOPIC_CONTROL << std::endl;
                 
                 // Clear retained messages
                 std::cout << "Clearing any retained messages..." << std::endl;
                 mqtt::message_ptr clearmsg = mqtt::make_message(TOPIC_CONTROL, "");
                 clearmsg->set_qos(QOS);
                 clearmsg->set_retained(true);
                 client.publish(clearmsg)->wait();
                 
                 // Publish status
                 publishStatus();
                 
                 reconnection_required = false;
                 return true;
                 
             } catch (const mqtt::exception& exc) {
                 std::cerr << "Error during reconnection: " << exc.what() << std::endl;
                 std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS));
             }
         }
         
         if (retries >= MAX_RETRIES) {
             std::cerr << "Failed to reconnect after " << MAX_RETRIES << " attempts" << std::endl;
         }
         
         return false;
     }
 
     /**
      * Handle incoming MQTT messages
      * @param msg The incoming message
      */
     void message_arrived(mqtt::const_message_ptr msg) override {
         std::cout << "\n===============================================" << std::endl;
         std::cout << "Message arrived on topic: " << msg->get_topic() << std::endl;
       //  std::cout << "Message ID: " << msg->get_message_id() << std::endl;
         std::cout << "Is retained: " << (msg->is_retained() ? "yes" : "no") << std::endl;
         std::cout << "QoS: " << msg->get_qos() << std::endl;
         
         if (msg->get_topic() == TOPIC_CONTROL) {
             std::string payload = msg->get_payload_str();
             std::cout << "Control message payload: '" << payload << "'" << std::endl;
             
             // Skip empty messages (used for clearing retained messages)
             if (payload.empty()) {
                 std::cout << "Empty payload, likely a retained message clear command. Ignoring." << std::endl;
                 return;
             }
             
             // Handle GPIO operations with retries
             bool success = false;
             int max_retries = 3;
             
             for (int retry = 0; retry < max_retries && !success; retry++) {
                 if (retry > 0) {
                     std::cout << "Retry " << retry << "/" << max_retries << std::endl;
                     
                     // Try to reset GPIO on retry
                     gpio.unexport();
                     std::this_thread::sleep_for(std::chrono::milliseconds(100));
                     gpio.exportPin();
                     gpio.setDirection("out");
                 }
                 
                 if (payload == "ON" || payload == "1") {
                     std::cout << "Turning LED ON" << std::endl;
                     success = gpio.writeValue(1);
                 } else if (payload == "OFF" || payload == "0") {
                     std::cout << "Turning LED OFF" << std::endl;
                     success = gpio.writeValue(0);
                 } else if (payload == "STATUS") {
                     std::cout << "Status request received" << std::endl;
                     success = true;  // No GPIO action needed for status
                 } else {
                     std::cerr << "Unknown control message: '" << payload << "'" << std::endl;
                     success = true;  // Mark as success to avoid retries for unknown commands
                 }
                 
                 if (!success) {
                     std::cout << "Operation failed, " << 
                         (retry < max_retries - 1 ? "retrying..." : "giving up.") << std::endl;
                     std::this_thread::sleep_for(std::chrono::milliseconds(100));
                 }
             }
             
             // Publish current status regardless of success
             publishStatus();
         }
         std::cout << "===============================================" << std::endl;
     }
     
     /**
      * Publish the current GPIO status to the status topic
      */
     void publishStatus() {
         if (!client.is_connected()) {
             std::cerr << "Client disconnected, cannot publish status" << std::endl;
             reconnection_required = true;
             return;
         }
         
         int value = gpio.readValue();
         if (value != -1) {
             std::string status = (value == 1) ? "ON" : "OFF";
             
             try {
                 mqtt::message_ptr pubmsg = mqtt::make_message(TOPIC_STATUS, status);
                 pubmsg->set_qos(QOS);
                 pubmsg->set_retained(false);
                 
                 client.publish(pubmsg)->wait_for(std::chrono::seconds(2));
                 std::cout << "Published status: " << status << std::endl;
             } catch (const mqtt::exception& exc) {
                 std::cerr << "Error publishing status: " << exc.what() << std::endl;
                 reconnection_required = true;
             }
         } else {
             std::cerr << "Error reading GPIO value, cannot publish status" << std::endl;
         }
     }
     
     /**
      * Check if reconnection is needed
      * @return true if reconnection is needed, false otherwise
      */
     bool needsReconnection() const {
         return reconnection_required;
     }
 };
 
 // Signal handler for graceful termination
 void signalHandler(int signum) {
     std::cout << "Interrupt signal (" << signum << ") received. Cleaning up..." << std::endl;
     running = false;
 }
 
 int main(int argc, char* argv[]) {
     // Register signal handlers
     signal(SIGINT, signalHandler);
     signal(SIGTERM, signalHandler);
     
     try {
         std::cout << "=== MQTT LED Controller - Robust Version ===" << std::endl;
         
         // Initialize GPIO
         std::cout << "Initializing GPIO..." << std::endl;
         GpioController gpio(GPIO_PIN);
         
         if (!gpio.exportPin()) {
             std::cerr << "Failed to export GPIO pin, retrying..." << std::endl;
             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
             
             if (!gpio.exportPin()) {
                 std::cerr << "Failed to export GPIO pin again, exiting" << std::endl;
                 return 1;
             }
         }
         
         if (!gpio.setDirection("out")) {
             std::cerr << "Failed to set GPIO direction, retrying..." << std::endl;
             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
             
             if (!gpio.setDirection("out")) {
                 std::cerr << "Failed to set GPIO direction again, exiting" << std::endl;
                 return 1;
             }
         }
         
         // Initialize the LED to OFF
         gpio.writeValue(0);
         
         // Create MQTT client
         std::cout << "Creating MQTT client..." << std::endl;
         mqtt::async_client client(MQTT_SERVER_ADDRESS, CLIENT_ID);
         
         // Set callback
         MqttCallback cb(client, gpio);
         client.set_callback(cb);
         
         // Set connection options
         auto connOpts = mqtt::connect_options_builder()
             .clean_session(true)
             .connect_timeout(std::chrono::milliseconds(CONNECTION_TIMEOUT_MS))
             .keep_alive_interval(std::chrono::seconds(20))
             .automatic_reconnect(true)
             .max_inflight(100)
             .finalize();
         
         std::cout << "Connecting to MQTT broker at " << MQTT_SERVER_ADDRESS << "..." << std::endl;
         
         // Connect with timeout
         bool connected = false;
         int connection_attempts = 0;
         const int MAX_CONNECTION_ATTEMPTS = 5;
         
         while (!connected && connection_attempts < MAX_CONNECTION_ATTEMPTS) {
             connection_attempts++;
             try {
                 mqtt::token_ptr conntok = client.connect(connOpts);
                 conntok->wait_for(std::chrono::milliseconds(CONNECTION_TIMEOUT_MS));
                 connected = true;
                 std::cout << "Connected to MQTT broker" << std::endl;
             } catch (const mqtt::exception& exc) {
                 std::cerr << "MQTT connection attempt " << connection_attempts << " failed: " 
                           << exc.what() << std::endl;
                 
                 if (connection_attempts < MAX_CONNECTION_ATTEMPTS) {
                     std::cout << "Retrying connection..." << std::endl;
                     std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS));
                 } else {
                     std::cerr << "Failed to connect after " << MAX_CONNECTION_ATTEMPTS 
                               << " attempts, exiting" << std::endl;
                     return 1;
                 }
             }
         }
         
         // Subscribe to control topic
         std::cout << "Subscribing to topic: " << TOPIC_CONTROL << std::endl;
         client.subscribe(TOPIC_CONTROL, QOS)->wait_for(std::chrono::seconds(5));
         std::cout << "Successfully subscribed to control topic" << std::endl;
         
         // Clear any retained messages on the control topic
         std::cout << "Clearing any retained messages..." << std::endl;
         mqtt::message_ptr clearmsg = mqtt::make_message(TOPIC_CONTROL, "");
         clearmsg->set_qos(QOS);
         clearmsg->set_retained(true);
         client.publish(clearmsg)->wait_for(std::chrono::seconds(2));
         std::cout << "Retained messages cleared" << std::endl;
         
         // Publish initial status
         cb.publishStatus();
         
         std::cout << "Entering main loop - program will continue running until interrupted" << std::endl;
         
         // Main loop - periodically publish status and check connection
         auto lastStatusTime = std::chrono::steady_clock::now();
         
         while (running) {
             auto currentTime = std::chrono::steady_clock::now();
             auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                 currentTime - lastStatusTime).count();
             
             // Publish status periodically
             if (elapsed >= PUBLISH_INTERVAL_MS) {
                 cb.publishStatus();
                 lastStatusTime = currentTime;
             }
             
             // Check for reconnection needs
             if (cb.needsReconnection()) {
                 cb.reconnect();
             }
             
             // Sleep to prevent CPU hogging
             std::this_thread::sleep_for(std::chrono::milliseconds(100));
         }
         
         // Graceful shutdown
         std::cout << "Shutting down..." << std::endl;
         
         // Turn off LED before exiting
         gpio.writeValue(0);
         
         // Disconnect from broker
         if (client.is_connected()) {
             std::cout << "Disconnecting from MQTT broker..." << std::endl;
             client.disconnect()->wait_for(std::chrono::seconds(2));
             std::cout << "Disconnected from MQTT broker" << std::endl;
         }
         
         // Unexport GPIO
         gpio.unexport();
         
     } catch (const mqtt::exception& exc) {
         std::cerr << "MQTT Error: " << exc.what() << std::endl;
         return 1;
     } catch (const std::exception& exc) {
         std::cerr << "Error: " << exc.what() << std::endl;
         return 1;
     }
     
     std::cout << "Program terminated gracefully" << std::endl;
     return 0;
 }