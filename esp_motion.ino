/*
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

// input pin from sensor
const byte sensorPin = 3;

// outgoing alert messages
const char* sensorLOW = "No Motion Detected";
const char* sensorHIGH = "Motion Detected!";
const char* sensorWARN = "Motion Recently Detected!";

// prev value is not 0 or 1 so it will send its condition after boot
byte previousValue = 3;
byte currentValue;

// hotspot to connect to
const char* ssid = "has-network";
const char* password = "12345678";
const char* mqttServer = "192.168.137.1";

// esp settings
const char* espHostname = "esp-has-motion";
const char* espClientId = "motionClient";


// topics to subscribe and publish to
const char* topic = "motion";
const char* topicAck = "motion/acknowledge";
const char* topicTest = "motion/test";
const char* topicStatus = "motion/status";

void setup() { 
  /* 
    If sending data to another board only TX can be used
    as the RX pin is used for input.
    ex: 
      Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
      Serial.write( digitalRead(3) );
  */
  pinMode(sensorPin, INPUT);
  
  // connect to hotspot on computer  
  WiFi.hostname(espHostname);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
  }

  // connect to server
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {  
  if ((char)payload[0] == '1') {
    client.publish(topicTest, "Success"); delay(2500);
    client.publish(topicTest, " ");
  }
  else if ((char)payload[0] == '2') {
    client.publish(topicAck, "Dismissing Warning ... 3"); delay(1000);
    client.publish(topicAck, "Dismissing Warning ... 2"); delay(1000);
    client.publish(topicAck, "Dismissing Warning ... 1"); delay(1000);
    client.publish(topicAck, " "); 
  }  
}

void loop() {
  // (re)connect to server
  while (!client.connected()) {
    // attempt to connect with a client id
    if (client.connect(espClientId)) {
      // (re)subscribe to topic
      client.subscribe(topic);
    } 
    else {      
      delay(5000); // failed connection. retry in 5 seconds
    }
  }
  
  client.loop();
  
  currentValue = digitalRead(sensorPin);
  if(currentValue != previousValue){
    if (currentValue == 1){
      client.publish(topicStatus, sensorHIGH);
      client.publish(topicAck, sensorWARN);
    }
    else {
      client.publish(topicStatus, sensorLOW);
    }    
  }
  previousValue = currentValue;  

  // check sensor every 200 milliseconds
  delay(200);
}
