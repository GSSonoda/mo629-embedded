/*

  This example shows how to connect to an EBYTE transceiver
  using an ESP32


  This example shows how to connect to an EBYTE transceiver
  using an ESP32

  This code for for the sender

  ESP32 won't allow SoftwareSerial (at least I can't get that lib to work
  so you will just hardwire your EBYTE directly to the Serial2 port


*/

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

//posicionado no IC
#define DEVICE_LONGITUDE "-47.06370"
#define DEVICE_LATITUDE  "-22.81321"


// i recommend putting this code in a .h file and including it
// from both the receiver and sender modules
struct DATA {
  unsigned long Count;
  int Bits;
  char longitude[10] = DEVICE_LONGITUDE;
  char latitude[10] = DEVICE_LATITUDE;
  float Volts;
};

// these are just dummy variables, replace with your own
int Chan;
DATA MyData;
unsigned long Last;
// Temperature measurement
double Vs = 3.3; // TENSÃO DE SAIDA DO ESP32
double R1 = 470; //RESISTOR UTILIZADO NO DIVISOR DE TENSÃO
double Beta = 3950; // VALOR DE BETA
double To=298.15; // VALOR EM KELVIN REFERENTE A 25° CELSIUS
double Ro=1000;
double adcMax = 4095.0;
float Temp;


// create the transceiver object, passing in the serial and pins
EBYTE Transceiver(&Serial2, PIN_M0, PIN_M1, PIN_AX);

void setup() {


  Serial.begin(9600);

  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("Starting Reader");

  // this init will set the pinModes for you
  Serial.println(Transceiver.init());

  // all these calls are optional but shown to give examples of what you can do

  // Serial.println(Transceiver.GetAirDataRate());
  // Serial.println(Transceiver.GetChannel());
  // Transceiver.SetAddressH(1);
  // Transceiver.SetAddressL(1);
  // Chan = 15;
  // Transceiver.SetChannel(Chan);
  // save the parameters to the unit,
  // Transceiver.SaveParameters(PERMANENT);

  // you can print all parameters and is good for debugging
  // if your units will not communicate, print the parameters
  // for both sender and receiver and make sure air rates, channel
  // and address is the same
  Transceiver.PrintParameters();

}

void loop() {
  //GARANTE QUE AS INFORMAÇÕES SERÃO RESETADAS APÓS CADA LEITURA
  double Vout, Rt = 0;
  double T, Tc;

  // measure some data and save to the structure
  MyData.Count++;
  MyData.Bits = analogRead(A13);
  MyData.Volts = MyData.Bits * Vs/adcMax;
  // Temperature Calculation
  Vout = MyData.Volts;
  Rt = R1 * Vout / (Vs - Vout);
  T = 1/(1/To + log(Rt/Ro)/Beta);
  Tc = T - 273.15;
  // i highly suggest you send data using structures and not
  // a parsed data--i've always had a hard time getting reliable data using
  // a parsing method
  Transceiver.SendStruct(&MyData, sizeof(MyData));

    // note, you only really need this library to program these EBYTE units
    // you can call write directly on the EBYTE Serial object
    // Serial2.write((uint8_t*) &Data, PacketSize );


  // let the use know something was sent
  Serial.print("Sending: "); Serial.println(MyData.Count);
  Serial.print("Temperature, in celsius: "); Serial.println(Tc);
  delay(20000);


}
