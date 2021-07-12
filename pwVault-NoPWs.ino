// #include "Keyboard.h"
#include <ProTrinketKeyboard.h>
#include "Wire.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "avr/pgmspace.h"

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 lcd = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

#define NAMLEN 15
#define PASLEN 31
#define UNMLEN 31
#define NSITES 31

const byte BUTTON1=8,BUTTON2=9,BUTTON3=0,BUTTON4=6;  // Button 4,3 changed from pin 7,10 to 6,0 for pro trinket

const byte pinCode[]={3,2,2,4,1};


const static char siteName[][NAMLEN] PROGMEM={
  "Starbucks",
  "Sleepnumber",
  "Adafruit",
  "Mouser",
  "Particle",
  "World of Tanks",
  "BangGood",
  "Amazon",
  "Credit Card",
  "Pax Enforcer",
  "Cut Splice",
  "Netflix",
  "Twitter",
  "LinkedIn",
  "Flixster",
  "FitBit Dashboa",
  "Router",
  "Paypal",
  "Alumni",
  "Fast Lane",
  "LOTRO",
  "NewEgg",
  "Steam",
  "Barnes & Noble",
  "Twitch",
  "Filters-Now",
  "IFTTT",
  "SWGEMU",
  "goodreads",
  "US Postal Serv",
  "Facebook"
};

// const static char* userName[]={
const static char userName[][UNMLEN] PROGMEM={
  "Starbucks-UserN",
  "Sleepnumber-UserN",
  "Adafruit-UserN",
  "Mouser-UserN",
  "Particle-UserN",
  "World of Tanks-UserN",
  "BangGood-UserN",
  "Amazon-UserN",
  "Credit Card-UserN",
  "Pax Enforcer-UserN",
  "Cut Splice-UserN",
  "Netflix-UserN",
  "Twitter-UserN",
  "LinkedIn-UserN",
  "Flixster-UserN",
  "FitBit Dashboa-UserN",
  "Router-UserN",
  "Paypal-UserN",
  "Alumni-UserN",
  "Fast Lane-UserN",
  "LOTRO-UserN",
  "NewEgg-UserN",
  "Steam-UserN",
  "Barnes & Noble-UserN",
  "Twitch-UserN",
  "Filters-Now-UserN",
  "IFTTT-UserN",
  "SWGEMU-UserN",
  "goodreads-UserN",
  "US Postal Serv-UserN",
  "Facebook"
};

const static char passWord[][PASLEN] PROGMEM={
  "StarbucksPW",
  "SleepnumberPW",
  "AdafruitPW",
  "MouserPW",
  "ParticlePW",
  "WorldofTanksPW",
  "BangGoodPW",
  "AmazonPW",
  "CreditCardPW",
  "PaxEnforcerPW",
  "CutSplicePW",
  "NetflixPW",
  "TwitterPW",
  "LinkedInPW",
  "FlixsterPW",
  "FitBitDashboaPW",
  "RouterPW",
  "PaypalPW",
  "AlumniPW",
  "FastLanePW",
  "LOTROPW",
  "NewEggPW",
  "SteamPW",
  "Barnes&NoblePW",
  "TwitchPW",
  "Filters-NowPW",
  "IFTTTPW",
  "SWGEMUPW",
  "goodreadsPW",
  "USPostalServPW",
  "FacebookPW"
};


byte pinDigit;
bool pinMatch,pinError,userNameSent,selected,bothSent,newKeyNeeded,screenScrolled,sendTab;
short indx,contrast;
unsigned long scrollTimeStart,contrastTimeStart,timeOutEnd,timeOutDelay=300000;
char myChar;
uint8_t testS;
byte srtPointer[NSITES],newSrtPoint[NSITES];
byte counts[128];

void fillScreen(short indx,short nLines) {
  short iOffset,testLen;

  lcd.clearDisplay();

  for (short i=0; i<nLines; i++) {
    lcd.setTextColor(BLACK);
    if (i == 2) lcd.setTextColor(WHITE,BLACK);
    iOffset = indx + i;
    if (nLines == 6) iOffset = iOffset - 2;
    testLen = sizeof(siteName)/NAMLEN-1;
    if (iOffset > testLen) iOffset = iOffset - sizeof(siteName)/NAMLEN;
    if (iOffset < 0) iOffset = iOffset + sizeof(siteName)/NAMLEN;
    
    for (short k=0;k<NAMLEN-1;k++) {
      myChar = pgm_read_byte_near(siteName[srtPointer[iOffset]] + k);
      if (myChar != '\0') lcd.print(myChar);
    }
    lcd.println(" ");
  }
  lcd.display();
  
  lcd.setTextColor(BLACK);
  return;
}

void pollDelay(int tDelay) {

  // the poll function must be called at least once every 10 ms
  // or cause a keystroke
  // if it is not, then the computer may think that the device
  // has stopped working, and give errors

  for (int i=0; i<(tDelay/5); i++) {
    TrinketKeyboard.poll();
    delay(5);
  }
  TrinketKeyboard.poll();
  delay(tDelay%5);
  TrinketKeyboard.poll();
  return;
}

