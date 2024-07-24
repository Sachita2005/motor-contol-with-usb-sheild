#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>
#include <math.h>
const int fPin1 = 2;  // Digital pin 1
const int fPin2 = 3;  // Digital pin 2
const int fEnablePin = 9;  // Digital (PWM) pin for motor speed control
const int sPin1 = 7;  // Digital pin 1
const int sPin2 = 8;  // Digital pin 2
const int sEnablePin = 6;  // Dig0ital (PWM) pin for moto r speed control
int PWM=255;

USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);

struct GamePadEventData {
    uint8_t X, Y, Z1, Z2, Rz;
};

class JoystickEvents {
public:
    int x1, y1, x2, y2, rz;
    int hat;
    int buttonPressed; // Stores the button press event

    virtual void OnGamePadChanged(const GamePadEventData *evt);
    virtual void OnHatSwitch(uint8_t hat);
    virtual void OnButtonUp(uint8_t but_id);
    virtual void OnButtonDn(uint8_t but_id);
};

#define RPT_GEMEPAD_LEN        5

class JoystickReportParser : public HIDReportParser {
    JoystickEvents *joyEvents;

    uint8_t oldPad[RPT_GEMEPAD_LEN];
    uint8_t oldHat;
    uint16_t oldButtons;

public:
    JoystickReportParser(JoystickEvents *evt);

    virtual void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
};

JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);

void setup() {
    Serial.begin(115200);
    pinMode(fPin1, OUTPUT);
  pinMode(fPin2, OUTPUT);
  pinMode(fEnablePin, OUTPUT);
  pinMode(sPin1, OUTPUT);
  pinMode(sPin2, OUTPUT);
  pinMode(sEnablePin, OUTPUT);
    #if !defined(MIPSEL)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
    #endif
    Serial.println("Start");

    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay(200);

    if (!Hid.SetReportParser(0, &Joy))
        ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);
}
String direction ;
void loop() {
    Usb.Task();
    // Print the stored values in loop
    Serial.print("X1: ");
    Serial.print(JoyEvents.x1);
    Serial.print("\tY1: ");
    Serial.print(JoyEvents.y1);
    Serial.print("\tX2: ");
    Serial.print(JoyEvents.x2);
    Serial.print("\tY2: ");
    Serial.print(JoyEvents.y2);
    Serial.print("\tHat: ");
    Serial.print(JoyEvents.hat);
    Serial.print("\tButton Pressed: ");
    Serial.print(JoyEvents.buttonPressed);
    Serial.print("\tDirection : ");
    Serial.println(direction);
    if(JoyEvents.y1<5){
      direction = "forward";
      forward();
    }
    else if(JoyEvents.y1>250){
      direction = "backward";
      backward();
    }
    else if(JoyEvents.x1>250){
      direction = "right";
      right();
    }
    else if(JoyEvents.x1<5){
      direction = "left";
      left();
    }
    else {
      direction = "stop";
      stop();
    }
    if(JoyEvents.hat==2){
      rotateright();
    }
    if(JoyEvents.hat==6){
      rotateleft();
    }
}

JoystickReportParser::JoystickReportParser(JoystickEvents *evt) :
joyEvents(evt),
oldHat(0xDE),
oldButtons(0) {
    for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
        oldPad[i] = 0xD;
}

void JoystickReportParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
    bool match = true;

    // Checking if there are changes in report since the method was last called
    for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
        if (buf[i] != oldPad[i]) {
            match = false;
            break;
        }

    // Calling Game Pad event handler
    if (!match && joyEvents) {
        joyEvents->OnGamePadChanged((const GamePadEventData*)buf);

        for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++) oldPad[i] = buf[i];
    }

    uint8_t hat = (buf[5] & 0xF);

    // Calling Hat Switch event handler
    if (hat != oldHat && joyEvents) {
        joyEvents->OnHatSwitch(hat);
        oldHat = hat;
    }

    uint16_t buttons = (0x0000 | buf[6]);
    buttons <<= 4;
    buttons |= (buf[5] >> 4);
    uint16_t changes = (buttons ^ oldButtons);

    // Calling Button Event Handler for every button changed
    if (changes) {
        for (uint8_t i = 0; i < 0x0C; i++) {
            uint16_t mask = (0x0001 << i);

            if (((mask & changes) > 0) && joyEvents) {
                if ((buttons & mask) > 0) {
                    joyEvents->OnButtonDn(i + 1);
                    joyEvents->buttonPressed = i + 1; // Set button pressed
                } else {
                    joyEvents->OnButtonUp(i + 1);
                }
            }
        }
        oldButtons = buttons;
    }
}

void JoystickEvents::OnGamePadChanged(const GamePadEventData *evt) {
    // Store the values in variables
    x1 = evt->Y;
    y1 = evt->Z1;
    x2 = evt->Z2;
    y2 = evt->Rz;
}

void JoystickEvents::OnHatSwitch(uint8_t hat) {
    this->hat = hat; // Store hat value
}

void JoystickEvents::OnButtonUp(uint8_t but_id) {
    // Not needed for storing button press
}

void JoystickEvents::OnButtonDn(uint8_t but_id) {
    // Not needed for storing button press
}
void forward(){
  analogWrite(fEnablePin, PWM);  // Example PWM value (0-255) for half-speed
  analogWrite(sEnablePin, PWM); 
  digitalWrite(fPin1, LOW);
  digitalWrite(fPin2, HIGH);
  digitalWrite(sPin1, LOW);
  digitalWrite(sPin2, HIGH);
}
void backward(){
  analogWrite(fEnablePin, PWM);  // Example PWM value (0-255) for half-speed
  analogWrite(sEnablePin, PWM); 
  digitalWrite(fPin1, HIGH);
  digitalWrite(fPin2, LOW);
  digitalWrite(sPin1, HIGH);
  digitalWrite(sPin2, LOW);
}
void stop(){
   analogWrite(fEnablePin, 0);  // Example PWM value (0-255) for half-speed
  analogWrite(sEnablePin, 0);
}
void right(){
 analogWrite(fEnablePin, PWM);  // Example PWM value (0-255) for half-speed
  analogWrite(sEnablePin, PWM); 
  digitalWrite(fPin1, LOW);
  digitalWrite(fPin2, LOW);
  digitalWrite(sPin1, LOW);
  digitalWrite(sPin2, HIGH);
}
void left(){
  analogWrite(fEnablePin, PWM);  // Example PWM value (0-255) for half-speed
  analogWrite(sEnablePin, PWM); 
  digitalWrite(fPin1, LOW);
  digitalWrite(fPin2, HIGH);
  digitalWrite(sPin1, LOW);
  digitalWrite(sPin2, LOW);
}
void rotateright(){
 analogWrite(fEnablePin, PWM);  // Example PWM value (0-255) for half-speed
  analogWrite(sEnablePin, PWM); 
  digitalWrite(fPin1, HIGH);
  digitalWrite(fPin2, LOW);
  digitalWrite(sPin1, LOW);
  digitalWrite(sPin2, HIGH);
}
void rotateleft(){
 analogWrite(fEnablePin, PWM);  // Example PWM value (0-255) for half-speed
  analogWrite(sEnablePin, PWM); 
  digitalWrite(fPin1, LOW);
  digitalWrite(fPin2, HIGH);
  digitalWrite(sPin1, HIGH);
  digitalWrite(sPin2, LOW);
}