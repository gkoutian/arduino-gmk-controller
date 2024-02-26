/* Arduino GMK Controller - Gamepad/Mouse/Keyboard controller */
/* created by gkoutian - 2024 / https://github.com/gkoutian */

#include <Joystick.h>
#include <Keyboard.h>
#include <Keypad.h>
#include <Mouse.h>

/* ----- Type selector defs section ----- */
#define TYPESEL_PIN A3 // Pin of type selector switch
boolean typeJoystick = true; // Stores if the selectors is type joystick or not
/* -------------------------------------- */

/* ----- Mouse defs section ----- */
#define MOUSE_HORZ_PIN A1  // Pin of horizontal mouse joystick
#define MOUSE_VERT_PIN A0  // Pin of vertical mouse joystick
#define MOUSE_SEL_PIN A2  // Pin of mouse joystick select button
#define MOUSE_SENSITIVITY 100 // Higher sensitivity value = slower mouse, should be <= about 500
#define MOUSE_INVERT -1 // If need to Invert joystick change from -1 to 1
int mouseVertZero, mouseHorzZero;  // Stores the initial value of each axis, usually around 512
int mouseClickFlag = 0; // Stores if the mouse was clicked
/* ----------------------------- */

/* ----- Keyboard defs section ----- */
const unsigned char keboardFsTable[16] = { // Assign the special keys from keyboard library for use with buttons
  NULL, KEY_F13, KEY_F14, KEY_F15,
  NULL, KEY_F16, KEY_F17, KEY_F18,
  NULL, KEY_F19, KEY_F20, KEY_F21,
  NULL, KEY_F22, KEY_F23, KEY_F24,
};
/* ----------------------------- */

/* ----- Rotaries encoders defs section ----- */
#define ENABLE_PULLUPS
#define ROTARIES_COUNT 3

struct rotariesdef {
    byte pin1; // Pin of rotary encoder
    byte pin2; // Pin 2 of rotary encoder
    int ccwchar; // Button number of ccw move used for the joystick
    int cwchar; // Button number of cw move used for the joystick
    int state; // Rotary state value, used for direction of rotation
};

rotariesdef rotaries[ROTARIES_COUNT] { // List of all the rotary encoders
    {9,8,16,17,0},
    {11,10,18,19,0},
    {13,12,20,21,0},
};

#define DIR_CCW 0x10   // == 16
#define DIR_CW 0x20 // == 32
#define R_START 0x0 // == 0

