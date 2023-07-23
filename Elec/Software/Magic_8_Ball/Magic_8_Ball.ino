/*
*  Application: Magic 8 Ball
*  Function: main
*  Date: 7/22/2023
*  Author: misperry
*  Webside: http://www.youtube.com/misperry
*
*  Description:  This is a port from Adafruits librarys located here: https://learn.adafruit.com/1-8-tft-display/graphics-library
*  This firmware will use a 1.8" tft dispay with onboard sdcard to randomly display the magic 8 ball answers.
*  This firmware is specific to the Arduino nano 33 BLE sense rev2 board because it makes use of the onboard 3-axis accelerometer
*  The code will read the Accelerometer to detect a shake and then generate randome number to then coorolate with a picture number
*  stored on the micro SD card to display an answer on the tft screen.
*
*  Libraries Needed:
*
*  - Adafruit GFX
*  - Adafruit ST7735 and ST7789
*  - Adafruit SPIFlash
*  - Adafruit imageReader
*  - Arduino BMI270_BMM150
*  
*  
*/

#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7735.h>      // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions
#include <string.h>               // Used for building string to search for picture
#include "Arduino_BMI270_BMM150.h"// Used to get accelerometer data

// Comment out the next line to load from SPI/QSPI flash instead of SD card:
#define USE_SD_CARD

// TFT display and SD card share the hardware SPI interface, using
// 'select' pins for each to identify the active device on the bus.

#define SD_CS    4 // SD card select pin
#define TFT_CS  10 // TFT select pin
#define TFT_DC   8 // TFT display/command pin
#define TFT_RST  9 // Or set to -1 and connect to Arduino RESET pin

#if defined(USE_SD_CARD)
  SdFat                SD;         // SD card filesystem
  Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
#else
  // SPI or QSPI flash filesystem (i.e. CIRCUITPY drive)
  #if defined(__SAMD51__) || defined(NRF52840_XXAA)
    Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS,
      PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
  #else
    #if (SPI_INTERFACES_COUNT == 1)
      Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
    #else
      Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
    #endif
  #endif
  Adafruit_SPIFlash    flash(&flashTransport);
  FatVolume        filesys;
  Adafruit_ImageReader reader(filesys); // Image-reader, pass in flash filesys
#endif

Adafruit_ST7735      tft    = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;

void setup(void) {

  Serial.begin(9600);

  tft.initR(INITR_BLACKTAB); // Initialize screen

  // The Adafruit_ImageReader constructor call (above, before setup())
  // accepts an uninitialized SdFat or FatVolume object. This MUST
  // BE INITIALIZED before using any of the image reader functions!
  Serial.print(F("Initializing filesystem..."));
#if defined(USE_SD_CARD)
  // SD card is pretty straightforward, a single call...
  if(!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }
#else
  // SPI or QSPI flash requires two steps, one to access the bare flash
  // memory itself, then the second to access the filesystem within...
  if(!flash.begin()) {
    Serial.println(F("flash begin() failed"));
    for(;;);
  }
  if(!filesys.begin(&flash)) {
    Serial.println(F("filesys begin() failed"));
    for(;;);
  }
#endif
  Serial.println(F("OK!"));

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // Fill screen blue. Not a required step, this just shows that we're
  // successfully communicating with the screen.
  tft.fillScreen(ST7735_BLUE);
  delay(2000);
}

void loop() {

  ImageReturnCode stat; // Status from image-reading functions
  String loadPic = "/pic";
  char arr[16];
  float x, y, z;

  //Generate a random number to represent a random response
  int randNumber;

  //Get accelerometer data  
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);

    //check the accelerometer data for shaking
    if (x > abs(2.5) || y > abs(2.5) || z > abs(2.5))
    {
      //Generate random number
      randNumber = random(1,20);
      //build the file name
      loadPic = loadPic + randNumber + ".bmp";
      loadPic.toCharArray(arr, sizeof(arr));

        // Load full-screen BMP file 'parrot.bmp' at position (0,0) (top left).
      // Notice the 'reader' object performs this, with 'tft' as an argument.
      Serial.print(F("Loading pic1.bmp to screen..."));
      stat = reader.drawBMP(arr, tft, 0, 0);
      reader.printStatus(stat);   // How'd we do?
      delay(5000);
    }
    // check to see if the shaking has stopped
    else if (x < abs(2) || y < abs(2) || z < abs(2))
    {
      //clear the screen of images
      tft.fillScreen(ST7735_BLACK);
      delay(1);
    }
    else
    {
      //in any other case clear the screen of images
      tft.fillScreen(ST7735_BLACK);
    }
  }
}
