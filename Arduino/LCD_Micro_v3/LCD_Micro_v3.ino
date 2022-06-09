/**************************************************************************
Title: Canal & River Trust Pride Bike Real-time Monitor Micro
Author: James Kincell
Date: 24-MAY-2022
***************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <movingAvg.h>

// Macros for TFT Colours:
#define tftBLACK    0x0000
#define tftWHITE    0xFFFF
#define tftRED      0xF800
#define tftGREEN    0x07E0
#define tftBLUE     0x001F
#define tftCYAN     0x07FF
#define tftMAGENTA  0xF81F
#define tftYELLOW   0xFFE0
#define tftORANGE   0xFC00

// TFT Pinning:
#define TFT_CS    10
#define TFT_RST   8 
#define TFT_DC    9

// Analog Input Pinning:
#define SubAnalogIn A0
#define MidHiAnalogIn A1
#define LedAnalogIn A2
#define BattVoltageAnalogIn A3

//Button Input:
#define button 2

// Some Display Constants:
#define BarChartHeight 12
#define TextHeight 8
#define yGap 2
#define ReadingStartX 90
#define UnitStartX 123
#define DecimalPointX 102
byte TextWidth[] = {0,6,12};

// Coordinates and stuff:
byte firstLineYvals [] ={0, 30, 60, 90, 140};
byte BarChartYStartArray[sizeof(firstLineYvals)];
byte ReadingYStartArray[sizeof(firstLineYvals)];
byte lastpixelwidth [] = {1, 1, 1, 1};

// Variables for measurements:
int lastSubAmps;
int lastMidHiAmps;
int lastLedAmps;
int lastBattVolts;
int SubAmps;
int MidHiAmps;
int LedAmps;
int BattVolts;
int instantaneousPower;
int movingAvgPower;
int lastReading [] = {9999, 9999, 9999, 9999, 9999};

//Create tft instance:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
//Creatr Moving Average instance for Power:
movingAvg avgPower(10);

//SETUP:
void setup(void) {
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(2);
  tft.fillScreen(tftBLACK);
  tft.setTextWrap(false);

  avgPower.begin();

  pinMode(button, INPUT);

  //Create some dynamic arrays:
  for (byte i=0; i<sizeof(firstLineYvals)-1; i++){
    BarChartYStartArray[i] = firstLineYvals[i] + TextHeight + yGap;
    //Serial.print("Bar ");
    //Serial.println(BarChartYStartArray[i]);
    ReadingYStartArray[i] = firstLineYvals[i] + TextHeight + yGap + BarChartHeight + yGap;
    //Serial.print("Reading ");
    //Serial.println(ReadingYStartArray[i]);
  }
  
  //DRAW STATIC ITEMS:
  tft.setTextColor(tftWHITE);
  tft.setTextSize(1);
  
  //Bass Current:
  tft.setCursor(0, firstLineYvals[0]);
  tft.println("Sub Current:");
  tft.drawRect(0, BarChartYStartArray[0], tft.width(), BarChartHeight, tftWHITE);
  tft.setCursor(DecimalPointX, firstLineYvals[0]);
  tft.println(".");
  tft.setCursor(UnitStartX, firstLineYvals[0]);
  tft.println("A");

  //MidHigh Current:
  tft.setCursor(0, firstLineYvals[1]);
  tft.println("Mid/Hi Current:");
  tft.drawRect(0, BarChartYStartArray[1], tft.width(), BarChartHeight, tftWHITE);
  tft.setCursor(DecimalPointX, firstLineYvals[1]);
  tft.println(".");
  tft.setCursor(UnitStartX, firstLineYvals[1]);
  tft.println("A");

  //LED Current:
  tft.setCursor(0, firstLineYvals[2]);
  tft.println("LED Current:");
  tft.drawRect(0, BarChartYStartArray[2], tft.width(), BarChartHeight, tftWHITE);
  tft.setCursor(DecimalPointX, firstLineYvals[2]);
  tft.println(".");
  tft.setCursor(UnitStartX, firstLineYvals[2]);
  tft.println("A");

  //Battery Voltage:
  tft.setCursor(0, firstLineYvals[3]);
  tft.println("Batt. Voltage:");
  tft.drawRect(0, BarChartYStartArray[3], tft.width(), BarChartHeight, tftWHITE);
  tft.setCursor(DecimalPointX, firstLineYvals[3]);
  tft.println(".");
  tft.setCursor(UnitStartX, firstLineYvals[3]);
  tft.println("V");

  tft.setCursor(0, 120);
  tft.setTextSize(2);
  tft.println("Power:");
}
// END SETUP.

// MAIN LOOP:
void loop() {
  // Read Unscaled Analog Inputs:
  SubAmps = analogRead(SubAnalogIn);
  delay(100);
  MidHiAmps = analogRead(MidHiAnalogIn);
  delay(100);
  //LedAmps = analogRead(LedAnalogIn);
  delay(100);
  BattVolts = analogRead(BattVoltageAnalogIn);
  delay(100);

  //DEBUG STUFF
  Serial.print("Bass Reading:     ");
  Serial.println(SubAmps);
  Serial.print("Mid/High Reading: ");
  Serial.println(MidHiAmps);
  Serial.print("LED Reading:      ");
  Serial.println(LedAmps);
  Serial.print("Batt Reading:     ");
  Serial.println(BattVolts);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");

  //delay(1000);  

  

  //Scale the readings:
  //  values are x100, decimal point added by print function
  SubAmps = map(SubAmps,0,1023,0,100);
  MidHiAmps = map(MidHiAmps,0,1023,0,100);
  LedAmps = map(LedAmps,0,1023,0,100);
  BattVolts = map(BattVolts,0,1023,0,15);
  //Serial.print("Battery Voltage: ");
  //Serial.println(BattVolts);
   
  barChart(SubAmps, 0);
  PrintReadingText(SubAmps*100, 0, ReadingStartX, 1);
  
  barChart(MidHiAmps, 1);
  PrintReadingText(MidHiAmps*100, 1, ReadingStartX, 1);
  
  barChart(LedAmps, 2);
  PrintReadingText(LedAmps*10, 2, ReadingStartX, 1);
  
  barChart((BattVolts/15)*100, 3);
  PrintReadingText(BattVolts*100, 3, ReadingStartX, 1);

  //Power Calculation:
  instantaneousPower = (SubAmps + MidHiAmps + LedAmps) * BattVolts;
  movingAvgPower = avgPower.reading(instantaneousPower);

  PrintReadingText(movingAvgPower, 4, 0, 2);

  if(digitalRead(button) != HIGH){
    Serial.println("Button Pressed");
  }
  
}
// END MAIN LOOP.

/////////////////////
// FUNCTIONS:
/////////////////////

//Bar Chart Function:
void barChart(int pc, byte chartID){
  int pixelwidth = map(pc, 0, 100, 1, 126);
  if (lastpixelwidth[chartID] < pixelwidth){
    tft.fillRect(lastpixelwidth[chartID], BarChartYStartArray[chartID]+1, (pixelwidth-lastpixelwidth[chartID]+1), BarChartHeight-2 , tftBLUE);
  }else if (lastpixelwidth[chartID] > pixelwidth){
    tft.fillRect(pixelwidth, BarChartYStartArray[chartID]+1, lastpixelwidth[chartID]-pixelwidth+1, BarChartHeight-2 , tftBLACK);
  }
  lastpixelwidth[chartID] = pixelwidth;
}

//Print Readings Function - avoids re-printing unchanged values, nifty!
void PrintReadingText(int reading, byte readingID, byte xPos, byte textSize){
  //Separate the individual base10 digits:
  byte readingOnes = (reading%10);
  byte readingTens = ((reading/10)%10);
  byte readingHundreds = ((reading/100)%10);
  byte readingThousands = (reading/1000);
  byte lastOnes = (lastReading[readingID]%10);
  byte lastTens = ((lastReading[readingID]/10)%10);
  byte lastHundreds = ((lastReading[readingID]/100)%10);
  byte lastThousands = (lastReading[readingID]/1000);

  tft.setTextSize(textSize);
  
  //THOUSANDS
  if(readingThousands != lastThousands){
    tft.setCursor(xPos, firstLineYvals[readingID]);
    tft.setTextColor(tftBLACK);
    tft.println(lastThousands);
    if(readingThousands != 0){
      tft.setCursor(xPos, firstLineYvals[readingID]);
      tft.setTextColor(tftWHITE);
      tft.println(readingThousands);
    }
  }

  //HUNDREDS
  if(readingHundreds != lastHundreds){
    tft.setCursor(xPos+TextWidth[textSize], firstLineYvals[readingID]);
    tft.setTextColor(tftBLACK);
    tft.println(lastHundreds);
    tft.setCursor(xPos+TextWidth[textSize], firstLineYvals[readingID]);
    tft.setTextColor(tftWHITE);
    tft.println(readingHundreds);
  }

  //TENS
  if(readingTens != lastTens){
    tft.setCursor(xPos+TextWidth[textSize]*3, firstLineYvals[readingID]);
    tft.setTextColor(tftBLACK);
    tft.println(lastTens);
    tft.setCursor(xPos+TextWidth[textSize]*3, firstLineYvals[readingID]);
    tft.setTextColor(tftWHITE);
    tft.println(readingTens);
  }

  //ONES
  if(readingOnes != lastOnes){
    tft.setCursor(xPos+TextWidth[textSize]*4, firstLineYvals[readingID]);
    tft.setTextColor(tftBLACK);
    tft.println(lastOnes);
    tft.setCursor(xPos+TextWidth[textSize]*4, firstLineYvals[readingID]);
    tft.setTextColor(tftWHITE);
    tft.println(readingOnes);
  }

  //Save this as the last reading, for next function call.
  lastReading[readingID] = reading;
}
