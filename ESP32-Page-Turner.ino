#define USE_NIMBLE
#include <BleKeyboard.h>

/**
 * Bluetooth Page Turner
 *
 * (c) Stuart Coyle 2023
 */
#define SERIAL_DEBUG 1

// Bluetooth device name. Anything longer than
// 16 characters will be truncated.
#define DEVICE_NAME "Page Turner"

// GPIO Pin assignments
#define NEXT_PAGE 12
#define PREVIOUS_PAGE 14
#define BATTERY_LEVEL 13
#define BLUETOOTH_LED 5

// Keystrokes to send.
#define NEXT_PAGE_KEY KEY_RIGHT_ARROW
#define PREVIOUS_PAGE_KEY KEY_LEFT_ARROW

// Time the device will remain disconnected before going
// into deep sleep.
#define MAX_DISCONNECTION_TIME 120000

/*
 *  Key debouncing variables.
 */
#define DEBOUNCE_TIME 20
#define REPEAT_TIME 200

BleKeyboard bleKeyboard(DEVICE_NAME, "Stuart Coyle", 100);
int lastConnection = 0;
bool lastConnectionState;
int debounceMillis = 0;
int repeatMillis = 0;

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

// Read the button state with debouncing.
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
    }
  }

  digitalWrite(BLUETOOTH_LED, !lastConnectionState);

  if (tick - lastConnection > MAX_DISCONNECTION_TIME)
  {
#if SERIAL_DEBUG
    Serial.println("Disconnected too long, sleeping.");
#endif
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_1, 0);
    esp_deep_sleep_start();
  }
}
