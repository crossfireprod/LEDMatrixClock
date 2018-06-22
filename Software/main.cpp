// CLOCK!
// Zach Cross - Jul 2017

// INCLUDES
#include "application.h"
#include "Adafruit_mfGFX.h"    // Core graphics library
#include "RGBmatrixPanel.h"    // Hardware-specific library
#include "math.h"

// DEFINES
    // RGB Matrix GPIO pins
    #define CLK D6
    #define OE  D7
    #define LAT TX
    #define A   A0
    #define B   A1
    #define C   A2

    // General Configuration Parameters
    #define DISPLAY_REFRESH_RATE    1000
    #define TIME_SYNC_INTERVAL      86400000    // 24 Hrs
    #define TIME_ZONE               -4          // EDT (-5 for EST)

// PROTOTYPES / DECLARATIONS
    // Function Prototypes
    void demo_animation(void);
    void flash_status_pixel(int, int, char);
    void refresh_display(void);
    void sync_time(void);

    // RGB LED Matrix
    RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

    // Software Timers
    Timer timerRefreshDisplay(DISPLAY_REFRESH_RATE, refresh_display);
    Timer timerSyncTime(TIME_SYNC_INTERVAL, sync_time);

    // Global Time Variables
    int intHour;
    int intMinute;
    int intSecond;
    int intDay;
    int intMonth;


// GLOBAL VARIABLES
    // Display Demo
    const char str[] = "RFSR STEM";
    int textX   = matrix.width(),
            textMin = sizeof(str) * -12,
            hue     = 0;
    int8_t ball[3][4] = {
        {  3,  0,  1,  1 }, // Initial X,Y pos & velocity for 3 bouncy balls
        { 17, 15,  1, -1 },
        { 27,  4, -1,  1 }
    };
    static const uint16_t ballcolor[3] = {
      0x0080, // Green=1
      0x0002, // Blue=1
      0x1000  // Red=1
    };


void setup() {
    // ISR Time Measurement
    pinMode(A5, OUTPUT);

    // Register Cloud Variables
    Particle.variable("intHour", intHour);
    Particle.variable("intMinute", intMinute);
    Particle.variable("intSecond", intSecond);
    Particle.variable("intDay", intDay);
    Particle.variable("intMonth", intMonth);

    // Setup Matrix
    matrix.begin();
    matrix.setTextWrap(false); // Allow text to run off right edge

    // Wait for Particle cloud connection.
    while(Particle.connected() == false) {
       flash_status_pixel(1, 100, 'R');
    }

    // Wait for time sync.
    Particle.syncTime();
    while (Particle.syncTimeDone() != true || Particle.syncTimePending() != false || Time.isValid() != true) {
       flash_status_pixel(1, 100, 'B');
    }

    // Indicate successful cloud connection & time sync.
    flash_status_pixel(15, 100, 'G');

    // Configure Clock
    Time.zone(TIME_ZONE);

    // Start Display Update Timer
    timerRefreshDisplay.start();
    timerSyncTime.start();

}


void loop() {

    //
    //demo_animation();

    //
    Particle.process();

}


void sync_time() {
    Particle.syncTime();

} // END sync_time


