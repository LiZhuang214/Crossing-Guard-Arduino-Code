/*
This is the code for the Master board:
Control traffic cycle and request signal from Slave boards
We used Arduino Mega 2560 Here
*/
//***********************************************************************************
#include <Wire.h>                  // Include the required Wire library for I2C#include <Wire.h>
#include <SD.h>                           //include SD module library
#define SD_ChipSelectPin 53                //define pin53 as CS pin
#include <TMRpcm.h>                       //include speaker control library
TMRpcm tmrpcm;                            //crete an object for speaker library
// Include the library code for Liquid Crystal Screen.
//Link for the library: http://wiki.sunfounder.cc/index.php?title=I%C2%B2C_LCD1602
#include <LiquidCrystal_I2C.h>
#define PAYLOAD_SIZE 2 // how many bytes to expect from each I2C salve node
#define NODE_MAX 6 // maximum number of slave nodes (I2C addresses) to probe
#define START_NODE 2 // The starting I2C address of slave nodes
#define NODE_READ_DELAY 1000 // Some delay between I2C node reads
int nodePayload[PAYLOAD_SIZE];
int Board2TagGroup;
int Board3TagGroup;
//************************************** Define the pins by names ************************
const int test1 = 34; //red LED that will light up when card for visual impaired is detected
const int test2 = 11; //yellow LED that will light up when card for mobility impaired is detected
//Traffic Light for the same direction of crossing signal
const int Red_PIN = 28;
const int Yellow_PIN = 29;
const int Green_PIN = 30;

//Traffic light for the other direction
const int Red_PIN2 = 22;
const int Yellow_PIN2 = 23;
const int Green_PIN2 = 24;
int timer = 500;    //A timer for the CountDown, to run the for loop 500 times, which turns out as 1 second.

LiquidCrystal_I2C lcd(0x27, 16, 1);   // set the LCD address to 0x27 for a 16 chars and 1 line display.

// Pin 2-8 is connected to the 7 segments of the display.
// We only need to count down from 10 to 0 so that only two digit L3 and L4 are used for this tutorial.
int pinA = 8;
int pinB = 7;
int pinC = 6;
int pinD = 5;
int pinE = 4;
int pinF = 3;
int pinG = 2;
int L3 = 13;
int L4 = 12;
//*********************************** photoresistor ************************** 
const int sensorPin = A0;
const int ledNight = 40;

// We'll also set up some global variables for the light level:

int lightLevel, low = 100, high = 0; //need calibration, change the number accordingly based on your environment
//***************************************************************************************

void setup()
{
  pinMode (test1, OUTPUT);
  pinMode (test2, OUTPUT);  
  Serial.begin(115200);
  Serial.print("Initializing SD card...");
  Serial.println("MASTER READER NODE");
  Serial.print("Maximum Slave Nodes: ");
  Serial.println(NODE_MAX);
  Serial.print("Payload size: ");
  Serial.println(PAYLOAD_SIZE);
  Serial.println("***********************");
  
  Wire.begin();        // Activate I2C link
  pinMode (53, OUTPUT);
  digitalWrite (53, HIGH);

if (!SD.begin(SD_ChipSelectPin)) {
   Serial.println("Card failed, or not present");
   // don't do anything more:
   return;
 }
 Serial.println("card initialized.");

//Traffic Lights
//Initialize the digital pins as outputs.
  pinMode(Red_PIN, OUTPUT);
  pinMode(Yellow_PIN, OUTPUT);
  pinMode(Green_PIN, OUTPUT);
  pinMode(Red_PIN2, OUTPUT);
  pinMode(Yellow_PIN2, OUTPUT);
  pinMode(Green_PIN2, OUTPUT);

//Liquid Crystal Screen
lcd.init();  //initialize the lcd.
  lcd.backlight();  //open the backlight.

//7 Digit Display
//Initialize the digital pins as outputs.
  pinMode(pinA, OUTPUT);     
  pinMode(pinB, OUTPUT);     
  pinMode(pinC, OUTPUT);     
  pinMode(pinD, OUTPUT);     
  pinMode(pinE, OUTPUT);     
  pinMode(pinF, OUTPUT);     
  pinMode(pinG, OUTPUT);   
  pinMode(L3, OUTPUT);  
  pinMode(L4, OUTPUT);  

  tmrpcm.speakerPin = 46;   //Define speaker pin. The speaker library is using pin9.
  pinMode(ledNight, OUTPUT);
}

