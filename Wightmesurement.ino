#include "HX711.h" // Include the HX711 library for interfacing with the load cell
#include <UTFT.h> // Include the UTFT library for managing the LCD display
#include <SD.h> // Include the SD library to handle SD card operations
#include <SoftwareSerial.h> // Include the SoftwareSerial library for bluetooth

#define buzzer 43

// Create a SoftwareSerial object for communication with the HC-08 Bluetooth module
SoftwareSerial hc08(15,14); // RX pin, TX pin;

HX711 scale; // Declare an HX711 object to manage the load cell amplifier

// Define pins for the HX711 module
uint8_t dataPin = 33;
uint8_t clockPin = 30;

const int chipSelect = 40; // Define the chip select pin for the SD card module
File myFile;// File object to manage files on the SD card

// Create a UTFT object to control the LCD screen with specific pin configurations
UTFT myGLCD(ST7735S_4L_80160,11,13,10,8,9);

extern uint8_t BigFont[];// Declare an external font array for the LCD display

long count = 0; // General-purpose counter
float scale1; // Variable to store the scale factor of the HX711
uint32_t offset; // Variable to store the tare offset value
int bread = 0; // Variable to store button states

// Function declaration for generating a buzz sound
void buzz();

void setup()
{
  Serial.begin(115200);  // Start Serial communication at 115200 baud for debugging
  Serial.println();      // Print a newline to format the output on Serial Monitor
  hc08.begin(9600);      // Start communication with HC-08 Bluetooth module at 9600 baud

  pinMode(buzzer, OUTPUT); // Set the buzzer pin as an output

  while (!Serial);  // Wait for Serial to initialize

   // Initialize the SD card and check if it's working
  Serial.print("Initializing SD card...");
  hc08.write("Initializing SD card...");
  hc08.write("\n");

  if (!SD.begin(chipSelect)) { // Check if the SD card initialization fails
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
    while (true); // Halt the program if the SD card is not initialized
  }

  Serial.println("initialization done.");
  hc08.write("initialization done.\n");

  pinMode(3, INPUT); // Set button pin (pin 3) as an input

  randomSeed(analogRead(0)); // Seed the random number generator with analog noise
  myGLCD.InitLCD();          // Initialize the LCD display
  myGLCD.setFont(BigFont);   // Set the font for the LCD display
  myGLCD.clrScr();           // Clear the screen

  // Initialize the HX711 scale
  scale.begin(dataPin, clockPin);
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10)); // Display the initial weight value on Serial Monitor
  myGLCD.print("Scale", LEFT, 1);
  myGLCD.print("Ready", LEFT, 10000);
  delay(3000);

  // Prompt user to empty the scale for taring
  myGLCD.print("Empty" , LEFT, 1);
  myGLCD.print("the scale", LEFT, 10000);
  delay(3000);
  myGLCD.clrScr();
  myGLCD.print("press" , LEFT, 1);
  myGLCD.print("Button", LEFT, 10000);
  myGLCD.print("to tare" , LEFT, 20000);
  Serial.println("\nEmpty the scale, press button to continue");
  hc08.write("Empty the scale, press button to continue\n");

  // Wait for button press to perform taring
  here:
  bread = digitalRead(3); // Read the button state
  if(bread == HIGH){ //if button is pressed
    myGLCD.clrScr();
    scale.tare(20); // Perform taring using 20 readings
    myGLCD.print("Taring...", LEFT, 10000);
    hc08.write("Taring\n");
    buzz(); //generate sound to show button is pressed
    myGLCD.clrScr();
    offset = scale.get_offset(); // Store the tare offset value
    myGLCD.print("Tare", LEFT, 1);
    myGLCD.print("Done", LEFT, 10000);
    hc08.write("Tare Done\n");
  }
  else{
    goto here; // Keep checking for button press if not pressed
  }
  delay(3000);
  
  // Prompt user to place a calibration weight
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));
  bread = 0;
  myGLCD.clrScr();
  myGLCD.print("Put 2.5" , LEFT, 1);
  myGLCD.print("lbs Weight" , LEFT, 10000);
  myGLCD.print("on the scale" , LEFT, 20000);
  delay(3000);
  myGLCD.clrScr();
  myGLCD.print("press" , LEFT, 1);
  myGLCD.print("Button to", LEFT, 10000);
  myGLCD.print("callibrate" , LEFT, 20000);
  Serial.println("\nPut 2.5lbs Weight on the scale, press button to continue");
  hc08.write("Put 2.5lbs Weight on the scale, press button to continue\n");
  uint32_t weight = 1134; // Set the calibration weight (2.5 lbs in grams)

  // Wait for button press to perform calibration
  there:
  bread = digitalRead(3);
  if(bread == HIGH){
    myGLCD.clrScr();
    scale.calibrate_scale(weight, 20); // Calibrate the scale with the specified weight
    myGLCD.print("Callibrating", LEFT, 1);
    myGLCD.print("...", LEFT, 10000);
    hc08.write("Calibrating...\n");
    buzz();
    scale1 = scale.get_scale(); // Store the scale factor
    myGLCD.clrScr();
    myGLCD.print("Callibration", LEFT, 1);
    myGLCD.print("done", LEFT, 10000);
    hc08.write("Calibration Done\n");
    delay(3000);
  }
  else{
    goto there; // Keep checking for button press if not pressed
  }

  // Finalize scale configuration
  scale.set_offset(offset); // Apply the stored offset
  scale.set_scale(scale1); // Apply the calculated scale factor
  myGLCD.clrScr();
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));

  // Notify user that the scale is calibrated
  myGLCD.print("Scale is " , LEFT, 1);
  myGLCD.print("calibrated," , LEFT, 10000);
  delay(3000);
  myGLCD.clrScr();
  myGLCD.print("press" , LEFT, 1);
  myGLCD.print("Button to", LEFT, 10000);
  myGLCD.print("continue" , LEFT, 20000);
  Serial.println("\nScale is calibrated, press button to continue");
  hc08.write("Scale is calibrated, press button to continue\n");
  bread=0; // Reset the button state

  //wait for button press
  wait:
  bread = digitalRead(3);
  if(bread == HIGH){
    buzz();
  }
  else{
    goto wait;
  }
  myGLCD.clrScr();
}


