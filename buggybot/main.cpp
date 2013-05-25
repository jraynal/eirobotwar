// Interactive Test Session for LeafLabs Maple
// Copyright (c) 2010 LeafLabs LLC.
//
// Useful for testing Maple features and troubleshooting.
// Communicates over SerialUSB.

#include <string.h>
#include <wirish/wirish.h>

#include <math.h>

#include "engine.h"
#include "animation.h"

// ASCII escape character
#define ESC       ((uint8)27)

// Default USART baud rate
#define BAUD     9600

// Pin PWM

void init_all_timers(uint16 prescale);
void enable_usarts(void);
//Global Variables
uint8 servoi;
int enginePin[MAX_ENGINE] = {3,4,5,9,8,10,11,25,16,15};

// Commands

// -- setup() and loop() ------------------------------------------------------

void setup() {
    //Le anim_load devra etre appele quand bouton presse
    anim_load(ANIM_NONE);
    pinMode(BOARD_LED_PIN, OUTPUT);
              
    // Start up the serial ports
    Serial3.begin(921600);
    SerialUSB.begin();

    // Initialisation pin servo
    
    //disable_usarts();
    init_all_timers(21);

    for (servoi=0;servoi<MAX_ENGINE;servoi++){
        pinMode(enginePin[servoi], PWM);
        pwmWrite(enginePin[servoi], 4000);
    }
}

int counter = 0;

char buffer[2048];
unsigned int cursor = 0;

char evt = 'B';

void loop () {
  static uint8 n_eng;

  if (Serial3.available()){
    evt = Serial3.read();
    SerialUSB.write(&evt, 1);
  }
  
  switch(evt) {
  case 'Z':
    anim_load(ANIM_WALK_FORWARD);
    break;
  case 'S':
    anim_load(ANIM_WALK_BACKWARD);
    break;
  default:
    anim_load(ANIM_NONE);
    break;
  }

  SerialUSB.println(date);

  for (n_eng=0;n_eng<MAX_ENGINE;n_eng++) {
    pwmWrite(enginePin[n_eng], anim_get_pwm(n_eng));
  }

  delay(25);
  anim_next_date(5);

  toggleLED();  
}

#define SERVOS_TIMERS_PRESCALE 24
#define SERVOS_TIMERS_OVERFLOW 60000

void initTimer(uint8_t i)
{
    HardwareTimer timer(i);
    timer.pause();
    timer.setPrescaleFactor(SERVOS_TIMERS_PRESCALE);
    timer.setOverflow(SERVOS_TIMERS_OVERFLOW);
    timer.refresh();
    timer.resume();
}

void init_all_timers(uint16 prescale) {
    //init_all_timers_prescale = prescale;
    //timer_foreach(set_prescale);
    initTimer(1);
    initTimer(2);
    initTimer(3);
    initTimer(4);
}


// -- premain() and main() ----------------------------------------------------

// Force init to be called *first*, i.e. before static object allocation.
// Otherwise, statically allocated objects that need libmaple may fail.
__attribute__((constructor)) void premain() {
    init();
}

int main(void) {
    setup();

    while (1) {
        loop();
    }
    return 0;
}
