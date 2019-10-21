// UI:
//
//  The fan remembers the last setting.
//  It waits 10 seconds before writing EEPROM to save writes.
//  Boot with button pressed resets level to 0 and stores it.
//
//  The fan stops at 90% duty cycle
//  The fan is at it's max at 10% duty cycle (or grounded)
//  The fan level is from 0..79, representing duty cycles
//      from 100/90% down to 10/0%.  Each rotary encoder detent
//      the value by 4 (so there are twenty steps)


#include <EEPROM.h>
// #include "myDebug.h"

#define MAX_FAN_LEVEL       79
#define FAN_LEVEL_INC       4
  
#define PIN_ENCODER_A       A3      // rotary encoder input pin
#define PIN_ENCODER_B       A5      // rotary encoder input pin
#define PIN_BUTTON          6       // rotary encoder button pin
#define PIN_PWM_OUT         9       // pwm out to fan
#define EPROM_ADDR          2

#define FAN_LEVEL_WRITE_TIME  5000 // milliseconds

int fan_level = 0;
int fan_level_change_time = 0;


int readEncoder()
{
    int inc = 0;
    static unsigned char last_encoder_A = 0;
    unsigned char encoder_A = digitalRead(PIN_ENCODER_A);
    unsigned char encoder_B = digitalRead(PIN_ENCODER_B);   
        
    if (!encoder_A && last_encoder_A)
    {
        if (encoder_B)      // B is high so clockwise
            inc = FAN_LEVEL_INC;
        else                // B is low so counter-clockwise
            inc = -FAN_LEVEL_INC;
    }   
    last_encoder_A = encoder_A;  // Store value of A for next time
    return inc;
}


void setFanLevel(int level)
{
    float pct = ((float)(MAX_FAN_LEVEL-level)) / MAX_FAN_LEVEL;
    int pwm_value = pct * 255.0;
    if (pwm_value < 7) pwm_value = 7;
    // display(0,"  pwm_value=%d",pwm_value);
    analogWrite(PIN_PWM_OUT, pwm_value);
    
}


void storeFanLevel(int level)
{
    // display(0,"storing=%d",level);
    EEPROM.write(EPROM_ADDR,level);
}


int recallFanLevel()
{
    int level = EEPROM.read(EPROM_ADDR);
    // display(0,"recalled=%d",level);
    if (level < 0 || level > MAX_FAN_LEVEL)
        level = 0;
    return level;
}



void setup()  
{
    // Serial.begin(115200);
    // delay(1000);
    // display(0,"fanTest.ino started",0);
    
    pinMode(13,OUTPUT);
    pinMode(PIN_PWM_OUT, OUTPUT);
    pinMode(PIN_ENCODER_A, INPUT_PULLUP);
    pinMode(PIN_ENCODER_B, INPUT_PULLUP);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    
    if (!digitalRead(PIN_BUTTON))
    {
        fan_level = 0;
        // display(0,"RESET",0);
        storeFanLevel(fan_level);
        for (int i=0; i<21; i++)
        {
            digitalWrite(13,i&1);
            delay(100);
        }
        while (!digitalRead(PIN_BUTTON));
    }
    else
        fan_level = recallFanLevel();
        
    setFanLevel(fan_level);
} 



void loop()  
{
    uint32_t now = millis();
    static uint32_t last_time = 0;
    if (now > last_time)    // every ms
    {
        last_time = now;
        int inc = readEncoder();
        int level = fan_level + inc;
        if (level < 0)
            level = 0;
        if (level > MAX_FAN_LEVEL)
            level = MAX_FAN_LEVEL;
        if (level != fan_level)
        {
            // display(0,"fan_level = %d",level);
            fan_level = level;
            setFanLevel(fan_level);
            fan_level_change_time = millis();
        }
        else if (fan_level_change_time &&
            (now > fan_level_change_time + FAN_LEVEL_WRITE_TIME))
        {
            fan_level_change_time = 0;
            storeFanLevel(fan_level);
            for (int i=0; i<17; i++)
            {
                digitalWrite(13,i&1);
                delay(100);
            }
        }
    }
}
