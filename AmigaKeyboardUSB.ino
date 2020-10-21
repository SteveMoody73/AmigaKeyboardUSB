#include <HID.h>
#include <Keyboard.h>

#define BITMASK_A500CLK     0b00010000    // IO 8
#define BITMASK_A500SP      0b00100000    // IO 9
#define BITMASK_A500RES     0b01000000    // IO 10
#define BITMASK_JOY1        0b10011111    // IO 0..4,6
#define BITMASK_JOY2A       0b11110000    // IO A0..A3     
#define BITMASK_JOY2B       0b00000110    // IO 15,16 (PB1, PB2)

#define SYNCH_HI 0
#define SYNCH_LO 1
#define HANDSHAKE 2
#define READ 3
#define WAIT_LO 4

#define MAX_KEYCODE   0x68 // 104 in decimal
#define RESET_CODE     126 // Keycode from keyboard when you press CTRL+Amiga+Amiga

// Joystick data structure
struct Joystick
{
  uint8_t buttons;
  int8_t x;
  int8_t y;
};

// USB joystick descriptors
// Commodore /  Atari Joystick 1
static const uint8_t _hidJoystick1ReportDescriptor[] PROGMEM = {
  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
  0x09, 0x05,                    // USAGE (Game Pad)
  0xa1, 0x01,                    // COLLECTION (Application)
  0x85, 0x03,                    //   REPORT_ID (3)  
  0xa1, 0x00,                    //   COLLECTION (Physical)
  0x05, 0x09,                    //   USAGE_PAGE (Button)
  0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
  0x29, 0x02,                    //   USAGE_MAXIMUM (Button 2)
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
  0x95, 0x02,                    //   REPORT_COUNT (2) ; 2 buttons
  0x75, 0x01,                    //   REPORT_SIZE (1)
  0x81, 0x02,                    //   INPUT (Data,Var,Abs)
  0x95, 0x06,                    //   REPORT_COUNT (6) ; to pad out the bits into a number divisible by 8
  0x81, 0x03,                    //   INPUT (Const,Var,Abs)
  0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
  0x09, 0x01,                    //   USAGE (Pointer)
  0x15, 0x80,                    //   LOGICAL_MINIMUM (-128)
  0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
  0x95, 0x02,                    //   REPORT_COUNT (2)
  0x75, 0x08,                    //   REPORT_SIZE (8) ; two bits to represent each axis
  0xa1, 0x00,                    //   COLLECTION (Physical)
  0x09, 0x30,                    //     USAGE (X)
  0x09, 0x31,                    //     USAGE (Y)
  0x81, 0x02,                    //     INPUT (Data,Var,Abs)
  0xc0,                          //   END_COLLECTION
  0xc0,                          // END_COLLECTION   
  0xc0                           // END_COLLECTION   
};
// Commodore /  Atari Joystick 2
static const uint8_t _hidJoystick2ReportDescriptor[] PROGMEM = {
  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
  0x09, 0x05,                    // USAGE (Game Pad)
  0xa1, 0x01,                    // COLLECTION (Application)
  0x85, 0x04,                    //   REPORT_ID (4)  
  0xa1, 0x00,                    //   COLLECTION (Physical)
  0x05, 0x09,                    //   USAGE_PAGE (Button)
  0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
  0x29, 0x02,                    //   USAGE_MAXIMUM (Button 2)
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
  0x95, 0x02,                    //   REPORT_COUNT (2) ; 2 buttons
  0x75, 0x01,                    //   REPORT_SIZE (1)
  0x81, 0x02,                    //   INPUT (Data,Var,Abs)
  0x95, 0x06,                    //   REPORT_COUNT (6) ; to pad out the bits into a number divisible by 8
  0x81, 0x03,                    //   INPUT (Const,Var,Abs)
  0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
  0x09, 0x01,                    //   USAGE (Pointer)
  0x15, 0x80,                    //   LOGICAL_MINIMUM (-128)
  0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
  0x95, 0x02,                    //   REPORT_COUNT (2)
  0x75, 0x08,                    //   REPORT_SIZE (8) ; two bits to represent each axis
  0xa1, 0x00,                    //   COLLECTION (Physical)
  0x09, 0x30,                    //     USAGE (X)
  0x09, 0x31,                    //     USAGE (Y)
  0x81, 0x02,                    //     INPUT (Data,Var,Abs)
  0xc0,                          //   END_COLLECTION
  0xc0,                          // END_COLLECTION 
  0xc0                           // END_COLLECTION   
};


