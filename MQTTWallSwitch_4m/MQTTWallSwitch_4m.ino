#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>

const char* ssid = "Seras";
const char* password = "Ivanhoe1";
const char* mqtt_server = "192.168.1.11";
const char* MQTTUser = "mqttroot";
const char* MQTTPass = "Kaffekop1";

char deviceId[16] ;

bool onArray[4] = {false, false, false, false};
bool offArray[4] = {false, false, false, false};
bool BlockOnArray[4] = {false, false, false, false};
unsigned long onTimeArray[4] = {0, 0, 0, 0};

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
  bool state = digitalRead(5);
  ProcessInterupt(3, state); 
}

// Interupt rutine for button 2
void ICACHE_RAM_ATTR stateChange1() {
  bool state = digitalRead(4);
  ProcessInterupt(1, state); 
}

// Interupt rutine for button 3
void ICACHE_RAM_ATTR stateChange2() {
  bool state = digitalRead(0);  //12
  ProcessInterupt(0, state); 
}

// Interupt rutine for button 4
void ICACHE_RAM_ATTR stateChange3() {
  bool state = digitalRead(2);  
  ProcessInterupt(2, state); 
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  // initialize the pushbutton pins as an input:
  pinMode(5, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP); 
  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);

  uint32_t ChipId = ESP.getChipId();
  sprintf(deviceId, "%lu",ChipId);
  
  // Attach interrupts to the ISR vector
  attachInterrupt(digitalPinToInterrupt(5), stateChange0, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(4), stateChange1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(0), stateChange2, CHANGE); //12
  attachInterrupt(digitalPinToInterrupt(2), stateChange3, CHANGE);
  
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
  Serial.print("Message arrived: ");
  Serial.print(topic);
//  Serial.print(" ");
//  String recieveTopic = (char*)topic;
//  Serial.print("Payload: ");
//  Serial.write(payload, length);
//  Serial.println();
//  char p[length + 1];
//  memcpy(p, payload, length);
//  p[length] = NULL;
//  String message(p);
//  if (recieveTopic == (char*)commandTopic0) {
//    if (message.equals("1")) {
//      digitalWrite(0, HIGH);
//    } else {
//      digitalWrite(0, LOW);
//    }
//  } else if (recieveTopic == (char*)commandTopic2) {
//    if (message.equals("1")) {
//      digitalWrite(2, HIGH);
//    } else {
//      digitalWrite(2, LOW);
//    }
//  } else if (recieveTopic == (char*)commandTopic4) {
//    if (message.equals("1")) {
//      digitalWrite(4, HIGH);
//    } else {
//      digitalWrite(4, LOW);
//    }
//  } else if (recieveTopic == (char*)commandTopic5) {
//    if (message.equals("1")) {
//      digitalWrite(5, HIGH);
//    } else {
//      digitalWrite(5, LOW);
//    }
//  } else {
//    Serial.print("Topic unknown");
//  }
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
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(deviceId, MQTTUser, MQTTPass)) {
      Serial.println("connected");
//      client.publish(stateTopic, "ready");
//      client.subscribe(commandTopic0);
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
