// Accelerometer
#include <Wire.h>
#include <Adafruit_MMA8451.h>  
#include <Adafruit_Sensor.h>
// Display
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>

// Hardware defines
// TFT
#define TFT_CS                         10
#define TFT_RST                        0  // Set to 0 to use Arduino reset pin
#define TFT_DC                         8  // Data/command
#define BACKLIGHT                      3  // PWM backlight
// Button
#define BUTTON                         2

// Algorithm defines
#define FADE_STEP_MS_FAST              5 // Time to wait every time we adjust the brightness on the display
#define FADE_STEP_MS_SLOW              30
#define FADE_STEP_PWM                  5  // Amount to change the duty cycle each step
#define PWM_MAX_VALUE                  255

#define TEXT_SIZE_SMALL                2
#define TEXT_SIZE_MED                  5
#define TEXT_SIZE_LARGE                8

#define SCREEN_ON_DURATION_MS_LONG     1500
#define SCREEN_ON_DURATION_MS_SHORT    750

/** The tpes of dice supported.  `die_type_t` is more technically correct, but English is a living language */
enum dice_type_t
{
   DICE_MODE_D4,
   DICE_MODE_D6,
   DICE_MODE_D8,
   DICE_MODE_D10,
   DICE_MODE_D12,
   DICE_MODE_D20,
   DICE_MODE_MAX,
};

/** The number of sides (and max value) on each die.  Must be in the same order as dice_type_t */
const uint8_t num_sides[] = 
{
   4,
   6,
   8,
   10,
   12,
   20,
};

/** The string corresponding to each die.  Must be in the same order as dice_type_t */
const char* dice_name[] = 
{
   "D4",
   "D6",
   "D8",
   "D10",
   "D12",
   "D20",
};

// Create objects for the display and accelerometer
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MMA8451 accel = Adafruit_MMA8451();

// Default to a 20 sided die.  Don't use the typedef because C++ won't let me increment it, because it's a worse language than C
uint8_t dice_mode = DICE_MODE_D20;


/** @brief Arduino setup
*/
void setup(void) 
{
   // Configure GPIOs
   pinMode(BUTTON, INPUT_PULLUP);
   pinMode(BACKLIGHT, OUTPUT);
   analogWrite(BACKLIGHT, 0);
   
   // Start the accelerometer
   // Use the default sensitivity values.  That may change when this moves off the breadboard
   accel.begin();
   
   // Initialize the display
   tft.initR(INITR_144GREENTAB); 
   tft.fillScreen(ST7735_BLACK);
   
   // Generate a random seed value
   // Reading from an unconnected IO is not as random as one would expect without an antenna 
   // Instead, make the user press the button and randomize based on how long it took them to do that

   // Display some instructions first
   tft.setTextSize(TEXT_SIZE_SMALL);
   tft.setTextWrap(false);
   tft.setCursor(0, 0);
   tft.print("Push \r\nbutton \r\nto charge \r\ndice");
   fade_display(true, FADE_STEP_MS_FAST); // Fade in the display
   
   // Count up as fast as we can
   long int randomSeedValue = 0;
   while(digitalRead(BUTTON))
   {
      randomSeedValue++;
   }
   randomSeed(randomSeedValue);
   
   // Display some more text
   tft.fillScreen(ST7735_BLACK);
   draw_text_centered("Good Luck");
   delay(SCREEN_ON_DURATION_MS_SHORT);

   
   // Wait for buton to be released
   while(!digitalRead(BUTTON)){}
   
   // Face out the display, clear the text
   fade_display(false, FADE_STEP_MS_SLOW);
   tft.fillScreen(ST7735_BLACK);
}

