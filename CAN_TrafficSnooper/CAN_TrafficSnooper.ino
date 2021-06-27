// Arduino ATmegaxxM1 - Displays all traffic found on canbus port
// By Thibaut Viard/Wilfredo Molina/Collin Kidder 2013-2014
//  Modified by Al Thomason for ATmegaxxM1 avr_CAN demo 2016

// Required libraries
#include <avr_can.h>
#include "IntList.h"

IntList<uint32_t> filter;
IntList<uint32_t> solo;

class SpeedEntry
{
public:
  SpeedEntry()
  {
    mSpeed = -1;
    mName = nullptr;
  }
  SpeedEntry(uint8_t speed, const char* name)
  {
    mSpeed = speed;
    mName = name;
  }

  uint8_t speed() const
  {
    return mSpeed;
  }

  const char* name() const
  {
    return mName;
  }

private:
  uint8_t mSpeed;
  const char* mName;
};

SpeedEntry SPEED_ARRAY[] = {
    SpeedEntry(CAN_BPS_100K, "100k"),
    SpeedEntry(CAN_BPS_125K, "125k"),
    SpeedEntry(CAN_BPS_200K, "200k"),
    SpeedEntry(CAN_BPS_250K, "250k"),
    SpeedEntry(CAN_BPS_500K, "500k"),
    SpeedEntry(CAN_BPS_1000K, "1M"),
    SpeedEntry() };

void setSpeed(const SpeedEntry& speedEntry)
{
  if (speedEntry.name() == nullptr) {
    return;
  }
  Can0.set_baudrate(speedEntry.speed());
  Serial.print("Set CAN baud rate to ");
  Serial.println(speedEntry.name());
}

void setSpeed(const char c)
{
  int asIndex = c - '1';
  if (asIndex < 0) {
    Serial.println("Speed not supported");
    return;
  }
  for (int i = 0; i <= asIndex; ++i) {
    if (SPEED_ARRAY[i].name() == nullptr) {
      Serial.println("Speed not supported");
      return;
    }
  }
  setSpeed(SPEED_ARRAY[asIndex]);
}

void setup()
{
  digitalWrite(A3, 0);
  pinMode(A3, OUTPUT);

  Serial.begin(115200);
  Serial.println("Ready");

  // Initialize CAN0 and CAN1, Set the proper baud rates here
  Can0.begin(CAN_BPS_125K);

  //This sets each mailbox to have an open filter that will accept extended
  //or standard frames
  Can0.setNumTXBoxes(0); // Use all the mailboxes for receiving.

  int filter;

  for (filter = 0; filter < 4; filter++) { //Set up 4 of the boxes for extended
    Can0.setRXFilter(filter, 0, 0, true);
  }

  while (Can0.setRXFilter(0, 0, false) > 0)
    ; // Set up the remaining MObs for standard messages.
}

String timeStamp(unsigned long time) {
  uint8_t hours = time / 1000 / 60 / 60;
  uint8_t minutes = (time / 1000 / 60) % 60;
  uint8_t seconds = (time / 1000) % 60;
  uint16_t mSeconds = time % 1000;

  char buffer[40];
  sprintf(buffer, "%d:%02d:%02d.%03d", hours, minutes, seconds, mSeconds);
  return String(buffer);
}

void printFrame(CAN_FRAME& frame)
{
  uint32_t id = frame.id;
  if (solo.length() > 0 && !solo.contains(id)) {
    return;
  }
  if (filter.contains(id)) {
    return;
  }

  Serial.print(timeStamp(millis()));
  Serial.print("  ID: 0x");
  Serial.print(frame.id, HEX);
  // Serial.print(" Rtr: ");
  // Serial.print(frame.rtr ? 1 : 0);
  Serial.print(" Len: ");
  Serial.print(frame.length);
  Serial.print(" Data: 0x");
  for (int count = 0; count < frame.length; count++) {
    char buffer[4];
    sprintf(buffer,"%02X", frame.data.bytes[count]);
    Serial.print(buffer);
    Serial.print(" ");
  }
  Serial.print("\r\n");
}

void showHelp()
{
  for (int i = 0; i < 10; ++i) {
    if (SPEED_ARRAY[i].name() == nullptr) {
      break;
    }
    Serial.print(char('1' + i));
    Serial.print(": Set CAN baudrate to ");
    Serial.println(SPEED_ARRAY[i].name());
  }
  Serial.println("C: Clear all filters and solos");
  Serial.println("f: Filter the given hex value");
  Serial.println("F: Clear the filter");
  Serial.println("s: Solo the given value");
  Serial.println("S: Clear the solo list");
  Serial.println("l: Log a manual message");
  Serial.println("?: Show this incredibly detailed help");
}

String readString() {
  String toReturn;
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == ' ' || c == '\n' || c == '\r') {
        return toReturn;
      }
      toReturn += c;
      Serial.print(c);
    }
  }
  Serial.println();
  return toReturn;
}

void readAndSet(const char* name, IntList<uint32_t>& list) {
  Serial.print("Enter HEX ID to ");
  Serial.print(name);
  Serial.print(": ");
  String value = readString();
  Serial.println();
  if (value.length() == 0) {
    // do nothing
  } else {
    uint32_t valueInt = strtol(value.c_str(), nullptr, 16);
    list.addValue(valueInt);
    Serial.print(timeStamp(millis()));
    Serial.print(" Adding to ");
    Serial.print(name);
    Serial.print(" 0x");
    Serial.print(valueInt, 16);
    Serial.print(" ");
    Serial.print(valueInt);
    Serial.println();
  }
}

void loop() {
  CAN_FRAME incoming;

  if (Serial.available() > 0) {
    uint8_t c = Serial.read();
    if (c == '?') {
      showHelp();
    } else if (c >= '0' && c <= '9') {
      setSpeed(c);
    } else if (c == 'C') {
      solo.clear();
      filter.clear();
      Serial.println("Cleared solos and filters");
    } else if (c == 'F') {
      filter.clear();
      Serial.println("Cleared filter");
    } else if (c == 'S') {
      solo.clear();
      Serial.println("Cleared solos");
    } else if (c == 's') {
      readAndSet("SOLO", solo);
    } else if (c == 'f') {
      readAndSet("FILTER", filter);
    } else if (c == 'l') {
      Serial.print("Log> ");
      String toLog(readString());
      Serial.println();
      Serial.print(timeStamp(millis()));
      Serial.print(" LOG ");
      Serial.println(toLog);
    } else {
      Serial.print("Unknown command: ");
      Serial.println((char)c);
    }
  }

  if (Can0.rx_avail()) {
    if (Can0.read(incoming))
      printFrame(incoming);
    else
      Serial.print(" -- FAILED");
  }
}
