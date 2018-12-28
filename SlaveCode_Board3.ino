#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h> // Include the required Wire library for I2C#include <Wire.h

#define SAD 10
#define RST 5
MFRC522 nfc(SAD, RST);

#define NODE_ADDRESS 3  // Change this unique address for each I2C slave node
#define PAYLOAD_SIZE 2 // Number of bytes  expected to be received by the master I2C node


byte keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
byte keyB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };

byte FoundTag; // Variable used to check if Tag was found
byte ReadTag; // Variable used to store anti-collision value to read Tag information
byte TagData[MAX_LEN]; // Variable used to store Full Tag Data
byte TagSerialNumber[5]; // Variable used to store only Tag Serial Number
byte TagGroup1[5] = {0x07, 0x1D, 0xF2, 0x64}; // Group 1
byte TagGroup2[5] = {0x96, 0xDE, 0x00, 0x1A}; // Group 2

byte nodePayload[PAYLOAD_SIZE];

void setup() {
  SPI.begin();
  Serial.begin(115200);

  Serial.println("Looking for MFRC522.");
  nfc.begin();

  // Get the firmware version of the RFID chip
  byte version = nfc.getFirmwareVersion();
  if (! version) {
    Serial.print("Didn't find MFRC522 board.");
    while(1); //halt
  }

  Serial.print("Found chip MFRC522 ");
  Serial.print("Firmware ver. 0x");
  Serial.print(version, HEX);
  Serial.println(".");

  Serial.println("SLAVE SENDER NODE");
  Serial.print("Node address: ");
  Serial.println(NODE_ADDRESS);
  Serial.print("Payload size: ");
  Serial.println(PAYLOAD_SIZE);
  Serial.println("***********************");

  Wire.begin(NODE_ADDRESS);  // Activate I2C network
  Wire.onRequest(requestEvent); // Request attention of master node  
}


void loop() 
{
  byte status;
  byte data[MAX_LEN];
  byte serial[5];
  int i, j, pos;

  status = nfc.requestTag(MF1_REQIDL, data); // if tag will respond MI_OK

  if (status == MI_OK) {
    Serial.println("Tag detected.");
    Serial.print("Type: ");
    Serial.print(data[0], HEX);
    Serial.print(", ");
    Serial.println(data[1], HEX);

    // calculate the anti-collision value for the currently detected
    // tag and write the serial into the data array.
    status = nfc.antiCollision(data);
    memcpy(serial, data, 5);

    Serial.println("The serial nb of the tag is:");
    for (i = 0; i < 3; i++) {
      Serial.print(serial[i], HEX);
      Serial.print(", ");
    }
    Serial.println(serial[3], HEX);

int TagGroup= 0; // Variable used to confirm good Tag Detected

// Get anti-collision value to properly read information from the Tag
ReadTag = nfc.antiCollision(TagData);
memcpy(TagSerialNumber, TagData, 4); // Write the Tag information in the TagSerialNumber variable

Serial.println("Tag detected.");
Serial.print("Serial Number: ");
for (int i = 0; i < 4; i++) { // Loop to print serial number to serial monitor
Serial.print(TagSerialNumber[i], HEX);
Serial.print(", ");
}
Serial.println("");
Serial.println();


// Group 1 Tag Serial Number check 
    for(int i=0; i < 4; i++){
      if (TagGroup1[i] != TagSerialNumber[i]) {
        break; // if not equal, then break out of the "for" loop
      }
      if (i == 3) { // if we made it to 4 loops then the Tag Serial numbers are matching
      TagGroup=1;
      } 
      }

      
//Group 2 Tag Serial Number check
    for(int i=0; i < 4; i++){
      if (TagGroup2[i] != TagSerialNumber[i]) {
        break; // if not equal, then break out of the "for" loop
      }
      if (i == 3) { // if we made it to 4 loops then the Tag Serial numbers are matching
      TagGroup=2;
      } 
      }


// Send signal back to large RFID sensor      
     if (TagGroup == 1){
       Serial.println("GROUP 1!!!");
       Serial.println();
       nodePayload[0] = NODE_ADDRESS; // Sending Node address back.   
       nodePayload[1] = TagGroup; // Send Tag Group number
       Serial.print("Node Address: ");
       Serial.println(NODE_ADDRESS);
       Serial.print("Tag Group: ");
       Serial.println(TagGroup);
       delay(10000);
       }

       
     else if (TagGroup == 2){
        Serial.println("Group 2!!");
        Serial.println();
        nodePayload[0] = NODE_ADDRESS; // Sending Node address back.   
        nodePayload[1] = TagGroup; // Send Tag Group number
        Serial.print("Node Address: ");
        Serial.println(NODE_ADDRESS);
        Serial.print("Tag Group: ");
        Serial.println(TagGroup);
        delay(10000);
     } 
     delay(500);
     }
        delay(1000);
        nodePayload[0] = NODE_ADDRESS; // Sending Node address back. 
        nodePayload[1] = 0; //
        Serial.println("otherwise 0");
         
}

void requestEvent()
{
  Wire.write(nodePayload,PAYLOAD_SIZE);  
  Serial.print("Tag Group: ");  // for debugging purposes. 
  Serial.println(nodePayload[1]); // for debugging purposes. 
  
}        