//***************************************************************************************************************
// Define the sequence of the pattern in this mini intersection.
// When the computer gets to the end of the loop() function, it starts loop() again.
//***************************************************************************************************************
void loop()
{
 RequestSignal (); //Request signal from slave boards
//*********************************************   Visual Impaired Extend traffic light and play audio   **************************
if (Board2TagGroup == 1)
    {
  Serial.println ("Group1");
  digitalWrite (test1, HIGH);
  digitalWrite (test2, LOW);
  
//Light up LED when light intensity is low. Check light intensity value, calibration needed
  Serial.print ("light intensity:");  
  Serial.println (analogRead(sensorPin));
  lightLevel = analogRead(sensorPin);
  void NightLED();
  delay(500);                 // wait for 1/2 second to start blinking

//Audio waiting
    tmrpcm.setVolume(5);         // Set the volume as 5.           
    tmrpcm.play("Wait2.wav");
      Serial.println("Do not cross"); 

//One direction starts the traffic flow, the other direction holds the flow. 
  digitalWrite(Green_PIN2, HIGH);   // turn the LED Green 2 on
  digitalWrite(Red_PIN, HIGH);   // turn the LED Red on
  lcd.setCursor(3, 0); // set the cursor to column 3, line 0
  lcd.print("DO NOT CROSS");  // Print a message to the LCD
  delay(6000);                       // wait for 6 seconds (I shorten the real world traffic flow time for display purpose)
  digitalWrite(Green_PIN2, LOW);   // turn the LED Green 2 off
  digitalWrite(Red_PIN, LOW);   // turn the LED Red off

//Transition: yellow lights turn on and off 
  digitalWrite(Yellow_PIN, HIGH);    // turn the yellow LED on
  digitalWrite(Yellow_PIN2, HIGH);    // turn the yellow LED 2 on
  delay(1000);                       // wait for a second
  digitalWrite(Yellow_PIN, LOW);    // turn the yellow LED off
  digitalWrite(Yellow_PIN2, LOW);    // turn the yellow LED 2 off

//play audio
    tmrpcm.play("Crossing.wav"); 
    tmrpcm.loop(1);  
    Serial.println ("Cross");

//Switch Traffic flow 
//One direction starts the traffic flow, the other direction holds the flow. 
  lcd.setCursor(3, 0); // set the cursor to column 3, line 0
  lcd.print("                     ");  // Print a message to the LCD (delete old message)    
  delay(1);              
//Print new message 
  lcd.setCursor(6, 0); // set the cursor to column 6, line 0
  lcd.print("CROSS");  // Print a message to the LCD
  digitalWrite(Red_PIN2, HIGH);   // turn the red LED 2 on 
  digitalWrite(Green_PIN, HIGH);   // turn the green LED on 
  delay (5000);
  //Start Count Down for pedestrian crossing     
    tmrpcm.loop(0);
    AudibleCountDown();
    RequestBroad3 (); //Request signal from slave board 3
    Group1Check(); //check whether the person reaches the other corner
    tmrpcm.volume(0);
    }
//*********************************************   Mobility Impaired Extend traffic light only   *********************************
if (Board2TagGroup == 2)
    {
      Serial.println ("Group2");
      digitalWrite (test1, LOW);
      digitalWrite (test2, HIGH);

      Serial.print ("light intensity:");
      Serial.println (analogRead(sensorPin));
      //digitalWrite (ledNight, HIGH);
      lightLevel = analogRead(sensorPin);
      void NightLED();

      Trafficlight();
      delay(10000);  //Wait for 10 seconds

      CountDown15();
      //Pedestrian with mobility impairment does not cross when count down ends
      RequestBroad3 ();
      Group2Check();
    }
//*********************************************   Regular intersection no RFID   *********************************
if (Board2TagGroup == 0)
   {
    Serial.println ("NoGroup");
    digitalWrite (test1, LOW);
    digitalWrite (test2, LOW);
    digitalWrite (ledNight, LOW);
    Trafficlight();
    delay(5000);  //Wait for 5 seconds

    //Start Count Down for pedestrian crossing  
    CountDown();
    CrossingEnd();
   }    
//********************************************** Functions *******************************************************
void NightLED() //Light up the ground LED at night
  if (lightLevel > low)
  {
    digitalWrite (ledNight, HIGH);
  }
  else
  {
  digitalWrite (ledNight,LOW);
  }
}

