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

//posicionado no ELDORADO
#define DEVICE_LONGITUDE "-47.06087"
#define DEVICE_LATITUDE  "-22.81351"


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
DATA MyData_local, MyData_remote;
unsigned long Last;


// create the transceiver object, passing in the serial and pins
EBYTE Transceiver(&Serial2, PIN_M0, PIN_M1, PIN_AX);

void setup() {


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

    MyData_local.Bits = 0;
    MyData_local.Volts = 0;
    MyData_local.Count = 0;
}

void loop() {

  // if the transceiver serial is available, proces incoming data
  // you can also use Transceiver.available()


  if (Serial2.available()) {

    // i highly suggest you send data using structures and not
    // a parsed data--i've always had a hard time getting reliable data using
    // a parsing method
    Transceiver.GetStruct(&MyData_remote, sizeof(MyData_remote));
    
    // note, you only really need this library to program these EBYTE units
    // you can call readBytes directly on the EBYTE Serial object
    // Serial2.readBytes((uint8_t*)& MyData, (uint8_t) sizeof(MyData));


    // dump out what was just received
    Serial.println("Received:");
    Serial.print("Count: "); Serial.println(MyData_remote.Count);
    Serial.print("Bits: "); Serial.println(MyData_remote.Bits);
    Serial.print("Longitude: "); Serial.println(MyData_remote.longitude);
    Serial.print("Latitude: "); Serial.println(MyData_remote.latitude);
    Serial.print("Volts: "); Serial.println(MyData_remote.Volts);
    Serial.println("");
    // if you got data, update the checker
    Last = millis();

    // generate my local data
    MyData_local.Count++;
    MyData_local.Bits+=2;
    MyData_local.Volts+=0.1; 

    //wait before forwarding
    delay(5000);

    Transceiver.SendStruct(&MyData_local, sizeof(MyData_local));
    //delay(10);
    Transceiver.SendStruct(&MyData_remote, sizeof(MyData_remote));

    Serial.print("Sending: "); Serial.print(MyData_local.Count); Serial.print("(local) and "); Serial.print(MyData_remote.Count); Serial.println("(remote)");
    Serial.println("");

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
