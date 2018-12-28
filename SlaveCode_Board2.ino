/**************************************************************************/
/*! 
This code is based on the file "readMifare.pde"
created by Adafruit Industries
	@license  BSD (see license.txt)

    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    - Authenticate block 4 (the first block of Sector 1) using
      the default KEYA of 0XFF 0XFF 0XFF 0XFF 0XFF 0XFF
    - If authentication succeeds, we can then read any of the
      4 blocks in that sector (though only block 4 is read here)
	 
    If the card has a 7-byte UID it is probably a Mifare
    Ultralight card, and the 4 byte pages can be read directly.
    Page 4 is read by default since this is the first 'general-
    purpose' page on the tags.


This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
This library works with the Adafruit NFC breakout 
  ----> https://www.adafruit.com/products/364
 
Check out the links above for our tutorials and wiring diagrams 
These chips use SPI or I2C to communicate.

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

*/
/**************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Wire.h> // Include the required Wire library for I2C to connect Arduinos
 
// To use the shield with I2C, define the pins connected to the IRQ and reset lines.  
#define PN532_IRQ   (2)
#define PN532_RESET (3)  
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#define NODE_ADDRESS 2  // Change this unique address for each I2C slave node
#define PAYLOAD_SIZE 2 // Number of bytes  expected to be received by the master I2C node
byte nodePayload[PAYLOAD_SIZE];

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using 
// programming port to program the Zero!
#define Serial SerialUSB
#endif


void setup(void) {

  pinMode(8, OUTPUT);   //Define Group 1 output pin (Red LED)
  pinMode(7, OUTPUT);   //Define Group 2 output pin (Yellow LED)
    
//Start Serial Monitor and set baud rate
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();  //initiate NFC connection

//Search for PN532
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
//If firmware found, print to serial monitor
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

//Identify node address
  Serial.println("SLAVE SENDER NODE");
  Serial.print("Node address: ");
  Serial.println(NODE_ADDRESS);
  Serial.print("Payload size: ");
  Serial.println(PAYLOAD_SIZE);
  Serial.println("***********************");

  Wire.begin(NODE_ADDRESS);  // Activate I2C network
  Wire.onRequest(requestEvent); // Request attention of master node  
  
  
//configure board to read RFID tags
  nfc.SAMConfig();  
  Serial.println("Waiting for an ISO14443A Card ...");
  Serial.println("");
  
}


void loop(void) {
  uint8_t success;    // Create variable for detected card
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
//Create Groups for different responses  
  uint32_t Group1 = 1693588743; //Red LED
  uint32_t Group2 = 436264598;  //Yellow LED
  uint32_t cardidentifier = 0;  //Variable for current card being read

//Success boolean is defined when a card is found and read  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Print that card is found
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    if (uidLength == 4) // Mifare Classic Card
    {
      Serial.println("This is a Mifare Classic card (4 byte UID)");
      Serial.println("Trying to authenticate block 4 with default KEYA value");
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

      // Redefine success boolean if card is authenticated
	    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);
	  
      if (success)
      {
        Serial.println("Sector 1 (Blocks 4..7) has been authenticated");
        Serial.println("");
        uint8_t data[16];           
        
        //Turn UID into a single variable
        cardidentifier = uid[3];
        cardidentifier <<= 8; cardidentifier |= uid[2];
        cardidentifier <<= 8; cardidentifier |= uid[1];
        cardidentifier <<= 8; cardidentifier |= uid[0];
        Serial.print("Card Number  ");
        Serial.print(cardidentifier);
        Serial.print(":  ");

        if(cardidentifier == Group1)  //Flash Red LED
          {
            if (cardidentifier == Group2)
             {
                responseBoth();
                delay(10000);
             }
            else
              {
                response1();
                delay(10000);
              }
              
          }
        else if(cardidentifier == Group2) //Flash Yellow LED
          {
            if (cardidentifier == Group1)
              {
               responseBoth();
               delay(15000);
              }
            else
            {
            response2();
            delay(15000);
            }
            
          }
        else  //Flash both LEDs 3 times
          {
            noGroup();
          }
              
          // Pause before reading another card
          delay(1000);    
         }

      }
      
      else
        {
          Serial.println("Ooops ... authentication failed: Try another key?");
        }
  }

      delay(1000);
      nodePayload[0] = NODE_ADDRESS; // Sending Node address back. 
      nodePayload[1] = 0; //

}

    

void response1()  //Tag Group 1
  {   
    digitalWrite(8, HIGH);
    Serial.println("GROUP 1!!!");
    Serial.println();
    nodePayload[0] = NODE_ADDRESS; // Sending Node address back.   
    nodePayload[1] = 1; // Send Tag Group number
    delay(1000);
    
  }

void response2()  //Tag Group 2
  {
    digitalWrite(7, HIGH);
    Serial.println("Group 2!!");
    Serial.println();
    nodePayload[0] = NODE_ADDRESS; // Sending Node address back.   
    nodePayload[1] = 2; // Send Tag Group number
    delay(1000);    
  }

void noGroup()    // Send error message to Serial Monitor
  {
    Serial.println("No Group Identified");
    Serial.println("Please try another card.");
    Serial.println("");
   }

void requestEvent()
{
  Wire.write(nodePayload,PAYLOAD_SIZE);  
  Serial.print("Sensor value: ");  // for debugging purposes. 
  Serial.println(nodePayload[1]); // for debugging purposes. 
}       
