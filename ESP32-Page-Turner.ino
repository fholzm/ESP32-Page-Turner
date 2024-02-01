#define USE_NIMBLE
#include <BleKeyboard.h>

/**
 * ESP32 Bluetooth Page Turner
 * 
 * Acts as a bluetooth keyboard sending 
 * keystroke commands to a client. 
 * 
 * (c) Stuart Coyle 2023
 */
#define SERIAL_DEBUG 1

/*  
 * Bluetooth device name. Anything longer than
 * 16 characters will be truncated.
 */
#define DEVICE_NAME "Page Turner"
#define DEVICE_MANUFACTURER "TheatreControlProducts"
/* GPIO Pin assignments */
#define NEXT_PAGE 12
#define PREVIOUS_PAGE 14
#define BLUETOOTH_LED 22
#define BATTERY_LEVEL 13

/* Keystrokes to send. */
#define NEXT_PAGE_KEY KEY_RIGHT_ARROW
#define PREVIOUS_PAGE_KEY KEY_LEFT_ARROW

/* 
 * Time the device will remain disconnected before going
 * into deep sleep.
 */
#define MAX_DISCONNECTION_TIME 120000

/*
 *  Key debouncing variables.
 *  DEBOUNCE_TIME is milliseconds to wait for a key to become stable. 
 *  REPEAT_TIME is the time in ms to repeat a held down key. 
 */
#define DEBOUNCE_TIME 20
#define REPEAT_TIME 200
#define FLASH_TIME 250
#define BATTERY_UPDATE_TIME 10000

BleKeyboard bleKeyboard(DEVICE_NAME, DEVICE_MANUFACTURER, 100);
int lastConnection = 0;
bool lastConnectionState;
int lastBatteryUpdate = 0;
int debounceMillis = 0;
int repeatMillis = 0;
int flashCount = 0;
int lastFlash = 0;

/*
 *  The button state is stored as a bitmap.
 *  0x01 is the Previous page button and
 *  0x10 is the Next page button.
 */
int buttonState = 0x00;
int lastButtonState = 0x11;

void setup()
{
  lastConnectionState = 0;

  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT0)
  {
    lastConnection = millis() - MAX_DISCONNECTION_TIME;
  }
  else
  {
    lastConnection = millis();
  }

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0);

#if SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("Starting page turner.");
#endif
  flash(6);
  bleKeyboard.begin();

  pinMode(BLUETOOTH_LED, OUTPUT);
  pinMode(NEXT_PAGE, INPUT_PULLUP);
  pinMode(PREVIOUS_PAGE, INPUT_PULLUP);
}

void sendNextPage()
{
  bleKeyboard.press(NEXT_PAGE_KEY);
#if SERIAL_DEBUG
  Serial.println("Next");
#endif
  bleKeyboard.releaseAll();
}

void sendPreviousPage()
{
  bleKeyboard.press(PREVIOUS_PAGE_KEY);
#if SERIAL_DEBUG
  Serial.println("Previous");
#endif
  bleKeyboard.releaseAll();
}

/* 
 * Read the button state with debouncing.
 */
void readButtons()
{
  int tick = millis();
  int reading = 0x00;
  if (!digitalRead(NEXT_PAGE))
  {
    reading |= 0x10;
  }

  if (!digitalRead(PREVIOUS_PAGE))
  {
    reading |= 0x01;
  }

  if (reading != lastButtonState)
  {
    debounceMillis = tick;
  }

  if (tick - debounceMillis > DEBOUNCE_TIME)
  {
    if (buttonState != reading || (tick - repeatMillis) > REPEAT_TIME)
    {
      repeatMillis = tick;
      buttonState = reading;

      if (buttonState & 0x10)
      {
        sendNextPage();
      }

      if (buttonState & 0x01)
      {
        sendPreviousPage();
      }
    }
  }

  lastButtonState = reading;
}

void readBatteryLevel()
{
  // Battery level estimation like in https://github.com/G6EJD/LiPo_Battery_Capacity_Estimator/blob/master/ReadBatteryCapacity_LIPO.ino
  float voltage = analogRead(BATTERY_LEVEL) / 4096.0 * 7.23;
  float percentage = 2808.3808 * pow(voltage, 4) - 43560.9157 * pow(voltage, 3) + 252848.5888 * pow(voltage, 2) - 650767.4615 * voltage + 626532.5703;

  #if SERIAL_DEBUG
  Serial.println("Voltage: " + String(voltage));
  Serial.println("percentage: " + String(int(percentage)) + "%");
  #endif

  // bleKeyboard.setBatteryLevel (int(percentage));
  bleKeyboard.setBatteryLevel (75);
}

void flash(int n){
  flashCount = n*2;   
}

void do_flash() {
  if(flashCount > 0) {
    if(millis() - lastFlash > FLASH_TIME){
      digitalWrite(BLUETOOTH_LED, flashCount % 2);
      flashCount--;
      lastFlash = millis();
    }
  }else{
    digitalWrite(BLUETOOTH_LED, !lastConnectionState);
  }
}

void loop()
{
  int tick = millis();
  int reading;

  if (bleKeyboard.isConnected())
  {
    if (lastConnectionState == 0)
    {
      #if SERIAL_DEBUG
      Serial.println("Connected.");
      #endif
      flash(3);
      lastConnectionState = 1;
    }

    lastConnection = tick;
    readButtons();
  }
  else
  {
    if (lastConnectionState == 1)
    {
      #if SERIAL_DEBUG
      Serial.println("Disconnected.");
      #endif
      lastConnectionState = 0;
      flash(5);
    }
  }

  if (tick - lastBatteryUpdate > BATTERY_UPDATE_TIME)
  {
    readBatteryLevel();
    lastBatteryUpdate = tick;
  }

  do_flash();

  if (tick - lastConnection > MAX_DISCONNECTION_TIME)
  {
#if SERIAL_DEBUG
    Serial.println("Disconnected too long, sleeping.");
#endif
    esp_deep_sleep_start();
  }
}
