#include "Keyboard.h"
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
#define PASLEN 18

const short BUTTON1=8,BUTTON2=9,BUTTON3=10,BUTTON4=7;

const short pinCode[]={2,3,4,4,1};

const static char siteName[][NAMLEN] PROGMEM={
  "Starbucks",
  "Sleepnumber",
  "Adafruit",
  "Mouser",
  "Particle",
  "World of Tanks",
  "BangGood",
  "Amazon",
  "Captl One Web",
  "Captl One03/16",
  "Pax Enforcer",
  "Cut Splice",
  "Netflix",
  "Paitent Gatewa",
  "Twitter",
  "LinkedIn",
  "Flixster",
  "FitBit Dashboa",
  "Verizon Router",
  "LinkSys Router",
  "MyVerizon Web",
  "Paypal",
  "BU Alumni",
  "Mass Pike",
  "LOTRO",
  "NewEgg",
  "Steam",
  "Barnes & Noble",
  "Social Securit",
  "Twitch",
  "Filters-Now",
  "IFTTT",
  "SWGEMU",
  "goodreads",
  "Troop 17",
  "US Postal Serv",
  "Facebook"
};

const static char* userName[]={
  "Starbucks-UserN",
  "Sleepnumber-UserN",
  "Adafruit-UserN",
  "Mouser-UserN",
  "Particle-UserN",
  "World of Tanks-UserN",
  "BangGood-UserN",
  "Amazon-UserN",
  "Captl One Web-UserN",
  "Captl One-CardNum",
  "Pax Enforcer-UserN",
  "Cut Splice-UserN",
  "Netflix-UserN",
  "Paitent Gatewa-UserN",
  "Twitter-UserN",
  "LinkedIn-UserN",
  "Flixster-UserN",
  "FitBit Dashboa-UserN",
  "Verizon Router-UserN",
  "LinkSys Router-UserN",
  "MyVerizon Web-UserN",
  "Paypal-UserN",
  "BU Alumni-UserN",
  "Mass Pike-UserN",
  "LOTRO-UserN",
  "NewEgg-UserN",
  "Steam-UserN",
  "Barnes & Noble-UserN",
  "Social Securit-UserN",
  "Twitch-UserN",
  "Filters-Now-UserN",
  "IFTTT-UserN",
  "SWGEMU-UserN",
  "goodreads-UserN",
  "Troop 17-UserN",
  "US Postal Serv-UserN",
  "Facebook"
};

const static char passWord[][PASLEN] PROGMEM={
  "StarbucksPW",
  "SleepnumberPW",
  "AdafruitPW",
  "MouserPW",
  "ParticlePW",
  "World of TanksPW",
  "BangGoodPW",
  "AmazonPW",
  "CaptlOneWebPW",
  "CaptlOneSecCd",
  "PaxEnforcerPW",
  "CutSplicePW",
  "NetflixPW",
  "PaitentGatewaPW",
  "TwitterPW",
  "LinkedInPW",
  "FlixsterPW",
  "FitBitDashboaPW",
  "VerizonRouterPW",
  "LinkSysRouterPW",
  "MyVerizonWebPW",
  "PaypalPW",
  "BUAlumniPW",
  "MassPikePW",
  "LOTROPW",
  "NewEggPW",
  "SteamPW",
  "Barnes&NoblePW",
  "SocialSecuritPW",
  "TwitchPW",
  "Filters-NowPW",
  "IFTTTPW",
  "SWGEMUPW",
  "goodreadsPW",
  "Troop17PW",
  "USPostalServPW",
  "FacebookPW"
};

short pinDigit;
bool pinMatch,pinError,userNameSent,selected,bothSent,newKeyNeeded,screenScrolled,sendTab;
int indx;
unsigned long scrollTimeStart,timeOutEnd,timeOutDelay=300000;
char myChar;

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
      myChar = pgm_read_byte_near(siteName[iOffset] + k);
      if (myChar != '\0') lcd.print(myChar);
    }
    lcd.println(" ");
  }
  lcd.display();
  
  lcd.setTextColor(BLACK);
  return;
}
  

