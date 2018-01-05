#include <EEPROM.h>

#define s1 A1         //start point
#define s2 A2         //wire
#define s3 A3         //end point

#define L1 13          //start point indicator LED
#define L2 12          //wire touched indicator LED
#define L3 11          //end point indicator LED

#define buzzer 3          //buzzer pin

//define pins used for seven segment display
#define seven_segment_enable 4
#define seven_segment_clock 7
#define seven_segment_data 8

byte segment_for_number[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0X80, 0X90};         //setting segment configuration for number 0 to 9
byte mask_for_dot = 0x7F;
byte segment_select[] = {0xF1, 0xF2, 0xF4, 0xF8};         //setting value for segment selection

unsigned long global_best_time = 0;

int turn = 1;       // to store how many time player has played
unsigned long start_time = 0, completion_time[3] = { -1, -1, -1};

void setup() {
  Serial.begin(9600);

  //setinng switch pins
  pinMode(s1, INPUT_PULLUP);
  pinMode(s2, INPUT_PULLUP);
  pinMode(s3, INPUT_PULLUP);

  //setting LEDs' pins
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  digitalWrite(L1, HIGH);
  digitalWrite(L2, HIGH);
  digitalWrite(L3, HIGH);

  //setting buzzer pin
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);

  //setting pin for seven segment display
  pinMode(seven_segment_enable, OUTPUT);
  digitalWrite(seven_segment_enable, HIGH);
  pinMode(seven_segment_clock, OUTPUT);
  pinMode(seven_segment_data, OUTPUT);

  //getting global score from EEPROM
  global_best_time = get_global_best_time();
  if (global_best_time == 0)
    global_best_time = -1;
  Serial.print("Global best time is ");
  Serial.println(global_best_time);

  clear_seven_segment_display();          //clearing seven segment in order ro get rid of some random value
}

void loop() {
  while (turn <= 3)
  {
    clear_seven_segment_display();          //clearing seven segment in order ro get rid of some random value
    while (digitalRead(s1));          //wait till start button is pressed
    while (!digitalRead(s1));         //wait till botton released.
    delay(50);          //avoiding debounce
    Serial.print("->turn no :- ");
    Serial.println(turn);
    display_turn(turn);
    show_countdown();

    start_time = millis();

    while (digitalRead(s2) && digitalRead(s3))
    {
      display_time(millis() - start_time);
    }
    if (!digitalRead(s2))         //wire touched
    {
      while (!digitalRead(s2));         //wait till botton is released.
      delay(50);          //avoiding debounce
      Serial.println("Wire Touched");

      sound_buzzer(false);          //sounding buzzer for wire touched
    }
    else          //end point reached
    {
      completion_time[turn - 1] = millis() - start_time;          //saving completion time
      while (!digitalRead(s3));         //wait till botton is released.
      delay(50);          //avoiding debounce
      Serial.print("End point reached at time ");
      Serial.println(completion_time[turn - 1]);

      sound_buzzer(true);         //sounding buzzer for endpoint reached
    }
    Serial.print("Turn ");
    Serial.print(turn);
    Serial.println(" is completed");
    turn++;
  }
  Serial.println("3 turn finished");

  unsigned long best_time = min(completion_time[0], min(completion_time[1], completion_time[2]));         //getting best completion time from all 3 turns
  Serial.print("This game's best time is ");
  Serial.println(best_time);
  start_time = millis();
  while (millis() - start_time < 2000)          //displaing this game's best turn on 7 segment displays
  {
    display_time(best_time);
  }

  best_time = min(best_time, global_best_time);

  //writting best time on EEPROM
  set_global_best_time(best_time);

  Serial.print("New global best time is ");
  Serial.println(best_time);
  start_time = millis();
  while (1)
  {
    display_time(best_time);
  }
}

void write_to_seven_segment(byte segment, byte value)         //write data to seven segment display
{
  digitalWrite(seven_segment_enable, LOW);
  shiftOut(seven_segment_data, seven_segment_clock, MSBFIRST, value);
  shiftOut(seven_segment_data, seven_segment_clock, MSBFIRST, segment_select[segment]);
  digitalWrite(seven_segment_enable, HIGH);
}

void display_time(unsigned long t)          //prepare number to show on seven segment displays
{
  if (t < 100000)
  {
    write_to_seven_segment(0, segment_for_number[(t / 10000) % 10]);
    write_to_seven_segment(1, segment_for_number[(t / 1000) % 10] & mask_for_dot);
    write_to_seven_segment(2, segment_for_number[(t / 100) % 10]);
    write_to_seven_segment(3, segment_for_number[(t / 10) % 10]);
  }
  else
  {
    write_to_seven_segment(0, segment_for_number[(t / 100000) % 10]);
    write_to_seven_segment(1, segment_for_number[(t / 10000) % 10]);
    write_to_seven_segment(2, segment_for_number[(t / 1000) % 10] & mask_for_dot);
    write_to_seven_segment(3, segment_for_number[(t / 100) % 10]);
  }
}

void sound_buzzer(bool won)
{
  if (won)          //for winning signal, buzzer will sound 3 times for short time
  {
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH);
  }
  else          //for losing signal, buzzer will sound 1 time for long time
  {
    digitalWrite(buzzer, LOW);
    delay(2000);
    digitalWrite(buzzer, HIGH);
  }
}

void display_turn(int n)
{
  switch (n)
  {
    case 1:
      digitalWrite(L1, LOW);
      break;

    case 2:
      digitalWrite(L2, LOW);
      break;

    case 3:
      digitalWrite(L3, LOW);
      break;
  }
}

void show_countdown()
{
  write_to_seven_segment(3, segment_for_number[3]);
  delay(1000);
  write_to_seven_segment(3, segment_for_number[2]);
  delay(1000);
  write_to_seven_segment(3, segment_for_number[1]);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(300);
  digitalWrite(buzzer, HIGH);
  Serial.println("game start");
}

unsigned long get_global_best_time()
{
  unsigned long t1, t2, t3, t4;
  t1 = EEPROM.read(0);
  t2 = EEPROM.read(1);
  t3 = EEPROM.read(2);
  t4 = EEPROM.read(3);
  return ((t1 << 24) | (t2 << 16) | (t3 << 8) | (t4));
}

void set_global_best_time(unsigned long t)
{
  EEPROM.write(0, (t >> 24));
  EEPROM.write(1, (t >> 16));
  EEPROM.write(2, (t >> 8));
  EEPROM.write(3, t);
}

void clear_seven_segment_display()
{
  write_to_seven_segment(3, 0xFF);
}

