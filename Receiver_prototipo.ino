#include "EBYTE.h"

/*
WARNING: IF USING AN ESP32
DO NOT USE THE PIN NUMBERS PRINTED ON THE BOARD
YOU MUST USE THE ACTUAL GPIO NUMBER
*/
#define PIN_RX 16   // Serial2 RX (connect this to the EBYTE Tx pin)
#define PIN_TX 17   // Serial2 TX pin (connect this to the EBYTE Rx pin)

#define PIN_M0 4    // D4 on the board (possibly pin 24)
#define PIN_M1 22   // D2 on the board (possibly called pin 22)
#define PIN_AX 21   // D15 on the board (possibly called pin 21)


// i recommend putting this code in a .h file and including it
// from both the receiver and sender modules
struct DATA {
  unsigned long Count;
  int Bits;
  char longitude[10];
  char latitude[10];
  float Volts;

};

// these are just dummy variables, replace with your own
int Chan;
DATA MyData;
unsigned long Last;


// create the transceiver object, passing in the serial and pins
EBYTE Transceiver(&Serial2, PIN_M0, PIN_M1, PIN_AX);

// =================================================================
// DAQUI PRA CIMA É DA BIBLIOTECA EBYTE
//==================================================================

// Isso daqui é meu
// Medida de temperatura
double Vs = 3.3; // TENSÃO DE SAIDA DO ESP32
double R1 = 470; //RESISTOR UTILIZADO NO DIVISOR DE TENSÃO
double Beta = 3950; // VALOR DE BETA
double To=298.15; // VALOR EM KELVIN REFERENTE A 25° CELSIUS
double Ro=1000;
//==================================================================

#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
  #elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
  #endif
  
  #include <InfluxDbClient.h>
  #include <InfluxDbCloud.h>
  
  // WiFi AP SSID
  #define WIFI_SSID "Internet Gratis"
  // WiFi password
  #define WIFI_PASSWORD "sopr4067"
  
  #define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
  #define INFLUXDB_TOKEN "4V0lbHBVWwmFGL8A1takTaNYsmhT_G_Pc3ZsYeCv5olkmEoRyYPrpS-CwwymJGp7Jhq7GR6geCWk8kF3uQBV1A=="
  #define INFLUXDB_ORG "Unicamp MO629"
  #define INFLUXDB_BUCKET "temp_data"

  // Time zone info
  #define TZ_INFO "UTC-3"
  
  // Declare InfluxDB client instance with preconfigured InfluxCloud certificate
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  
  // Declare Data point
  // Point sensor("wifi_status");
  Point sensor("temperature");
  
  void setup() {

    // =================================================================
    // TRECHO DA BIBLIOTECA EBYTE

    Serial.begin(9600);

    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    Serial.println("Starting Reader");

    // this init will set the pinModes for you
    Transceiver.init();

    // all these calls are optional but shown to give examples of what you can do

    // Serial.println(Transceiver.GetAirDataRate());
    // Serial.println(Transceiver.GetChannel());
    // Transceiver.SetAddressH(1);
    // Transceiver.SetAddressL(1);
    // Chan = 15;
    // Transceiver.SetChannel(Chan);
    // save the parameters to the unit,
    // Transceiver.SetPullupMode(1);
    // Transceiver.SaveParameters(PERMANENT);

    // you can print all parameters and is good for debugging
    // if your units will not communicate, print the parameters
    // for both sender and receiver and make sure air rates, channel
    // and address is the same
    Transceiver.PrintParameters();
    //=====================================================================
    // Add tags to the data point
    sensor.addTag("device", DEVICE);

    //===========
  }

  void loop() {
    //==============
    // Isso aqui é meu
    //GARANTE QUE AS INFORMAÇÕES SERÃO RESETADAS APÓS CADA LEITURA
    double Vout, Rt = 0;
    double T, Tc;
    //==============
  // =================================================================
  // TRECHO DA BIBLIOTECA EBYTE

  // if the transceiver serial is available, proces incoming data
  // you can also use Transceiver.available()


  if (Serial2.available()) {

    // i highly suggest you send data using structures and not
    // a parsed data--i've always had a hard time getting reliable data using
    // a parsing method

    Transceiver.GetStruct(&MyData, sizeof(MyData));

    // note, you only really need this library to program these EBYTE units
    // you can call readBytes directly on the EBYTE Serial object
    // Serial2.readBytes((uint8_t*)& MyData, (uint8_t) sizeof(MyData));


    // dump out what was just received
    Serial.print("Count: "); Serial.println(MyData.Count);
    Serial.print("Bits: "); Serial.println(MyData.Bits);
    Serial.print("Longitude: "); Serial.println(MyData.longitude);
    Serial.print("Latitude: "); Serial.println(MyData.latitude);
    Serial.print("Volts: "); Serial.println(MyData.Volts);
    //==============
    // Isso aqui é meu

    //GARANTE QUE AS INFORMAÇÕES SERÃO RESETADAS APÓS CADA LEITURA
    double Vout, Rt = 0;
    double T, Tc;

    // Cálculo de Temperatura
    Vout = MyData.Volts;
    Rt = R1 * Vout / (Vs - Vout);
    T = 1/(1/To + log(Rt/Ro)/Beta);
    Tc = T - 273.15;
    //==============
    // if you got data, update the checker
    Last = millis();

    //==================================================================
    // Setup wifi
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
    Serial.print("Connecting to wifi");
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
    }
    Serial.println();
  
    // Accurate time is necessary for certificate validation and writing in batches
    // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
    // Syncing progress and the time will be printed to Serial.
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  
  
    // Check server connection
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }

    // Clear fields for reusing the point. Tags will remain the same as set above.
    sensor.clearFields();
  
    // Store measured value into point
    // Report RSSI of currently connected network
    //sensor.addField("rssi", WiFi.RSSI());
    //===========
    // Isso aqui é meu
    sensor.addField("temperature", Tc);
    sensor.addField("longitude", MyData.longitude);
    sensor.addField("latitude", MyData.latitude);
    //===========
  
    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(sensor.toLineProtocol());
  
    // Check WiFi connection and reconnect if needed
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("Wifi connection lost");
    }
  
    // Write point
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  
    // Serial.println("Waiting 1 second");
    // delay(1000); não precisa ficar esperando, senão vai perder pacote
    
    //============
    // Isso aqui é meu
    // Desliga o WiFi para evitar interferência
    WiFi.disconnect();
    // WiFi.mode(WIFI_OFF);
    Serial.println("WiFi turned off.");
    //===========
  }
    else {
      // if the time checker is over some prescribed amount
      // let the user know there is no incoming data
      if ((millis() - Last) > 1000) {
        Serial.println("Searching: ");
        Last = millis();
      }

    }
    }