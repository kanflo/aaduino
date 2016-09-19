
/*

Simple thermometer demo for the AAduino

Probes the RFM69C radio and both DS18B20s and starts radio transmission of the
two temperature sensor readings every 2.5s

*/

#include <RFM69.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// Network credentials
#define GATEWAYID   1
#define NODEID      42
#define NETWORKID   100
#define FREQUENCY   RF69_868MHZ
#define KEY         "sampleEncryptKey" // 16 bytes

int led = 9;

OneWire ds1(A1); // Nearest the negative terminal on the AAduino
OneWire ds2(A0);
DallasTemperature sensor1(&ds1);
DallasTemperature sensor2(&ds2);


bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

RFM69 radio;

// the setup routine runs once when you press reset:
void setup() {         
  int radio_temp;  
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  led_flash();
  delay(150);
  led_flash();
  delay(150);
  led_flash();
  Serial.begin(115200);
  Serial.println("aaduino!");
  Serial.println("RFM init!");
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  //radio.setHighPower(); //uncomment only for RFM69HW!
  radio.encrypt(KEY);
  radio.promiscuous(promiscuousMode);
  Serial.print("Radio temperature is ");
  radio_temp = (int) radio.readTemperature(-1);
  Serial.print(radio_temp);
  Serial.println("");
  sensor1.begin();
  sensor2.begin();
}

void led_flash() {
  digitalWrite(led, HIGH);
  delay(50);
  digitalWrite(led, LOW);
}

typedef struct {		
  int           node_id;
  unsigned long counter;
  int  rfm_temp;
  int  temp0;
  int  temp1;
} Payload;

unsigned int counter = 0;
long lastPeriod = -1;
Payload packet;
int xmit_period_ms = 2500;

// the loop routine runs over and over again forever:
void loop() {
  float temp;
  int currPeriod = millis() / xmit_period_ms;
  if (currPeriod != lastPeriod) {
    sensor1.requestTemperatures();
    sensor2.requestTemperatures();
    packet.node_id = NODEID;
    packet.counter = counter++;
    packet.rfm_temp = (int) radio.readTemperature(-1) * 10;
    temp = sensor1.getTempCByIndex(0);
    if ((int) temp == -127) {
      packet.temp0 = 0xffff;
    } else {
      packet.temp0 = temp * 10;
    }
    temp = sensor2.getTempCByIndex(0);
    if ((int) temp == -127) {
      packet.temp1 = 0xffff;
    } else {
      packet.temp1 = temp * 10;
    }

    Serial.print("temp1:");
    Serial.println(packet.temp0);
    Serial.print("temp2:");
    Serial.println(packet.temp1);
    Serial.print("Sending struct (");
    Serial.print(sizeof(packet));
    Serial.print(" bytes) ... ");
    if (radio.sendWithRetry(GATEWAYID, (const void*)(&packet), sizeof(packet))) {
      Serial.print(" ok!");
    } else {
      Serial.print(" nothing...");
    }
    Serial.println();
    led_flash();
    lastPeriod = currPeriod;
  }
}