void loop()
{
  static long lastWeight = 0; // Variable to store the last measured weight
  static unsigned long lastChangeTime = millis(); // Timestamp of the last significant weight change
  long currentWeight; // Variable to store the current weight

  // Initial instruction to the user
  myGLCD.clrScr();
  myGLCD.print("Press button", LEFT, 1);
  myGLCD.print("to save weight", LEFT, 10000);
  hc08.write("Press button to save Weight\n");
  delay(5000); // Display message for 5 seconds

  // Start continuous weight display
  while (true) {
    // Clear previous display
    myGLCD.clrScr();
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("weight=", LEFT, 1);
    hc08.write("Weight = ");
    // Get current weight
    currentWeight = scale.get_units(5);
    myGLCD.printNumI(currentWeight, LEFT, 10000);
    myGLCD.print("g", RIGHT, 10000);
    Serial.print("UNITS: ");
    Serial.println(currentWeight);
    hc08.write(currentWeight);
    hc08.write(" g\n");
    // Check if the weight changed significantly
    if (abs(currentWeight - lastWeight) > 10) {
      lastChangeTime = millis(); // Reset the timer if weight changes
      lastWeight = currentWeight;
    }

    // Check if button is pressed to save weight
    int bread = digitalRead(3); // Button for saving weight
    if (bread == HIGH) {
      buzz();
      myFile = SD.open("test.txt", FILE_WRITE);
      if (myFile) {
        Serial.print("Writing to test.txt...");
        myFile.println(currentWeight);
        // close the file:
        myFile.close();
        Serial.println("done.");
      } else {
      // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
      }
      Serial.println("Weight saved.");
      hc08.write("Weight Saved\n");
      myGLCD.clrScr();
      myGLCD.print("Weight", LEFT, 1);
      myGLCD.print("Saved!", LEFT, 10000);
      delay(2000); // Show saved message for 2 seconds
    }

    // Delay between readings
    delay(2000);

    // Save offset and scale settings
    scale.set_offset(offset);
    scale.set_scale(scale1);

    // Check if 2 minutes have passed without significant weight change
    if (millis() - lastChangeTime > 120000) { // 2 minutes = 120000 ms
      hc08.write("Turning off");
      buzz();
      buzz();
      buzz();
      myGLCD.clrScr();
      myGLCD.print("No change", LEFT, 1);
      myGLCD.print("in weight.", LEFT, 10000);
      myGLCD.print("Turning off", LEFT, 20000);
      delay(5000); // Show message for 5 seconds
      myGLCD.clrScr();
      myGLCD.print("Goodbye!", LEFT, 1);
      delay(2000); // Show goodbye message for 2 seconds
      myGLCD.clrScr(); // Turn off the display
      while (1); // End the program
    }
  }
}

void buzz(){
  pinMode(buzzer, OUTPUT);
  tone(buzzer,3);
  delay(150);
  tone(buzzer,6);
  delay(150);
  tone(buzzer,10);
  delay(150);
  noTone(buzzer);
}