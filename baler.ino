#include <EEPROM.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int lcd_key     = 0;
int adc_key_in  = 0;
int baleDiameterPotentiometer = 0;
unsigned int refreshLCDTime;
volatile boolean bunkerStatus;

#define ACTUATOR_POSITION_CENTER_PIN 1
#define ACTUATOR_POSITION_95_PIN 2
#define ACTUATOR_POSITION_CUT_PIN 3
#define BUNKER_STATUS_PIN 12
#define BEEPER_PIN 13
#define REVERSE_PIN 3

#define PWM 3

// 3 O
// 4 I
// 1 A

#define BEEPER_ON digitalWrite(BEEPER_PIN, HIGH)
#define BEEPER_OFF digitalWrite(BEEPER_PIN, LOW)
#define BALER_REVERS_ON digitalWrite(REVERSE_PIN, HIGH)
#define BALER_REVERS_OFF digitalWrite(REVERSE_PIN, LOW)

#define BUNKER_CLOSED 0

#define STATUS_OF_BUNKER_INT 1
#define btnRIGHT  0
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define BUTTON_DELAY 250
#define EDIT_BALE_DIAMETER_AT_THE_OUTPUT 1
#define EDIT_KNITTING_SPEED_OF_THE_BALES 2
#define EDIT_TIME_KNITTING_THE_ENDS 3
#define EDIT_AMOUNT_OF_THE_BALES 4
#define EDIT_DELAY_BEFORE_KNITTING 5
#define EDIT_OFF 6
unsigned int selectedMode;

struct Baler {
  unsigned int baleDiameterAtTheOutput = 800;
  unsigned int knittingSpeedOfBale = 1;
  unsigned int timeKnittingTheEnds = 7;
  unsigned int amountOfTheBales = 0;
  unsigned int delayBeforeKnitting = 4;
};

Baler baler;

void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.blink();
  lcd.cursor();
  EEPROM.get(0, baler);
  if (baler.baleDiameterAtTheOutput == 65535) {
    Baler defaultBaler;
    
    baler = defaultBaler;
  }
  pinMode(BUNKER_STATUS_PIN, INPUT);
  pinMode(BEEPER_PIN, OUTPUT);
  //pinMode(PWM, OUTPUT);
}

int read_LCD_buttons()
{
  adc_key_in = analogRead(0);
  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 195)  return btnUP;
  if (adc_key_in < 380)  return btnDOWN;
  if (adc_key_in < 555)  return btnLEFT;
  if (adc_key_in < 790)  return btnSELECT;
  return btnNONE;
}

void draw() {
  //lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("   ");
  lcd.setCursor(0, 0);
  lcd.print("= ");
  baleDiameterPotentiometer = analogRead(1);
  lcd.print(baleDiameterPotentiometer);
  lcd.setCursor(0, 1);
  lcd.print("d ");
  lcd.print(baler.baleDiameterAtTheOutput);
  lcd.setCursor(7, 1);
  lcd.print("n ");
  lcd.print(baler.amountOfTheBales);
  lcd.setCursor(7, 0);
  lcd.print("t ");
  lcd.print(baler.knittingSpeedOfBale);
  lcd.setCursor(14, 0);
  lcd.print("/");
  lcd.print(baler.timeKnittingTheEnds);
  lcd.setCursor(14, 1);
  lcd.print("/");
  lcd.print(baler.delayBeforeKnitting);
}

int actuatorStatus;

unsigned long beepTime;

void beep() {
  /*if (millis() >= beepTime + 300) {
    BEEPER_OFF;
  }
  if (millis() >= beepTime) {
    BEEPER_ON;
    beepTime = millis() + 1500;
  }*/
  BEEPER_ON;
  delay(3000);
  BEEPER_OFF;
}

void knitting() {
  BALER_REVERS_ON;
  analogWrite(PWM, 245);
  while (digitalRead(ACTUATOR_POSITION_CENTER_PIN)) {
    beep();
  }
  analogWrite(PWM, 0);
  BALER_REVERS_OFF;
  delay(baler.timeKnittingTheEnds * 1000);
  analogWrite(PWM, baler.knittingSpeedOfBale);
  while (digitalRead(ACTUATOR_POSITION_95_PIN)) {
    beep();
  }
  analogWrite(PWM, 0);
  delay(baler.timeKnittingTheEnds * 1000);
  analogWrite(PWM, 245);
  while (digitalRead(ACTUATOR_POSITION_CUT_PIN)) {
    beep();
  }
  analogWrite(PWM, 0);
}

/*
 * Включаємо реверс
 * включаємо шим на 255
 *
 * чекаємо поки з пін 1 з логічної 1 став 0
 *  Виключаємо пвм та реверс
 *
 * затримка на delay(baler.timeKnittingTheEnds * 1000);
 * analogWrite(PWM, baler.knittingSpeedOfBale);
 *
 * чекаємо поки з пін 2 з логічної 1 став 0
 *
 * analogWrite(PWM, 0);
 * delay(baler.timeKnittingTheEnds * 1000);
 *
 *
 *
 */