KeyReport _keyReport;
int intStart = 1;
uint32_t counter = 0;
uint8_t Joy, MemoJoy1, MemoJoy2, state, bitn, key, fn, keydown;

uint8_t ktab[0x68] = {
    // Tilde, 1-9, 0, sz, Accent, backslash, Num 0 (00 - 0F)
    0x35, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2D, 0x2E, 0x31, 0, 0x62,
    // Q bis +, -, Num 1, Num 2, Num3 (10 - 1F)
    0x14, 0x1A, 0x08, 0x15, 0x17, 0x1C, 0x18, 0x0C, 0x12, 0x13, 0x2F, 0x30, 0, 0x59, 0x5A, 0x5B,
    // A-#, -, Num 4, Num 5, Num 6 (20 - 2F)
    0x04, 0x16, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x33, 0x34, 0x32, 0, 0x5C, 0x5D, 0x5E,
    // <>,Y- -, -, Num . , Num 7, Num 8, Num 9 (30 - 3F)
    0x64, 0x1D, 0x1B, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0, 0x63, 0x5F, 0x60, 0x61,
    // Space, BS, Tab, Enter, Return, ESC, Del, -, -, -, Num -, -, up, down, right, left (40 - 4F)
    0x2C, 0x2A, 0x2B, 0x58, 0x28, 0x29, 0x4C, 0, 0, 0, 0x56, 0, 0x52, 0x51, 0x4F, 0x50,
    // F1-F10, -, -, Num /, Num *, Num +, - (50 - 5F)
    0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0, 0, 0x54, 0x55, 0x57, 0,
    // modifiers: Shift left, Shift right, -, Crtl left, Alt left, Alt right, Win (amiga) left, Ctrl (amiga)right
    0x02, 0x20, 0x00, 0x01, 0x04, 0x40, 0x08, 0x10
};

// Used for sending HID joystick data
Joystick joy1 = {B00000011, 0, 0};
Joystick joy2 = {B00000011, 0, 0};

void setup()
{
  // HID descriptor for two digital joysticks
  static HIDSubDescriptor node1(_hidJoystick1ReportDescriptor, sizeof(_hidJoystick1ReportDescriptor));
  HID().AppendDescriptor(&node1);
  static HIDSubDescriptor node2(_hidJoystick2ReportDescriptor, sizeof(_hidJoystick2ReportDescriptor));
  HID().AppendDescriptor(&node2);
  
  // Joystick 1 (Port D)
  DDRD = ~(BITMASK_JOY1); // Direction INPUT
  PORTD = BITMASK_JOY1;   // Activate PULLUP
  
  // Joystick 2 (Port F)
  DDRF = ~(BITMASK_JOY2A); // Direction INPUT
  PORTF = BITMASK_JOY2A;   // Activate PULLUP

  // Keyboard (Port B - the header pins)
  DDRB = ~(BITMASK_A500CLK | BITMASK_A500SP);  // direction INPUT

  // Numlock LED (Digital pin 5 of the Leonardo/Pro Micro)
  //DDR_LED  |= MASK_LED; 
  //PORT_LED |= MASK_LED; // <--Comment this out if numlock status is off on bootup

  // Initialise the HID keyboard
  Keyboard.begin();
}