//Since we only deal with one direction crossing, assume the other direction do not have RFID intervention in this scenario//
void Trafficlight()
{
  delay(500);                 // wait for 1/2 second to start blinking
//One direction starts the traffic flow, the other direction holds the flow. 
  digitalWrite(Green_PIN2, HIGH);   // turn the LED Green 2 on
  digitalWrite(Red_PIN, HIGH);   // turn the LED Red on
  lcd.setCursor(3, 0); // set the cursor to column 3, line 0
  lcd.print("DO NOT CROSS");  // Print a message to the LCD
  delay(6000);                       // wait for 6 seconds (I shorten the real world traffic flow time for display purpose)
  digitalWrite(Green_PIN2, LOW);   // turn the LED Green 2 off
  digitalWrite(Red_PIN, LOW);   // turn the LED Red off
//Transition: yellow lights turn on and off 
  digitalWrite(Yellow_PIN, HIGH);    // turn the yellow LED on
  digitalWrite(Yellow_PIN2, HIGH);    // turn the yellow LED 2 on
  delay(1000);                       // wait for a second
  digitalWrite(Yellow_PIN, LOW);    // turn the yellow LED off
  digitalWrite(Yellow_PIN2, LOW);    // turn the yellow LED 2 off
//Switch Traffic flow 
//One direction starts the traffic flow, the other direction holds the flow. 
  lcd.setCursor(3, 0); // set the cursor to column 3, line 0
  lcd.print("                         ");  // Print a message to the LCD    
  delay(1);              
//Print new message 
  lcd.setCursor(6, 0); // set the cursor to column 6, line 0
  lcd.print("CROSS");  // Print a message to the LCD
  digitalWrite(Red_PIN2, HIGH);   // turn the red LED 2 on 
  digitalWrite(Green_PIN, HIGH);   // turn the green LED on 
}


//Pedestrian Crossing count down for last 10 seconds: from 10 to 0
void CountDown()
{
for (int i=0; i<timer; i++)
  {
  //10
  digitalWrite(L3, LOW);
  digitalWrite(L4, HIGH);
  //0
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, HIGH);   
  delay(1);                
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  //1
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);   
  delay(1);                   // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //9
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW); 
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);                  // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //8
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);  
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);                 // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //7
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW); 
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //6
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, HIGH);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //5
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, HIGH);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);               //The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //4
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //3
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //2
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, HIGH);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //1
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //0
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, HIGH);     
  delay(2);               // The for loop, for running the program 500 times.
  }

 //Turn off the 7 digit display
  digitalWrite(L3, LOW);
  digitalWrite(L4, LOW);
  delay(2);               
}


void AudibleCountDown()
{
tmrpcm.setVolume(5);
tmrpcm.play("10.wav");
for (int i=0; i<timer; i++)
  {
  //10
  digitalWrite(L3, LOW);
  digitalWrite(L4, HIGH);
  //0
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, HIGH);   
  delay(1);               
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  //1
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);   
  delay(1);                   // The for loop, for running the program 500 times.
  }

tmrpcm.play("9.wav");
for (int i=0; i<timer; i++)
  {
  //9
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);                  // The for loop, for running the program 500 times.
  }