void refresh_display(void) {
    // Toggle GPIO pin to time ISR speed w/ externally connected oscilloscope.
    digitalWrite(A5, HIGH);

    // Variable Declarations
    char charSecond[2];
    char charMinute[2];
    char charHour[2];
    char charDay[2];
    char charMonth[4];
    uint16_t colonColorBright;
    uint16_t colonColorDim;

    // Update Current Time Variables
    intHour         = Time.hourFormat12();
    intMinute       = Time.minute();
    intSecond       = Time.second();
    intDay          = Time.day();
    intMonth        = Time.month();

    // Start with a blank canvas; all elements are re-drawn for every update.
    matrix.fillScreen(0x0000);

    // Set Month
    switch(intMonth) {
        case 1:
            strcpy(charMonth, "JAN");
        break;

        case 2:
            strcpy(charMonth, "FEB");
        break;

        case 3:
            strcpy(charMonth, "MAR");
        break;

        case 4:
            strcpy(charMonth, "APR");
        break;

        case 5:
            strcpy(charMonth, "MAY");
        break;

        case 6:
            strcpy(charMonth, "JUN");
        break;

        case 7:
            strcpy(charMonth, "JUL");
        break;

        case 8:
            strcpy(charMonth, "AUG");
        break;

        case 9:
            strcpy(charMonth, "SEP");
        break;

        case 10:
            strcpy(charMonth, "OCT");
        break;

        case 11:
            strcpy(charMonth, "NOV");
        break;

        case 12:
            strcpy(charMonth, "DEC");
        break;
    }

  // Set Month
    matrix.setTextColor(matrix.Color333(1, 1, 1));
    matrix.setTextSize(1);
    matrix.setCursor(1, 8);
    matrix.print(charMonth);

    // Set Day
    sprintf(charDay, "%2d", intDay);
    matrix.setTextColor(matrix.Color333(1, 1, 1));
    matrix.setTextSize(1);
    matrix.setCursor(20, 8);
    matrix.print(charDay);

    // Set Hour
    sprintf(charHour, "%2d", intHour);             // No Loading Zero (%2d)
    matrix.setTextColor(matrix.Color333(7, 7, 7));
    matrix.setTextSize(1);
    matrix.setCursor(2, 0);
    matrix.print(charHour);

    // Set Minute
    sprintf(charMinute, "%02d", intMinute);        // Leading Zero (%02d)
    matrix.setTextColor(matrix.Color333(7, 7, 7));
    matrix.setTextSize(1);
    matrix.setCursor(19, 0);
    matrix.print(charMinute);

    // Set Second
        // Print (For Debugging)
        // sprintf(charSecond, "%2d", intSecond);
        // matrix.setTextColor(matrix.Color333(7, 7, 7));
        // matrix.setTextSize(1);
        // matrix.setCursor(2, 8);
        // matrix.print("  ");
        // matrix.setCursor(2, 8);
        // matrix.print(charSecond);

        // Set colon (:) color to indicate AM / PM
        if (Time.isAM()) {
            colonColorBright = matrix.Color333(7, 0, 0); // RED
            colonColorDim = matrix.Color333(1, 0, 0);    // Subdued RED
        } else {
            colonColorBright = matrix.Color333(0, 7, 0); // BLUE
            colonColorDim = matrix.Color333(0, 1, 0);    // Subdued BLUE
        }

        // Blink colon (:) every second
        if (Time.second() % 2) {
            matrix.drawRect(15, 1, 2, 2, colonColorBright);
            matrix.drawRect(15, 4, 2, 2, colonColorBright);
        } else {
            matrix.drawRect(15, 1, 2, 2, colonColorDim);
            matrix.drawRect(15, 4, 2, 2, colonColorDim);
        }

  // Update Matrix
  matrix.swapBuffers(false);

    // Toggle GPIO pin to time ISR speed w/ externally connected oscilloscope.
    digitalWrite(A5, LOW);

} // END refresh_display


void flash_status_pixel(int loops, int del, char col) {
    int i;
    uint16_t color;

    switch (col) {
        case 'R':
            color = matrix.Color333(7, 0, 0);
        break;

        case 'G':
            color = matrix.Color333(0, 0, 7);
        break;

        case 'B':
            color = matrix.Color333(0, 7, 0);
        break;

        case 'W':
            color = matrix.Color333(7, 7, 7);
        break;
    }

    for (i = 0; i <= loops; i++) {
        matrix.drawPixel(0, 0, color);
        matrix.swapBuffers(false);
        delay(del);
        matrix.drawPixel(0, 0, matrix.Color333(0, 0, 0));
        matrix.swapBuffers(false);
        delay(del);
    }

}  // END flash_status_pixel


void demo_animation() {
    byte i;

    // Clear background
    matrix.fillScreen(0);

    // Bounce three balls around
    for(i=0; i<3; i++) {
        // Draw 'ball'
        matrix.fillCircle(ball[i][0], ball[i][1], 5, ballcolor[i]);
        // Update X, Y position
        ball[i][0] += ball[i][2];
        ball[i][1] += ball[i][3];
        // Bounce off edges
        if((ball[i][0] == 0) || (ball[i][0] == (matrix.width() - 1)))
            ball[i][2] *= -1;
        if((ball[i][1] == 0) || (ball[i][1] == (matrix.height() - 1)))
            ball[i][3] *= -1;
    }

    // Draw big scrolly text on top
    matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
    matrix.setCursor(textX, 1);
    matrix.print(str);

    // Move text left (w/wrap), increase hue
    if((--textX) < textMin) textX = matrix.width();
    hue += 7;
    if(hue >= 1536) hue -= 1536;

} // END demo_animation