void loop()
{
  // Joystick 1
  // ------------------------------------
  Joy = ~PIND & BITMASK_JOY1;
  if (Joy != MemoJoy1)
  {
    // Axes
    joy1.x = (Joy & B00000010) ? -128 : ((Joy & B00000001) ? 127 : 0);
    joy1.y = (Joy & B00001000) ? -128 : ((Joy & B00000100) ? 127 : 0);
    // Buttons
    joy1.buttons = 0 | ((Joy & B00010000)>>4) | ((Joy & B10000000)>>6);
    // Send joystick 1 data
    HID().SendReport(3, &joy1, sizeof(Joystick));
    // Save old data
    MemoJoy1 = Joy;
  }  

  // Joystick 2
  // ------------------------------------
  Joy = ~PINF & BITMASK_JOY2A;
  if (Joy != MemoJoy2)
  {
    // Axes
    joy1.x = (Joy & B00100000) ? -128 : ((Joy & B00010000) ? 127 : 0);
    joy1.y = (Joy & B10000000) ? -128 : ((Joy & B01000000) ? 127 : 0);
    // Buttons
    joy2.buttons = 0 | (Joy & B00000011);
    // Send joystick 2 data
    HID().SendReport(4, &joy2, sizeof(Joystick));
    // Save old data
    MemoJoy2 = Joy;
  }
  
  // Keyboard
  if (state == SYNCH_HI)
  { // Sync-Pulse HI
    if ((PINB & BITMASK_A500CLK) == 0)
      state = SYNCH_LO;
  }

  else if (state == SYNCH_LO)
  { // Sync-Pulse LOW
    if ((PINB & BITMASK_A500CLK) != 0)
      state = HANDSHAKE;
  }

  else if (state == HANDSHAKE)
  { // Handshake
    if (counter == 0)
    {
      DDRB |= BITMASK_A500SP;   // set IO direction to OUTPUT
      PORTB &= ~BITMASK_A500SP; // set OUTPUT to LOW
      counter = millis();
    }
    else if (millis() - counter > 10)
    {
      counter = 0;
      DDRB &= ~BITMASK_A500SP; // set IO direction to INPUT
      state = WAIT_LO;
      key = 0;
      bitn = 7;
    }
  }

  else if (state == READ)
  {
    // read key message (8 bits)
    if ((PINB & BITMASK_A500CLK) != 0)
    {
      if (bitn--)
      {
        key += ((PINB & BITMASK_A500SP) == 0) << (bitn); // key code (add bits 0...6)
        state = WAIT_LO;
      }
      else
      {                                           // read last bit (key down)
        keydown = ((PINB & BITMASK_A500SP) != 0); // true if key down
        interrupts();
        state = HANDSHAKE;
        if (key == 0x5F)
          fn = keydown; // "Help" key: special function on/off
        else if (key == 0x62)
          keystroke(0x39, 0x00); // CapsLock
        else
        {
          if (keydown)
          {
            // keydown message received------
            if (fn)
            {
              // special function with "Help" key
              if (key == 0x50)
                keystroke(0x44, 0); // F11
              else if (key == 0x51)
                keystroke(0x45, 0); // F12
              else if (key == 0x5A)
                keystroke(0x53, 0); // NumLock
              else if (key == 0x5B)
                keystroke(0x47, 0); // ScrollLock
              else if (key == 0x5D)
                keystroke(0x46, 0); // PrtSc
            }
            else
            {
              if (key == 0x5A)
                keystroke(0x26, 0x20); // (
              else if (key == 0x5B)
                keystroke(0x27, 0x20); // )
              else if (key < 0x68)
                keypress(key); // Code table
            }
          }
          else
          {
            // keyrelease message received
            if (key < 0x68)
              keyrelease(key); // Code table
          }
        }
      }
    }
  }

  else if (state == WAIT_LO)
  { // waiting for the next bit
    if ((PINB & BITMASK_A500CLK) == 0)
    {
      noInterrupts();
      state = READ;
    }
  }
}

void keypress(uint8_t k)
{

  if (k > 0x5f)
    _keyReport.modifiers |= ktab[key]; // modifier
  else
  {
    for (uint8_t i = 0; i < 6; i++)
    {
      if (_keyReport.keys[i] == 0)
      {
        _keyReport.keys[i] = ktab[key];
        break;
      }
    }
  }
  HID().SendReport(2, &_keyReport, 8);
}

void keyrelease(uint8_t k)
{

  if (k > 0x5f)
    _keyReport.modifiers &= ~ktab[key]; // modifier
  else
  {
    for (uint8_t i = 0; i < 6; i++)
    {
      if (_keyReport.keys[i] == ktab[key])
        _keyReport.keys[i] = 0;
    }
  }
  HID().SendReport(2, &_keyReport, 8);
}

void keystroke(uint8_t k, uint8_t m)
{
  unsigned short memomodifiers = _keyReport.modifiers; // Save last modifier state
  for (uint8_t i = 0; i < 6; i++)
  {
    if (_keyReport.keys[i] == 0)
    {
      _keyReport.keys[i] = k;
      _keyReport.modifiers = m;
      HID().SendReport(2, &_keyReport, 8);
      _keyReport.keys[i] = 0;
      _keyReport.modifiers = memomodifiers; // Recover modifier state
      HID().SendReport(2, &_keyReport, 8);
      break;
    }
  }
}
 