tmrpcm.play("8.wav");
for (int i=0; i<timer; i++)
  {
  //8
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);                 // The for loop, for running the program 500 times.
  }

tmrpcm.play("7.wav");
for (int i=0; i<timer; i++)
  {
  //7
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);     
  delay(2);               // The for loop, for running the program 500 times.
  }

tmrpcm.play("6.wav");
for (int i=0; i<timer; i++)
  {
  //6
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, HIGH);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

tmrpcm.play("5.wav");
for (int i=0; i<timer; i++)
  {
  //5
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, HIGH);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);               //The for loop, for running the program 500 times.
  }

tmrpcm.play("4.wav");
for (int i=0; i<timer; i++)
  {
  //4
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }
tmrpcm.play("3.wav");
for (int i=0; i<timer; i++)
  {
  //3
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

tmrpcm.play("2.wav");
for (int i=0; i<timer; i++)
  {
  //2
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, HIGH);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

tmrpcm.play("1.wav");
for (int i=0; i<timer; i++)
  {
  //1
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);     
  delay(2);               // The for loop, for running the program 500 times.
  }

//tmrpcm.play("0.wav");
for (int i=0; i<timer; i++)
  {
  //0
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, HIGH);     
  delay(2);               // The for loop, for running the program 500 times.
  }

 //Turn off the 7 digit display
  digitalWrite(L3, LOW);
  digitalWrite(L4, LOW);
  delay(2);               
}


void CrossingEnd()
{
  //Pedestrian Crossing finish
  digitalWrite(Red_PIN2, LOW);   // turn the red LED 2 off
  digitalWrite(Green_PIN, LOW);   // turn the green LED off
  digitalWrite(Yellow_PIN, HIGH);    // turn the yellow LED on
  digitalWrite(Yellow_PIN2, HIGH);    // turn the yellow LED 2 on
  delay(1000);                       // wait for a second
  digitalWrite(Yellow_PIN, LOW);    // turn the yellow LED off
  digitalWrite(Yellow_PIN2, LOW);    // turn the yellow LED 2 off
}


//Pedestrian Crossing count down for last 15 seconds: from 15 to 0
void CountDown15()
{
for (int i=0; i<timer; i++)
  {
  //15
  digitalWrite(L3, LOW);
  digitalWrite(L4, HIGH);

  //5
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, HIGH);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);             
  delay(1);               
  
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);

  //1
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);   
  delay(1);                   // The for loop, for running the program 500 times.
  }

  for (int i=0; i<timer; i++)
  {
  //14
  digitalWrite(L3, LOW);
  digitalWrite(L4, HIGH);
  //4
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);         
  delay(1);               
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  //1
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);   
  delay(1);                   // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //13
  digitalWrite(L3, LOW);
  digitalWrite(L4, HIGH);
  //3
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, LOW);        
  delay(1);               
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  //1
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);   
  delay(1);                   // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //12
  digitalWrite(L3, LOW);
  digitalWrite(L4, HIGH);
  //2
 digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, HIGH);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, LOW);     
  delay(1);                
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  //1
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);   
  delay(1);                   // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //11
  digitalWrite(L3, LOW);
  digitalWrite(L4, HIGH);
  //1
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);  
  delay(1);                 
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  //1
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);   
  delay(1);                   // The for loop, for running the program 500 times.
  }
  
for (int i=0; i<timer; i++)
  {
  //10
  digitalWrite(L3, LOW);
  digitalWrite(L4, HIGH);
  //0
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, HIGH);   
  delay(1);               
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  //1
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);   
  delay(1);                   // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //9
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);                  // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //8
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW); 
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);                 // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //7
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW); 
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //6
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, HIGH);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //5
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, HIGH);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);               //The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //4
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //3
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //2
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, HIGH);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, LOW);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //1
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, HIGH);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, HIGH);   
  digitalWrite(pinE, HIGH);   
  digitalWrite(pinF, HIGH);   
  digitalWrite(pinG, HIGH);     
  delay(2);               // The for loop, for running the program 500 times.
  }