void setup() {
  // put your setup code here, to run once:
//  Serial.begin(9600);
  
  lcd.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  contrast = 60;
  lcd.setContrast(contrast);

  pinMode(BUTTON1,INPUT_PULLUP);
  pinMode(BUTTON2,INPUT_PULLUP);
  pinMode(BUTTON3,INPUT_PULLUP);
  pinMode(BUTTON4,INPUT_PULLUP);
 
  lcd.setTextSize(1);
  lcd.setTextColor(BLACK);
  lcd.setTextWrap(false);

  lcd.clearDisplay();
  lcd.display();
  lcd.setCursor(0,0);
  lcd.println("Attempting");
  lcd.println("to");
  lcd.println("Initalize...");
  lcd.display();
  TrinketKeyboard.begin();
  while (((int)TrinketKeyboard.isConnected()) == 0) {
    TrinketKeyboard.poll();
//    delay(100);
  }

  lcd.clearDisplay();
  lcd.display();
  lcd.setCursor(0,0);
  lcd.println("Sorting Names");
  lcd.display();
  TrinketKeyboard.poll();
  pollDelay(1000);

// create pointer array and then sort it alphabetically

  for (indx = 0; indx < NSITES; indx++) {
    srtPointer[indx] = indx;
  }
  TrinketKeyboard.poll();

  for (short decPlace = NAMLEN-2; decPlace >= 0; decPlace--) {
    for (short i = 0; i<128; i++) {
      counts[i] = -1;
    }
    TrinketKeyboard.poll();

    for (indx = 0; indx < NSITES; indx++) {
      for (short i = pgm_read_byte_near(siteName[indx] + decPlace); i<128; i++) {
          counts[i]+=1;
      }
      TrinketKeyboard.poll();
    }

    for (short i = NSITES-1; i>=0; i--) {
      indx = pgm_read_byte_near(siteName[srtPointer[i]] + decPlace);
      if (indx < 0) indx = 0;
      if (indx > 127) indx = 127;
      if (counts[indx] > NSITES-1) counts[indx] = NSITES-1;
      if (counts[indx] < 0) counts[indx] = 0;
      newSrtPoint[counts[indx]] = srtPointer[i]; 
      
      counts[indx] -= 1;
    }
    TrinketKeyboard.poll();

    for (short indx =0; indx<NSITES; indx++) {
      srtPointer[indx] = newSrtPoint[indx];
    }
    TrinketKeyboard.poll();

  }

}

