// **** TSOP is connected to port PB2 **** //////
#define MOUSE_SENSITIVITY 1
#define REPEAT_DELAY 220

// Set to 0 after finding your codes
#define DEBUG 0

// Specify your remote codes here:

#define REMOTE_OK 0x4FB52AD
#define REMOTE_LEFT 0x4FB926D
#define REMOTE_RIGHT 0x4FBD22D
#define REMOTE_DOWN 0x4FBB24D
#define REMOTE_UP 0x4FBE21D
#define REMOTE_MOUSE_SWITCH 0x4FB02FD
#define REMOTE_SLEEP 0x4FB728D

#define REMOTE_MUTE 0x4FBCA35
#define REMOTE_MENU 0x4FB609F
#define REMOTE_HOME 0x4FB42BD
#define REMOTE_RETURN 0x4FB50AE
#define REMOTE_PLAYPAUSE 0x4FBC23D
#define REMOTE_NEXT 0x4FB10EF
#define REMOTE_PREV 0x4FBEA15
#define REMOTE_YM_LIKE 0x4FBBA45
#define REMOTE_YM_DISLIKE 0x4FB3AC5

#include "TrinketHidCombo.h"
#include <Arduino.h>
#include <avr/delay.h>
volatile uint8_t m = 0, tcnt = 0, startflag = 0;
uint32_t irdata = 0, keydata = 0;
bool mouse = false;
bool pressed = false;
bool complete = false;
int lastMouseX = 0, lastMouseY = 0;

void Action(uint32_t keycode);
void ms_delay(uint16_t x);
void setup() {
  // delay(30000);
  DDRB |= (1 << DDB1); // P1 (LED) OUT not used in sketch
  PORTB |= 1 << PB2;   // a PB2 lift will not hurt.
  GIMSK |= 1 << INT0;  // interrupt int0 enable
  MCUCR |=
      1 << ISC00; // Any logical change on INT0 generates an interrupt request
  GTCCR |= 1 << PSR0;
  TCCR0A = 0;
  TCCR0B = (1 << CS02) | (1 << CS00); // divider /1024
  TIMSK = 1 << TOIE0;      // interrupt Timer/Counter1 Overflow  enable
  TrinketHidCombo.begin(); // start the USB device engine and enumerate
}

void loop() {

  if (complete) { // if a code has been received

    if (keydata != 0) // if a code is new
    {
      Action(keydata);
      pressed = true;
    } else if (mouse) // Make mouse movements accelerate
    {
      lastMouseX *= 2;
      if (lastMouseX > 64)
        lastMouseX = 64;
      else if (lastMouseX < -64)
        lastMouseX = -64;
      lastMouseY *= 2;
      if (lastMouseY > 64)
        lastMouseY = 64;
      else if (lastMouseY < -64)
        lastMouseY = -64;
    }

    TrinketHidCombo.mouseMove(lastMouseX, lastMouseY, 0);

    complete = false;
    ms_delay(REPEAT_DELAY); // to balance repeating/input delay of the remote

  } else if (pressed) {
    digitalWrite(1, LOW);
    if (mouse)
      TrinketHidCombo.mouseMove(0, 0, 0);
    else
      TrinketHidCombo.pressKey(0, 0);
    pressed = false;
  } else {
    _delay_ms(1);           // restrain USB polling on empty cycles
    TrinketHidCombo.poll(); // check if USB needs anything done
  }
}

ISR(INT0_vect) {
  if (PINB & 1 << 2) { // If log1
    TCNT0 = 0;
  } else {
    tcnt = TCNT0; // If log0
    if (startflag) {
      if (30 > tcnt && tcnt > 2) {
        if (tcnt > 15 && m < 32) {
          irdata |= (2147483648 >> m);
        }
        m++;
      }
    } else
      startflag = 1;
  }
}
ISR(TIMER0_OVF_vect) {
  if (m)
    complete = true;
  m = 0;
  startflag = 0;
  keydata = irdata;
  irdata = 0; // if the index is not 0, then create an end flag
}

void ms_delay(uint16_t x) // USB polling delay function
{
  for (uint16_t m = 0; m < (x / 10); m++) {
    _delay_ms(10);
    TrinketHidCombo.poll();
  }
}

void Action(uint32_t keycode) {
  switch (keycode) {
  case REMOTE_OK:
    if (mouse) {
      lastMouseX = 0;
      lastMouseY = 0;
      TrinketHidCombo.mouseMove(lastMouseX, lastMouseY, MOUSEBTN_LEFT_MASK);
    } else
      TrinketHidCombo.pressKey(0, KEYCODE_ENTER);
    break;

  case REMOTE_LEFT:
    if (mouse) {
      lastMouseX = -MOUSE_SENSITIVITY;
      lastMouseY = 0;
      TrinketHidCombo.mouseMove(lastMouseX, lastMouseY, 0);
    }

    else
      TrinketHidCombo.pressKey(0, KEYCODE_ARROW_LEFT);
    break;

  case REMOTE_RIGHT:
    if (mouse) {
      lastMouseX = MOUSE_SENSITIVITY;
      lastMouseY = 0;
      TrinketHidCombo.mouseMove(lastMouseX, lastMouseY, 0);
    } else
      TrinketHidCombo.pressKey(0, KEYCODE_ARROW_RIGHT);
    break;
  case REMOTE_DOWN:
    if (mouse) {
      lastMouseX = 0;
      lastMouseY = MOUSE_SENSITIVITY;
      TrinketHidCombo.mouseMove(lastMouseX, lastMouseY, 0);
    } else
      TrinketHidCombo.pressKey(0, KEYCODE_ARROW_DOWN);
    break;
  case REMOTE_UP:
    if (mouse) {
      lastMouseX = 0;
      lastMouseY = -MOUSE_SENSITIVITY;
      TrinketHidCombo.mouseMove(lastMouseX, lastMouseY, 0);
    } else
      TrinketHidCombo.pressKey(0, KEYCODE_ARROW_UP);
    break;
  case REMOTE_SLEEP:
    TrinketHidCombo.pressSystemCtrlKey(SYSCTRLKEY_SLEEP);
    break;
  case REMOTE_RETURN:
    TrinketHidCombo.pressKey(0, KEYCODE_ESC);
    break;
  case REMOTE_MOUSE_SWITCH:
    mouse = !mouse;
    lastMouseX = 0;
    lastMouseY = 0;
    break;
  case REMOTE_MUTE:
    TrinketHidCombo.pressMultimediaKey(MMKEY_MUTE);
    break;
  case REMOTE_HOME:
    TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_CONTROL, KEYCODE_ESC);
    break;

  case REMOTE_MENU:
    TrinketHidCombo.pressKey(0, KEYCODE_APP);
    break;
  case REMOTE_PREV:
    TrinketHidCombo.pressMultimediaKey(MMKEY_SCAN_PREV_TRACK);
    break;
  case REMOTE_NEXT:
    TrinketHidCombo.pressMultimediaKey(MMKEY_SCAN_NEXT_TRACK);
    break;
  case REMOTE_PLAYPAUSE:
    TrinketHidCombo.pressMultimediaKey(MMKEY_PLAYPAUSE);
    break;
  case REMOTE_YM_LIKE:
    TrinketHidCombo.pressKey(0, KEYCODE_F);
    break;
  case REMOTE_YM_DISLIKE:
    TrinketHidCombo.pressKey(0, KEYCODE_D);
    break;
  default:
    if (DEBUG)
      TrinketHidCombo.print(keydata, HEX);
    else
      return;
  }
  digitalWrite(1, HIGH);
}