void setup() {
  // put your setup code here, to run once:
//  Serial.begin(9600);
  
  lcd.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  lcd.setContrast(60);

  pinMode(BUTTON1,INPUT_PULLUP);
  pinMode(BUTTON2,INPUT_PULLUP);
  pinMode(BUTTON3,INPUT_PULLUP);
  pinMode(BUTTON4,INPUT_PULLUP);
 
  lcd.setTextSize(1);
  lcd.setTextColor(BLACK);
  lcd.setTextWrap(false);

}

void loop() {
  // put your main code here, to run repeatedly:

  pinMatch = false;
  while (!pinMatch) {

    lcd.clearDisplay();
    lcd.display();
    lcd.setCursor(0,0);
    lcd.println("    LOCKED");
    lcd.print("     ");
    lcd.display();
    
    pinError = false;
    for (indx = 0; indx < sizeof(pinCode)/2; indx++) {
      pinDigit = 0;
      while (pinDigit == 0) {
        pinDigit = !digitalRead(BUTTON1)+(!digitalRead(BUTTON2)*2)+(!digitalRead(BUTTON3)*3)+(!digitalRead(BUTTON4)*4);
      }
      delay(50);
      while ((!digitalRead(BUTTON1)+(!digitalRead(BUTTON2)*2)+(!digitalRead(BUTTON3)*3)+(!digitalRead(BUTTON4)*4)) != 0);
      delay(50);

      lcd.print("*");
      lcd.display();
      if (pinDigit != pinCode[indx]) pinError = true;
    }
    if (!pinError) pinMatch=true;
    delay(500);
  }
  
  lcd.clearDisplay();
  lcd.print("   UNLOCKED");
  lcd.display();
  delay(1000);

  timeOutEnd = millis() + timeOutDelay;
  lcd.clearDisplay();
  lcd.display();

// make sure we reset if & when millis overflow rollovers  
  while (timeOutEnd < millis()) {
    lcd.println("Time-Out Lock");
    lcd.print(millis());
    lcd.display();
    delay(1000);
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
      }
      delay(50);
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
            delay(150);
          }

          if (pinDigit == 2) {
            indx -= 1;
            if (indx < 0) indx = sizeof(siteName)/NAMLEN-1;
            fillScreen(indx,6);
            userNameSent = false;
            selected = false;
            delay(150);
          }
        }

        if (millis() > scrollTimeStart+1000 && pinDigit == 3 && selected && !userNameSent) {
          sendTab = false;
        }
      }
      delay(50);

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
        Keyboard.begin();
        Keyboard.print(userName[indx]);
        if (sendTab) Keyboard.write(0xB3);
        Keyboard.end();
        
        lcd.setCursor(0,18);
        lcd.print("Password...");
        lcd.display();
        userNameSent = true;
        newKeyNeeded = true;

        if (!sendTab) {
          while ((!digitalRead(BUTTON1)+(!digitalRead(BUTTON2)*2)+(!digitalRead(BUTTON3)*3)+(!digitalRead(BUTTON4)*4)) != 0 && millis() < timeOutEnd) ;
        }
      }
    
      if (pinDigit == 3 && selected && userNameSent && !newKeyNeeded) {
//  Send Password Here
        Keyboard.begin();
 //       Keyboard.print(passWord[indx]);
        for (short k=0;k<PASLEN-1;k++) {
          myChar = pgm_read_byte_near(passWord[indx] + k);
          if (myChar != '\0') Keyboard.print(myChar);
          delay((k % 3) * 25);  // make the password entry look a little more human (slower/random), required by Starbucks.com
        }
        Keyboard.end();
        
//        lcd.setCursor(0,1);
//        lcd.print("            ");
//        lcd.display();
        bothSent = true;
        newKeyNeeded = true;
      }

      if (pinDigit == 4) bothSent = true;

  }
  
/*  
  while (digitalRead(BUTTON1));
  delay(100);
  while (!digitalRead(BUTTON1));  

  Keyboard.begin();
  Keyboard.print("davidhunter@rocketmail.com");
  Keyboard.end();

  while (digitalRead(BUTTON1));
  delay(100);
  while (!digitalRead(BUTTON1));  
  
  Keyboard.begin();
  Keyboard.print("xxxxxxxx");
  Keyboard.end();
*/

  
}

