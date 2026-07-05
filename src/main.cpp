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

enum GameState
  {
    STANDBY,
    SHOW_SEQUENCE,
    WAIT_FOR_PLAYER,
    NEXT_LEVEL,
    GAME_OVER
  
  };

GameState gameState = STANDBY;

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
    switch (gameState)
    {
        //========================================
        // WAITING FOR PLAYER TO PRESS START
        //========================================
        case STANDBY:

            if (digitalRead(START_BUTTON) == LOW)
            {
                delay(200);

                currentLevel = 1U;
                playerIndex = 0U;

                generateNextMove();

                gameState = SHOW_SEQUENCE;
            }

        break;

        //========================================
        // SHOW THE SIMON SEQUENCE
        //========================================
        case SHOW_SEQUENCE:

            playSequence();

            playerIndex = 0U;

            gameState = WAIT_FOR_PLAYER;

        break;

        //========================================
        // WAIT FOR PLAYER INPUT
        //========================================
        case WAIT_FOR_PLAYER:

            if (getPlayerInput())
            {
                playerIndex++;

                if (playerIndex >= currentLevel)
                {
                    gameState = NEXT_LEVEL;
                }
            }

        break;

        //========================================
        // PREPARE NEXT LEVEL
        //========================================
        case NEXT_LEVEL:

            currentLevel++;

            if(currentLevel > MAX_SEQUENCE)
              {
                gameState = GAME_OVER;
                break;
              }

            generateNextMove();

            gameState = SHOW_SEQUENCE;

        break;

        //========================================
        // PLAYER LOST
        //========================================
        case GAME_OVER:

            gameOver();

            showStandbyScreen();

            gameState = STANDBY;

        break;
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

    if (!key)
    {
        return false;
    }

    uint8_t playerMove;

    switch(key)
    {
        case '1':
            playerMove = 0U;
        break;

        case '2':
            playerMove = 1U;
        break;

        case '3':
            playerMove = 2U;
        break;

        case 'A':
            playerMove = 3U;
        break;

        default:
            return false;
    }

    flashLed(playerMove,300);

    playTone(playerMove,300);

    if(playerMove == gameSequence[playerIndex])
    {
        return true;
    }

    gameState = GAME_OVER;

    return false;

     

  GameState gameState = STANDBY;

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
   for(uint8_t led=0U; led < TOTAL_LEDS; led++)
    {
      flashLed(led,200);
    };

  
 
}
  
  