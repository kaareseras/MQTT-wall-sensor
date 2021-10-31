#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>

const char* ssid = "Seras";
const char* password = "Ivanhoe1";
const char* mqtt_server = "192.168.1.11";
const char* MQTTUser = "mqttroot";
const char* MQTTPass = "Kaffekop1";


char deviceId[16] ;


bool onArray[2] = {false, false};
bool offArray[2] = {false, false};
bool BlockOnArray[2] = {false, false};
unsigned long onTimeArray[4] = {0, 0};

int prell = 10;

int j =0;


WiFiClient espClient;
PubSubClient client(espClient);

//Common interupt rutine
void ICACHE_RAM_ATTR ProcessInterupt(int button, bool state) {
  if(state==false)
  {
    
    if (BlockOnArray[button] == false)
    {
      BlockOnArray[button] = true;
      //Serial.println("ON");
      onArray[button] = true;
           
    }  
  }
  else
  {
    //Serial.println("OFF");
    offArray[button] = true;
    BlockOnArray[button] = false;
  }  
}

// Interupt rutine for button 1
void ICACHE_RAM_ATTR stateChange0() {
  bool state = digitalRead(0);
  ProcessInterupt(2, state); 
}

// Interupt rutine for button 2
void ICACHE_RAM_ATTR stateChange1() {
  bool state = digitalRead(2);
  ProcessInterupt(1, state); 
}

unsigned long lastLoop = 0;
unsigned long now = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  // initialize the pushbutton pins as an input:
  pinMode(0, INPUT_PULLUP); 
  pinMode(2, INPUT_PULLUP);
  pinMode(4, OUTPUT);

  uint32_t ChipId = ESP.getChipId();
  sprintf(deviceId, "%lu",ChipId);
  
  // Attach interrupts to the ISR vector
  attachInterrupt(digitalPinToInterrupt(0), stateChange0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(2), stateChange1, CHANGE);
  
  setup_wifi();
  delay(10);
  client.setServer(mqtt_server, 1883);
  delay(10);
  client.setCallback(callback);
  delay(10);
  Serial.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  processButtonPresses();

  now = millis();
    if (now - lastLoop > 100) 
    {
      client.loop();
      lastLoop = now;
    }
}

void processButtonPresses(){

  char topic[50];
  
  for (int i = 0; i <= 3; i++)
  {
    if (onArray[i])   //Button pressed
    {
      onTimeArray[i] = millis();
      onArray[i] = false;
      sprintf(topic, "state/%s/%i", deviceId, i);
      client.publish(topic, "ON");
      
    }
    if (offArray[i])  //Button released
    {   
      offArray[i] = false;
      if( millis() - onTimeArray[i] < prell) //prell
      {
        onTimeArray[i] = 0;
      }
      else
      {
        sprintf(topic, "state/%s/%i", deviceId, i);
        client.publish(topic, "OFF");
      }
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(4, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(4, LOW);  // Turn the LED off by making the voltage HIGH
  }
}



void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.softAPdisconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  char topic[50];
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(deviceId, MQTTUser, MQTTPass)) {
      Serial.println("connected");
//      client.publish(stateTopic, "ready");
      sprintf(topic, "state/%s/%s/#", deviceId, "Led");
      client.subscribe(topic);
      
//      client.subscribe(commandTopic2);
//      client.subscribe(commandTopic4);
//      client.subscribe(commandTopic5);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
