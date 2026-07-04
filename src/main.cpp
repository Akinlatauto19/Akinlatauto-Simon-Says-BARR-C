#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27,16,2);

/*--------------------------------------------------
 * THE Game Keypad DEFINITIONS
 *-------------------------------------------------*/

const byte ROWS = 4U;

const byte COLS = 4U;

char keys[ROWS][COLS] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

//ROW PIN DEFINITIONS
byte rowPins[ROWS] =
{
    9,
    8,
    7,
    6
};

//COLUMN PIN DEFINITIONS
byte colPins[COLS] =
{
    5,
    4,
    3,
    2
};

Keypad keypad =
    Keypad
    (
        makeKeymap(keys),
        rowPins,
        colPins,
        ROWS,
        COLS
   );


/*--------------------------------------------------
 * THE GAME PIN DEFINITIONS AND CONSTANTS
 *-------------------------------------------------*/

constexpr uint8_t TOTAL_LEDS = 4U;

constexpr uint8_t MAX_SEQUENCE = 50U;

constexpr uint16_t START_SPEED = 600U;

constexpr uint16_t LEVEL_DELAY = 250U;

constexpr uint8_t LED_RED = 10U;

constexpr uint8_t LED_GREEN = 11U;

constexpr uint8_t LED_BLUE = 12U;

constexpr uint8_t LED_YELLOW = 13U;

constexpr uint8_t BUZZER_PIN = A0;

constexpr uint8_t START_BUTTON = A2;

uint8_t gameSequence[MAX_SEQUENCE];

uint8_t currentLevel = 0U;

uint8_t playerIndex = 0U;

bool gameStarted = false;
/*--------------------------------------------------
 * THE GAME FUNCTIONS
 *-------------------------------------------------*/

void initializeHardware();

void showStandbyScreen();

void generateNextMove();

void playSequence();

bool getPlayerInput();

void gameOver();

void flashLed(uint8_t ledIndex, uint16_t flashTime);

void playTone(uint8_t ledIndex, uint16_t toneDuration);

void displayLevel();

/*--------------------------------------------------
 * THE GAME SETUP FUNCTION
 *-------------------------------------------------*/

void setup() 
{
  
   initializeHardware();
                 
   showStandbyScreen();

}

void initializeHardware() 
{
 // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  //initialize the leds
  const uint8_t ledPins[TOTAL_LEDS] = {LED_RED, LED_GREEN, LED_BLUE, LED_YELLOW};

     for (uint8_t led = 0U; led < TOTAL_LEDS; led++)
       {
         pinMode(ledPins[led], OUTPUT);
         digitalWrite(ledPins[led], LOW);
       }

  //initialize the buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  //initialize the start button
  pinMode(START_BUTTON, INPUT_PULLUP);
}

void clearDisplay() 
{
  
  lcd.clear();
  
}

void showStandbyScreen() 
{
    clearDisplay();

      lcd.setCursor(0,0);
      lcd.print("SIMON SAYS");

      lcd.setCursor(0,1);
      lcd.print("Press START");
    
}
/*--------------------------------------------------
 * THE GAME loop FUNCTION
 *-------------------------------------------------*/
 
void loop() 
{
  if (!gameStarted) 
  {
    if (digitalRead(START_BUTTON) == LOW) 
    {
      delay(200); // Debounce delay

      gameStarted = true;
      currentLevel = 1U;
      playerIndex = 0U;
      generateNextMove();
      playSequence();

    }
  } 
  else 
  {
    if (getPlayerInput()) 
    {
      playerIndex++;
      
      if (playerIndex >= currentLevel) 
      {
         currentLevel++;
         playerIndex = 0U;
        
         generateNextMove();
        
         playSequence();
      }
    } 
     
  else 
    {
       gameOver();

       Serial.println("GAME OVER");

       flashLed(0, 1000);

       playTone(0, 1000);

       delay(1000);

       showStandbyScreen();
       
      }
  }

}
/*--------------------------------------------------------------------
 * THE GAME generate sequence for next level function IMPLEMENTATION
 *--------------------------------------------------------------------*/

void generateNextMove()
{
    if (currentLevel > MAX_SEQUENCE)
    {
        return;
    }

    gameSequence[currentLevel - 1U] = random(0U, TOTAL_LEDS);
}

void playSequence()
{
    displayLevel();

      for (uint8_t index = 0U; index < currentLevel; index++)
    {
        const uint8_t currentMove = gameSequence[index];

        flashLed(currentMove, START_SPEED);

        playTone(currentMove, START_SPEED);

        delay(LEVEL_DELAY);
    }
}

/*--------------------------------------------------
 * THE GAME level display function
 *-------------------------------------------------*/
void displayLevel() 
{
    clearDisplay();

      lcd.setCursor(0,0);
      lcd.print("Level: ");
      lcd.print(currentLevel);
}

/*--------------------------------------------------
 * THE GAME flash
 *-------------------------------------------------*/
void flashLed(uint8_t ledIndex, uint16_t flashTime)

{
    const uint8_t ledPins[TOTAL_LEDS] = {LED_RED, LED_GREEN, LED_BLUE, LED_YELLOW};

      digitalWrite(ledPins[ledIndex], HIGH);
      delay(flashTime);
      digitalWrite(ledPins[ledIndex], LOW);

}

/*--------------------------------------------------
 * THE GAME play sound function
 *-------------------------------------------------*/
void playTone(uint8_t ledIndex, uint16_t toneDuration)
{
    const uint16_t tones[TOTAL_LEDS] = {440U, 494U, 523U, 587U};

      tone(BUZZER_PIN, tones[ledIndex], toneDuration);
      delay(toneDuration);
      noTone(BUZZER_PIN);
}

/*-------------------------------------------------------------------------------------------
 * THE players function to get input from the keypad and check if it matches the expected move
 *--------------------------------------------------------------------------------------------*/
bool getPlayerInput() 
{
    char key = keypad.getKey();

      if (key) 
      {
        uint8_t expectedMove = gameSequence[playerIndex];

          switch (key) 
          {
            case '1':
              flashLed(0, 300);
              playTone(0, 300);
              return expectedMove == 0U;

            case '2':
              flashLed(1, 300);
              playTone(1, 300);
              return expectedMove == 1U;

            case '3':
              flashLed(2, 300);
              playTone(2, 300);
              return expectedMove == 2U;

            case '4':
              flashLed(3, 300);
              playTone(3, 300);
              return expectedMove == 3U;

            default:
              return false;
          }
      }

      return true; // No input yet
}

// game over function implementation
 void gameOver()
 {
  clearDisplay();

    lcd.setCursor(0,0);
    lcd.print("GAME OVER");

    lcd.setCursor(0,1);
    lcd.print("Level: ");
    lcd.print(currentLevel - 1U);

   tone(BUZZER_PIN, 200U, 1000U);
   for(uint8_t led=0U; led<TOTAL_LEDS; led++)
    {
      flashLed(led,200);
    };

    gameStarted = false;
 }
  
  
