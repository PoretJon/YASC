#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <pio_encoder.h>

// put function declarations here:
void setButtonValue(int button, int value);

void volLeftChanged();
void volRightChanged();


/////////// tinyusb setup ////////////////

// mostly stolen from example code
int usb_hid_poll_interval = 1; // Set HID poll interval to 1ms (1kHz)

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_GAMEPAD()
};

// USB HID object
Adafruit_USBD_HID usb_hid;

// Report payload defined in src/class/hid/hid.h
// - For Gamepad Button Bit Mask see  hid_gamepad_button_bm_t
// - For Gamepad Hat    Bit Mask see  hid_gamepad_hat_t
hid_gamepad_report_t gp;

/////////// BUTTON DECLARAIONS ///////////

// bt buttons
int bt_a = 0;
int bt_b = 1;
int bt_c = 2;
int bt_d = 3;

// fx buttons
int fx_l = 8;
int fx_r = 9;

//menu
int start = 17;

// encoders
int vol_l_left = 5;
int vol_l_right = 6;

int vol_r_left = 26;
int vol_r_right = 27;

PioEncoder vol_r(vol_r_left);
PioEncoder vol_l(vol_l_left);

int last_volR;
int last_volL;

int sensitivity = 15; 
int decay = 300;

void setup() {
  // put your setup code here, to run once:

  TinyUSBDevice.setManufacturerDescriptor("Amiphyst");
  TinyUSBDevice.setProductDescriptor("YASC");
  
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  // Setup HID
  usb_hid.setPollInterval(1);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();

  // If already enumerated, additional class driver begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  pinMode(bt_a, INPUT_PULLUP);
  pinMode(bt_b, INPUT_PULLUP);
  pinMode(bt_c, INPUT_PULLUP);
  pinMode(bt_d, INPUT_PULLUP);

  pinMode(fx_l, INPUT_PULLUP);
  pinMode(fx_r, INPUT_PULLUP);

  pinMode(start, INPUT_PULLUP);

  vol_r.begin();
  vol_l.begin();

  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }


  if (!usb_hid.ready()) return;

  // poll for inputs
  int bta = !digitalRead(bt_a);
  int btb = !digitalRead(bt_b);
  int btc = !digitalRead(bt_c);
  int btd = !digitalRead(bt_d);

  int fxl = !digitalRead(fx_l);
  int fxr = !digitalRead(fx_r);

  int btnStart = !digitalRead(start);

  int volr;
  int volL;

  // knob inputs? probably interrupts for these tbh
  // gp.buttons = 0;

  setButtonValue(0, bta);
  setButtonValue(1, btb);
  setButtonValue(2, btc);
  setButtonValue(3, btd);

  setButtonValue(4, fxl);
  setButtonValue(5, fxr);

  setButtonValue(6, btnStart);

  // knob code: right knob affects y axis, left knob affects x axis

  // basic explanation of function: x and y axis values wrap

  int rCount = vol_r.getCount();
  int lCount = vol_l.getCount();

  gp.x += lCount;
  gp.y += rCount;

  Serial.println(digitalRead(vol_l_left));

  usb_hid.sendReport(0, &gp, sizeof(gp));


  vol_r.reset();
  vol_l.reset();

  delay(20); //debounce
}

// put function definitions here:

// sets the button value
void setButtonValue(int button, int value) 
{

  if (value) {
    gp.buttons |= (1<<button);
  }
  else {
    gp.buttons &= ~(1<<button);
  }

}