#ifdef HALF_STEP
#define R_CCW_BEGIN 0x1 // == 1
#define R_CW_BEGIN 0x2 // == 2
#define R_START_M 0x3 // == 3
#define R_CW_BEGIN_M 0x4 // == 4
#define R_CCW_BEGIN_M 0x5 // == 5
const unsigned char ttable[6][4] = {
    // R_START (00)
    {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
    // R_CCW_BEGIN
    {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
    // R_CW_BEGIN
    {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
    // R_START_M (11)
    {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
    // R_CW_BEGIN_M
    {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
    // R_CCW_BEGIN_M
    {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
};
#else
#define R_CW_FINAL 0x1 // == 1
#define R_CW_BEGIN 0x2 // == 2
#define R_CW_NEXT 0x3 // == 3
#define R_CCW_BEGIN 0x4 // == 4
#define R_CCW_FINAL 0x5 // == 5
#define R_CCW_NEXT 0x6 // == 6

const unsigned char ttable[7][4] = {
    // R_START
    {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
    // R_CW_FINAL
    {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
    // R_CW_BEGIN
    {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
    // R_CW_NEXT
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
    // R_CCW_BEGIN
    {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
    // R_CCW_FINAL
    {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
    // R_CCW_NEXT
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};
#endif
/* ------------------------------------- */

/* ----- Joystick Buttons defs section ----- */
#define BUTTONS_ROWS_COUNT 4
#define BUTTONS_COLS_COUNT 4

byte buttons[BUTTONS_ROWS_COUNT][BUTTONS_COLS_COUNT] = {
    {0,1,2,3},
    {4,5,6,7},
    {8,9,10,11},
    {12,13,14,15},
};

byte rowPins[BUTTONS_ROWS_COUNT] = {0,1,2,3};  // Pins numbers of matrix buttons rows
byte colPins[BUTTONS_COLS_COUNT] = {4,5,6,7};   // Pins numbers of matrix buttons columns

Keypad buttbx = Keypad( makeKeymap(buttons), rowPins, colPins, BUTTONS_ROWS_COUNT, BUTTONS_COLS_COUNT);

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,  // Joystick library configuration
  JOYSTICK_TYPE_JOYSTICK, 32, 0,
  false, false, false, false, false, false,
  false, false, false, false, false);
/* ------------------------------------- */

void setup() {
  Serial.begin(9600);
    mouseInit();
    rotaryInit();
    typeSelectorInit();
}

void loop() {
    checkMouse();
    checkRotaryEncoders();
    checkTypeSelectorValue();
    checkButtons();
}

void typeSelectorInit() {
    pinMode(TYPESEL_PIN, INPUT_PULLUP);
    if (digitalRead(TYPESEL_PIN) == LOW) {  // If keyboard is selected
        Keyboard.begin();
        typeJoystick = false; // Store selection
    } else {
        Joystick.begin();
        typeJoystick = true;
    }
}

void checkTypeSelectorValue() {
    if (digitalRead(TYPESEL_PIN) == LOW && typeJoystick) { // If keyboard selected and was joystick
        Joystick.end();
        Keyboard.begin();
        typeJoystick = false;  // Store selection
        return;
    }
    if (digitalRead(TYPESEL_PIN) == HIGH && !typeJoystick) { // If joystick selected and was keyboard
        Keyboard.end();
        Joystick.begin();
        typeJoystick = true;
    }
}

void mouseInit() {
    pinMode(MOUSE_HORZ_PIN, INPUT);
    pinMode(MOUSE_VERT_PIN, INPUT);
    pinMode(MOUSE_SEL_PIN, INPUT);
    digitalWrite(MOUSE_SEL_PIN, HIGH);
    delay(1000);
    mouseVertZero = analogRead(MOUSE_HORZ_PIN); // Read initial vertical mouse value and store to variable
    mouseHorzZero = analogRead(MOUSE_VERT_PIN); // Read initial horizontal mouse value and store to variable
    Mouse.begin();
}

void checkMouse() {
    int vertValue = analogRead(MOUSE_VERT_PIN) - mouseVertZero;  // Read vertical mouse offset
    int horzValue = analogRead(MOUSE_HORZ_PIN) - mouseHorzZero;  // Read horizontal mouse offset

    if (vertValue != 0) {
        Mouse.move(0, (-1 * MOUSE_INVERT * (vertValue / MOUSE_SENSITIVITY)), 0); // Move mouse on y axis
    }
    if (horzValue != 0) {
        Mouse.move((MOUSE_INVERT * (horzValue / MOUSE_SENSITIVITY)), 0, 0); // Move mouse on x axis
    }
    if ((digitalRead(MOUSE_SEL_PIN) == 0) && (!mouseClickFlag)) {  // If the joystick button is pressed
        mouseClickFlag = 1;
        Mouse.press(MOUSE_LEFT);  // Click the left button down
    }
    else if ((digitalRead(MOUSE_SEL_PIN)) && (mouseClickFlag)) { // If the joystick button is not pressed
        mouseClickFlag = 0;
        Mouse.release(MOUSE_LEFT);  // Release the left button
    }
}

void rotaryInit() {
    for (int i=0;i<ROTARIES_COUNT;i++) {
        pinMode(rotaries[i].pin1, INPUT);
        pinMode(rotaries[i].pin2, INPUT);
        #ifdef ENABLE_PULLUPS
        digitalWrite(rotaries[i].pin1, HIGH);
        digitalWrite(rotaries[i].pin2, HIGH);
        #endif
    }
}

unsigned char rotary_process(int _i) {
    unsigned char pinstate = (digitalRead(rotaries[_i].pin2) << 1) | digitalRead(rotaries[_i].pin1);
    rotaries[_i].state = ttable[rotaries[_i].state & 0xf][pinstate];
    return (rotaries[_i].state & 0x30);
}

void checkRotaryEncoders() {
    for (int i=0;i<ROTARIES_COUNT;i++) {
        int rotaryButtonNumber = 0; // Store joystick button number of encoder
        unsigned char result = rotary_process(i);
        if (result == DIR_CCW) { // If encoder was moved ccw
            rotaryButtonNumber = rotaries[i].ccwchar;
        };
        if (result == DIR_CW) { // If encoder was moved cw
            rotaryButtonNumber = rotaries[i].cwchar;
        };
        if (rotaryButtonNumber != 0) { // If only encoder was moved
            Joystick.setButton(rotaryButtonNumber, 1);
            delay(50);
            Joystick.setButton(rotaryButtonNumber, 0);
        }
    }
}

void checkButtons() {
    if (buttbx.getKeys()) {
        for (int i=0; i<LIST_MAX; i++) {
            if ( buttbx.key[i].stateChanged ) { // If state of button changed
                switch (buttbx.key[i].kstate) {
                    case PRESSED:
                    case HOLD:
                        if (typeJoystick) {
                            Joystick.setButton(buttbx.key[i].kchar, 1);
                        } else {
                            Keyboard.press(keboardFsTable[buttbx.key[i].kchar]);
                        }
                        break;
                    case RELEASED:
                    case IDLE:
                        if (typeJoystick) {
                            Joystick.setButton(buttbx.key[i].kchar, 0);
                        } else {
                            Keyboard.release(keboardFsTable[buttbx.key[i].kchar]);
                        }
                        break;
                }
            }
        }
    }
}
