/*
 * FreeRTOS practice with ESP32
 * Fade an LED in over 4 seconds, keep it on for 7 seconds, then fade it out
*/
#include <driver/ledc.h>
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

int ledResolution=8;
int ledFrequency=100;
byte button=0;
ledc_channel_t channel;
double brightness;

#ifndef LED_BUILTIN
#define LED_BUILTIN 16
#endif

//Interrupt function called whenever the ESP32 button is pushed
void IRAM_ATTR start() {
  Serial.println("Button pushed");
  xTaskCreatePinnedToCore(
    TaskFadeIn
    ,  "TaskFadeIn"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
}

void TaskFadeIn( void *pvParameters );
void TaskFadeOut( void *pvParameters );
void TaskHold(void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  Serial.println("Setup Complete");

  // Setup the Ledc channel
  channel=(ledc_channel_t)0;
  ledcSetup(channel, ledFrequency, ledResolution);
  ledcAttachPin(LED_BUILTIN, 0);

  // Turn the led off
  brightness=256;
  Serial.println(brightness);
  ledcWrite(0,brightness);

  // Set the built-in button as an input, and attach an interrupt to it
  pinMode(button, INPUT);
  attachInterrupt(digitalPinToInterrupt(button), start, RISING);

}

void loop()
{
  // Empty, everything is done in the tasks
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskFadeIn(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Fades in an led over 4 seconds. 
*/

  for(;;) // A Task shall never return or exit.
  {
    int timer = 4000;

    // Sets up everything necessary to fade in the LED over time timer (4000ms). 
    ledc_fade_func_install(0);
    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE,channel,0,timer);
    ledc_fade_start(LEDC_HIGH_SPEED_MODE, channel, LEDC_FADE_WAIT_DONE);
    
    // Starts the next task to be executed (keeps the LED on)
    xTaskCreatePinnedToCore(
    TaskHold
    ,  "TaskHold"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
    
    // Deletes this task
    vTaskDelete(NULL);
    
  }
}

void TaskFadeOut(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Fades out an led over 2 seconds.
*/
  for(;;) {
    int timer = 2000;

    // Sets up everything necessary to fade out the LED over time timer (2000ms).
    ledc_fade_func_install(0);
    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE,channel,256,timer);
    ledc_fade_start(LEDC_HIGH_SPEED_MODE, channel, LEDC_FADE_WAIT_DONE);

    // Deletes this task.
    vTaskDelete(NULL);
  }
}

void TaskHold(void *pvParameters) {
  (void) pvParameters;

  for(;;) {
    // Delays the task for 7000ms worth of ticks.
    int timer = 7000;
    TickType_t xTimeInTicks = pdMS_TO_TICKS(timer);
    vTaskDelay(xTimeInTicks);

    // Starts the next task to be executed (fade LED out)
    xTaskCreatePinnedToCore(
    TaskFadeOut
    ,  "TaskFadeOut"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
    
    // Deletes this task
    vTaskDelete(NULL);
    
    
  }
}
