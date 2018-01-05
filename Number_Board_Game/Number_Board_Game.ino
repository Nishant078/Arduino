#define button A1         //define pin for button

//define pins used for seven segment display
#define seven_segment_enable 4
#define seven_segment_clock 7
#define seven_segment_data 8

#define buzzer 3

byte segment_for_number[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0X80, 0X90};          //setting segment configuration for number 0 to 9
byte mask_for_dot = 0x7F;
byte segment_select[] = {0xF1, 0xF2, 0xF4, 0xF8};       //setting value for segment selection

int led[4] = {13, 12, 11, 10};          //defining LED pins

unsigned int number_of_time_button_pressed = 0;        //to keep count of how many time number pressed

unsigned long winning_time = 0;         //to store winning time

void setup() {
  //setting LED pins (pin no. - 10,11,12,13)
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  //setting pin for seven segment display
  pinMode(seven_segment_enable, OUTPUT);
  digitalWrite(seven_segment_enable, HIGH);         //because shift registers for seven segment display in active LOW ic
  pinMode(seven_segment_clock, OUTPUT);
  pinMode(seven_segment_data, OUTPUT);

  //setting pin for buzzer
  pinMode(buzzer,OUTPUT);
  digitalWrite(buzzer,HIGH);

  //switch off all seven segment displayes
  write_to_seven_segment(0, 0xFF);

  //setting button with pullup
  pinMode(button, INPUT_PULLUP);

  //providing open analog pin as a seed for random number generater in order to get random number in each run
  randomSeed(analogRead(A5));

  Serial.begin(9600);
}

void loop()
{
  unsigned int random_number = random(1, 16);       //generate random number from 1 to 15
  Serial.print("random number is");
  Serial.println(random_number);
  display_binary(random_number);          //displaing random number on LEDs

  unsigned int start_time = 0;          //to keep rafrence time for 2 second waiting time
  bool button_pressed = false;          //to check whether button pressed or not

  while (1)     //button pressing starts
  {
    button_pressed = false;
    start_time = millis();

    while (millis() < (start_time + 2000))
    {
      if (!digitalRead(button))         //buttom pressed within time
      {
        while (!digitalRead(button));         //handeling debounce
        delay(50);          //handeling debounce
        button_pressed = true;
        break;
      }
    }
    if (button_pressed) {
      number_of_time_button_pressed++;          //incrementing counter for number of time buttom pressed
    }
    else
    {
      break;          //buttom not pressed
    }
  }         //finished button pressing

  //checking whether player has pressed button right number of thimes
  if (number_of_time_button_pressed != random_number)
  {
    lose();         //player lose
  }
  else
  {
    winning_time = millis();
    won();          //player won
  }
}

void display_binary(unsigned int n)         //To display binary on LEDs
{
  digitalWrite(led[0], !(n & 0x01));
  digitalWrite(led[1], !(n >> 1 & 0x01));
  digitalWrite(led[2], !(n >> 2 & 0x01));
  digitalWrite(led[3], !(n >> 3 & 0x01));
}

void lose()         //player lose routine
{
  Serial.print(number_of_time_button_pressed);
  Serial.println(" time button pressed");
  Serial.println("LOSE");

  //sounding buzzer for 3 second
  digitalWrite(buzzer,LOW);
  delay(2000);
  digitalWrite(buzzer,HIGH);
  
  while (1)
    display_number_of_time_button_pressed();
}

void won()          //player won routine
{
  Serial.print(number_of_time_button_pressed);
  Serial.println(" time button pressed");
  Serial.println("WON");

  //sounding buzzer for 3 second
  digitalWrite(buzzer,LOW);
  delay(3000);
  digitalWrite(buzzer,HIGH);
  
  display_number_of_time_button_pressed();
  while (1)
    display_winnin_time();
}

void write_to_seven_segment(byte segment, byte value)         //write data to seven segment display
{
  digitalWrite(seven_segment_enable, LOW);
  shiftOut(seven_segment_data, seven_segment_clock, MSBFIRST, value);
  shiftOut(seven_segment_data, seven_segment_clock, MSBFIRST, segment_select[segment]);
  digitalWrite(seven_segment_enable, HIGH);
}

void display_number_of_time_button_pressed()          //to display total number of time button pressed
{
  int start_time = millis();
  while (millis() < (start_time + 5000))
  {
    if (number_of_time_button_pressed / 10 != 0)        //to display first digit if number of time button pressed in more than or equal to 10
    {
      write_to_seven_segment(2, segment_for_number[number_of_time_button_pressed / 10]);
    }
    write_to_seven_segment(3, segment_for_number[number_of_time_button_pressed % 10]);        //to display last digit of number of time button pressed
  }
}

void display_winnin_time()        //to display winning time in second with precision upto 2 decimal point if player wins
{
  int start_time = millis();
  while (millis() < (start_time + 5000))
  {
    write_to_seven_segment(0, segment_for_number[(winning_time / 10000) % 10]);
    write_to_seven_segment(1, segment_for_number[(winning_time / 1000) % 10] & mask_for_dot);
    write_to_seven_segment(2, segment_for_number[(winning_time / 100) % 10]);
    write_to_seven_segment(3, segment_for_number[(winning_time / 10) % 10]);
  }
}