for (int i=0; i<timer; i++)
  {
  //0
  digitalWrite(L3, HIGH);
  digitalWrite(L4, LOW);
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, HIGH);     
  delay(2);               // The for loop, for running the program 500 times.
  }
 //Turn off the 7 digit display
  digitalWrite(L3, LOW);
  digitalWrite(L4, LOW);
  delay(2);               
}



void Group1Check()
{
  if (Board2TagGroup != Board3TagGroup)
    {
          Serial.println("Extra Crossing for visual impaired");
          for (int i = 0; i < 3; i++)
            {             
          //Print new message 
           tmrpcm.setVolume(5);
           tmrpcm.play("Finish.wav"); 
           lcd.setCursor(1, 0); // set the cursor to column 3, line 0
           lcd.print("Finish Crossing");  // Print a message to the LCD
           delay (1000);
           lcd.setCursor(1, 0); // set the cursor to column 3, line 0
           lcd.print("                        ");  // Print a message to the LCD    
           delay(1000);      
           tmrpcm.volume(0);
            }
            CrossingEnd();
        }
        else
        {
         CrossingEnd();     
        }
}

void Group2Check()
{
  if (Board2TagGroup != Board3TagGroup)
        {
          Serial.println("Extra Crossing for mobility impaired");
          for (int i = 0; i < 3; i++)
            {             
          //Print new message 
           lcd.setCursor(1, 0); // set the cursor to column 3, line 0
           lcd.print("Finish Crossing");  // Print a message to the LCD
           delay (1000);
           lcd.setCursor(1, 0); // set the cursor to column 3, line 0
           lcd.print("                            ");  // Print a message to the LCD    
           delay(1000);   
            }
           CrossingEnd();  
        }
        else
        {
      CrossingEnd();
        }
}

void RequestSignal ()   //change this to only request from broad 2
{
    for (int nodeAddress = START_NODE; nodeAddress <= NODE_MAX; nodeAddress++) { // we are starting from Node address 2
    Wire.requestFrom(nodeAddress, PAYLOAD_SIZE);    // request data from node#
    if(Wire.available() == PAYLOAD_SIZE) {  // if data size is avaliable from nodes
      for (int i = 0; i < PAYLOAD_SIZE; i++) nodePayload[i] = Wire.read();  // get nodes data
      for (int j = 0; j < PAYLOAD_SIZE; j++) {Serial.println(nodePayload[j]);   // print nodes data
      if(nodeAddress == 2){
        Board2TagGroup = nodePayload[j];
      }
      else if(nodeAddress == 3){
        Board3TagGroup = nodePayload[j];
      }
      }
      Serial.println("*************************");      
      }
    }
    delay(NODE_READ_DELAY);

    Serial.print("Board 2 Tag Group: ");
    Serial.println(Board2TagGroup);
    Serial.print("Board 3 Tag Group: ");
    Serial.println(Board3TagGroup); 
}


void RequestBroad3 ()//change this to only request from broad 3
{
    //for (int nodeAddress = START_NODE; nodeAddress <= NODE_MAX; nodeAddress++) { // we are starting from Node address 2
    Wire.requestFrom(3, PAYLOAD_SIZE);    // request data from node#
    if(Wire.available() == PAYLOAD_SIZE) {  // if data size is avaliable from nodes
      for (int i = 0; i < PAYLOAD_SIZE; i++) nodePayload[i] = Wire.read();  // get nodes data
      for (int j = 0; j < PAYLOAD_SIZE; j++) {Serial.println(nodePayload[j]);   // print nodes data
        Board3TagGroup = nodePayload[j];
      }
      Serial.println("*************************");      
    }
    delay(NODE_READ_DELAY);
    Serial.print("Board 2 Tag Group: ");
    Serial.println(Board2TagGroup);
    Serial.print("Board 3 Tag Group: ");
    Serial.println(Board3TagGroup); 
}