boolean saved;
boolean beepStatus;
void loop() {
  lcd_key = read_LCD_buttons();
  switch (lcd_key)
  {
    case btnRIGHT:
      {
        break;
      }
    case btnLEFT:
      {
        break;
      }
    case btnUP:
      {
        break;
      }
    case btnSELECT:
      {
        selectedMode++;
        delay(BUTTON_DELAY);
        break;
      }
  }
  mode();
  if (millis() >= refreshLCDTime) {
    draw();
    refreshLCDTime = millis() + 797;
  }
  if (baleDiameterPotentiometer >= baler.baleDiameterAtTheOutput) {
    beep();
    while (digitalRead(BUNKER_STATUS_PIN) == BUNKER_CLOSED) {
      //BEEPER_ON;
      //delay(baler.delayBeforeKnitting * 1000);
      //BEEPER_OFF;
      //knitting();
      saved = true;
      if (millis() >= refreshLCDTime) {
        draw();
        refreshLCDTime = millis() + 797;
      }
    }
    if (saved) {
      baler.amountOfTheBales++;
      EEPROM.put(0, baler);
      saved = false;
      //beep();
    }

  }



}

void mode() {
  switch (selectedMode)
  {
    case EDIT_BALE_DIAMETER_AT_THE_OUTPUT:
      {
        lcd.blink();
        lcd.cursor();
        lcd.setCursor(2, 1);
        switch (lcd_key)
        {
          case btnUP:
            {
              baler.baleDiameterAtTheOutput++;
              if (baler.baleDiameterAtTheOutput > 1024) {
                baler.baleDiameterAtTheOutput--;
              }
              delay(BUTTON_DELAY);
              break;
            }
          case btnDOWN:
            {
              baler.baleDiameterAtTheOutput--;
              if (baler.baleDiameterAtTheOutput < 90) {
                baler.baleDiameterAtTheOutput++;
              }
              delay(BUTTON_DELAY);
              break;
            }
        }
        break;
      }
    case EDIT_KNITTING_SPEED_OF_THE_BALES:
      {
        lcd.setCursor(9, 0);
        switch (lcd_key)
        {
          case btnUP:
            {
              baler.knittingSpeedOfBale++;
              delay(BUTTON_DELAY);
              break;
            }
          case btnDOWN:
            {
              baler.knittingSpeedOfBale--;
              if (baler.knittingSpeedOfBale == 65535 || baler.knittingSpeedOfBale == 0) {
                baler.knittingSpeedOfBale = 1;
              }
              delay(BUTTON_DELAY);
              break;
            }
        }
        break;
      }
    case EDIT_AMOUNT_OF_THE_BALES:
      {
        lcd.setCursor(9, 1);
        switch (lcd_key)
        {
          case btnUP:
            {
              baler.amountOfTheBales++;
              delay(BUTTON_DELAY);
              break;
            }
          case btnDOWN:
            {
              baler.amountOfTheBales--;
              if (baler.amountOfTheBales == 65535) {
                baler.amountOfTheBales = 0;
              }
              delay(BUTTON_DELAY);
              break;
            }
        }
        break;
      }
    case EDIT_TIME_KNITTING_THE_ENDS:
      {
        lcd.setCursor(15, 0);
        switch (lcd_key)
        {
          case btnUP:
            {
              baler.timeKnittingTheEnds++;
              if (baler.timeKnittingTheEnds > 9) {
                baler.timeKnittingTheEnds--;
              }
              delay(BUTTON_DELAY);
              break;
            }
          case btnDOWN:
            {
              baler.timeKnittingTheEnds--;
              if (baler.timeKnittingTheEnds < 2) {
                baler.timeKnittingTheEnds++;
              }
              delay(BUTTON_DELAY);
              break;
            }
        }
        break;
      }
    case EDIT_DELAY_BEFORE_KNITTING:
      {
        lcd.setCursor(15, 1);
        switch (lcd_key)
        {
          case btnUP:
            {
              baler.delayBeforeKnitting++;
              if (baler.delayBeforeKnitting > 9) {
                baler.delayBeforeKnitting--;
              }
              delay(BUTTON_DELAY);
              break;
            }
          case btnDOWN:
            {
              baler.delayBeforeKnitting--;
              if (baler.delayBeforeKnitting < 2) {
                baler.delayBeforeKnitting++;
              }
              delay(BUTTON_DELAY);
              break;
            }
        }
        break;
      }
    case EDIT_OFF:
      {
        selectedMode = 0;
        lcd.noBlink();
        lcd.noCursor();
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("SAVED");
        lcd.setCursor(0, 1);
        lcd.print("+380971848484");
        delay(1797);
        lcd.clear();
        EEPROM.put(0, baler);
        break;
      }
  }
}