/** @brief Arduino main loop
*/
void loop()
{
   uint8_t int_source = accel.getIntSource();
   
   // Button Pressed
   // Changes the type of die
   if (!digitalRead(BUTTON))
   {
      // Turn on screen
      analogWrite(BACKLIGHT, PWM_MAX_VALUE);
      
      // Increment die type and wrap around
      dice_mode++;
      if (dice_mode >= DICE_MODE_MAX)
      {
         dice_mode = 0;
      }
      
      // Display the new die
      tft.setTextColor(ST7735_WHITE);
      tft.setTextSize(TEXT_SIZE_MED);
      draw_text_centered(dice_name[dice_mode]);
      
      // Display until they release the button, then fade out
      while(!digitalRead(BUTTON)){};
      fade_display(false, FADE_STEP_MS_FAST);
      tft.fillScreen(ST7735_BLACK);
   }
   
   // Accelerometer tap
   // "Rolls" the die
   else if (int_source & ACCEL_INT_SOURCE_TAP)
   {
      // Get the random value
      char buffer[8] = {0};
      uint8_t roll = random(1, num_sides[dice_mode] + 1);
      sprintf(buffer, "%i", roll);

      // Display it
      tft.setTextSize(TEXT_SIZE_LARGE);
      // Use fancy colors if it's the max or min value
      if (roll == 1)
      {
         tft.setTextColor(ST7735_RED);
         draw_text_centered(buffer);
      }
      // Blue is good
      else if (roll == num_sides[dice_mode])
      {
         tft.setTextColor(ST7735_BLUE);
         draw_text_centered(buffer);
      }
      // Red is bad
      else
      {
         tft.setTextColor(ST7735_WHITE);
         draw_text_centered(buffer);
      }
      
      // Fade in
      fade_display(true, FADE_STEP_MS_SLOW);
      delay(SCREEN_ON_DURATION_MS_LONG);
      // Fade out 
      fade_display(false, FADE_STEP_MS_SLOW);
      tft.fillScreen(ST7735_BLACK);
      
      // Debounce the accelerometer
      // If it got a tap during the time we were displaying the last value, we don't want to go straight into the next roll
      int_source = accel.getIntSource();
   }
}

/** @brief Helper function to draw text centered on the screen.  
  * Note that it doesn't really work on multi-line text
  * @param text the null-terminated string to print
  */
void draw_text_centered(char *text) 
{
   uint16_t x, y, width, height;
   
   // Get the size of the smallest rectangle that holds the text
   // We really only care about the width and height
   tft.getTextBounds(text, 0, 0, &x, &y, &width, &height);
   
   // Use the width and height and the size of the screen to get the x and y that we want
   y = (ST7735_TFTHEIGHT_128 - height)/2;
   x = (ST7735_TFTWIDTH_128 - width)/2;
   
   // Set the cursor and print it
   tft.setCursor(x, y);
   tft.print(text);
}

/**
  * @brief Helper function to fade the screen in or out (blocking)
  * @param fade_in true if the screen should fade from off to on, false otherwise
  * @param fade_step_ms time in ms to wait between PWM steps.  If I were to do this again, I'd make this parameter the total fade time and calculate this internally
  */
void fade_display(bool fade_in, uint8_t fade_step_ms)
{
   // Use a signed int16 so we don't have to worry about whether PWM_MAX_VALUE is evenly divisible by FADE_STEP_PWM
   int16_t brightness;
   
   if(fade_in)
   {
      brightness = 0;
      while (brightness < PWM_MAX_VALUE)
      {
         // Increase and bounds check brightness
         brightness += FADE_STEP_PWM;
         if (brightness > PWM_MAX_VALUE)
         {
            brightness = PWM_MAX_VALUE;
         }
         analogWrite(BACKLIGHT, brightness);
         // Wait so the dimming effect is visible
         delay(fade_step_ms);
      }
   }
   else
   {
      brightness = PWM_MAX_VALUE;
      while (brightness > 0)
      {
         // Decrease and bounds check brightness
         brightness -= FADE_STEP_PWM;
         if (brightness < 0)
         {
            brightness = 0;
         }
         analogWrite(BACKLIGHT, brightness);
         // Wait so the dimming effect is visible
         delay(fade_step_ms);
      }
   }
}