void loop() {
  // put your main code here, to run repeatedly:
  TrinketKeyboard.poll();
  
  pinMatch = false;
  while (!pinMatch) {

    lcd.clearDisplay();
    lcd.display();
    lcd.setCursor(0,0);
    lcd.println("    LOCKED");
    lcd.print("     ");
    lcd.display();
    TrinketKeyboard.poll();
    
    pinError = false;
    for (indx = 0; indx < sizeof(pinCode); indx++) {
      pinDigit = 0;
      
// check for button push      
      while (pinDigit == 0) {
        pinDigit = !digitalRead(BUTTON1)+(!digitalRead(BUTTON2)*2)+(!digitalRead(BUTTON3)*3)+(!digitalRead(BUTTON4)*4);
        TrinketKeyboard.poll();
      }
      
      pollDelay(50);
      contrastTimeStart = millis();
      
//    wait for button release
      while ((!digitalRead(BUTTON1)+(!digitalRead(BUTTON2)*2)+(!digitalRead(BUTTON3)*3)+(!digitalRead(BUTTON4)*4)) != 0) {
        
// If button 1 or 2 held on lock screen, adjust contrast        
        if ((millis() > contrastTimeStart+1000) && (pinDigit == 1 || pinDigit == 2)) {
          if (pinDigit == 1) {
            contrast = min(contrast+1,70);
          } else {
            contrast = max(contrast-1,45);
          }
          lcd.print(contrast);
          lcd.display();
          pollDelay(1000);
          lcd.setContrast(contrast);

// abort any PIN progress
          pinError = true;
          indx = sizeof(pinCode);
          break;
        }
        TrinketKeyboard.poll();
      }
      pollDelay(50);

      lcd.print("*");
      lcd.display();
      if (pinDigit != pinCode[indx]) pinError = true;
    }
    if (!pinError) {

      testS = (TrinketKeyboard.getLEDstate() & KB_LED_CAPS);
      if (testS != 0) {  // Check to see if Trinket thinks cap locks is on
        lcd.clearDisplay();
        lcd.println(" ");
        lcd.println("Cap Locks On");
        lcd.println(TrinketKeyboard.getLEDstate());
        lcd.println((int)TrinketKeyboard.isConnected());
        lcd.println("Cap Locks On");
        lcd.display();
        pollDelay(1000);
      } else {
        pinMatch=true;
      }
    }
    
    pollDelay(500);
  }
  
  lcd.clearDisplay();
  lcd.println("   UNLOCKED");
  lcd.display();
  pollDelay(750);

  timeOutEnd = millis() + timeOutDelay;
  lcd.clearDisplay();
  lcd.display();

// make sure we reset if & when millis overflow rollovers  
  while (timeOutEnd < millis()) {
    lcd.println("Time-Out Lock");
    lcd.print(millis());
    lcd.display();
    pollDelay(1000);
    timeOutEnd = millis() + timeOutDelay;
  }

  indx = 0;
  fillScreen(indx,6);
 
  bothSent = false;
  userNameSent = false;
  selected = false;
  while (millis() < timeOutEnd && !bothSent) {
      pinDigit = 0;
      while (pinDigit == 0 && millis() < timeOutEnd) {
        pinDigit = !digitalRead(BUTTON1)+(!digitalRead(BUTTON2)*2)+(!digitalRead(BUTTON3)*3)+(!digitalRead(BUTTON4)*4);
        TrinketKeyboard.poll();
      }
      pollDelay(50);
      sendTab = true;
      screenScrolled = false;
      scrollTimeStart = millis();
      while ((!digitalRead(BUTTON1)+(!digitalRead(BUTTON2)*2)+(!digitalRead(BUTTON3)*3)+(!digitalRead(BUTTON4)*4)) != 0 && millis() < timeOutEnd && sendTab) {
        if ((millis() > scrollTimeStart+1000) && (pinDigit == 1 || pinDigit == 2)) {
          screenScrolled = true;
          if (pinDigit == 1) {
            indx += 1;
            if (indx > sizeof(siteName)/NAMLEN-1) indx=0;
            fillScreen(indx,6);
            userNameSent = false;
            selected = false;
            pollDelay(25);
          }

          if (pinDigit == 2) {
            indx -= 1;
            if (indx < 0) indx = sizeof(siteName)/NAMLEN-1;
            fillScreen(indx,6);
            userNameSent = false;
            selected = false;
            pollDelay(25);
          }
        }

        if (millis() > scrollTimeStart+1000 && pinDigit == 3 && selected && !userNameSent) {
          sendTab = false;
        }
        TrinketKeyboard.poll();
      }
      pollDelay(50);

      if (screenScrolled) {
        newKeyNeeded = false;
        pinDigit = 0;
      }

      if (pinDigit != 0) newKeyNeeded = false;

      if (pinDigit == 1) {
        indx += 1;
        if (indx > sizeof(siteName)/NAMLEN-1) indx=0;
        fillScreen(indx,6);
        userNameSent = false;
        selected = false;
      }

      if (pinDigit == 2) {
        indx -= 1;
        if (indx < 0) indx = sizeof(siteName)/NAMLEN-1;
        fillScreen(indx,6);
        userNameSent = false;
        selected = false;
      }

      if (pinDigit == 3 && !selected && !newKeyNeeded) {
        fillScreen(indx,1);
        lcd.setCursor(0,8);
        lcd.print("Username...");
        lcd.display();
        selected = true;
        newKeyNeeded = true;
      }

      if (pinDigit == 3 && selected && !userNameSent && !newKeyNeeded) {
//  Send Username Here
//        TrinketKeyboard.print(userName[indx]);
        for (short k=0;k<UNMLEN-1;k++) {
          myChar = pgm_read_byte_near(userName[srtPointer[indx]] + k);
          if (myChar != '\0') {
            TrinketKeyboard.print(myChar);
            pollDelay(50);
          }
        }
//        if (sendTab) Keyboard.write(0xB3);
        if (sendTab) TrinketKeyboard.print("\t");
//        Keyboard.end();
        
        lcd.setCursor(0,18);
        lcd.print("Password...");
        lcd.display();
        userNameSent = true;
        newKeyNeeded = true;

        TrinketKeyboard.poll();
        if (!sendTab) {
          while ((!digitalRead(BUTTON1)+(!digitalRead(BUTTON2)*2)+(!digitalRead(BUTTON3)*3)+(!digitalRead(BUTTON4)*4)) != 0 && millis() < timeOutEnd) {
            TrinketKeyboard.poll();
          }
        }
      }
    
      if (pinDigit == 3 && selected && userNameSent && !newKeyNeeded) {
//  Send Password Here
//       Keyboard.print(passWord[indx]);
        for (short k=0;k<PASLEN-1;k++) {
          myChar = pgm_read_byte_near(passWord[srtPointer[indx]] + k);
          if (myChar != '\0') {
            TrinketKeyboard.print(myChar);
            pollDelay((k % 3) * 25);  // make the password entry look a little more human (slower/random), required by Starbucks.com
          }
        }
//        Keyboard.end();
        
        bothSent = true;
        newKeyNeeded = true;
      }

      if (pinDigit == 4) bothSent = true;
      TrinketKeyboard.poll();
  }
    
}

