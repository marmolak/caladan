#include <Arduino.h>

#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "MasterBoard/HAL/Communication.hpp"

// Workaround for newest c++14 version

void operator delete(void *ptr, size_t size)
{
  (void) size;
  free(ptr);
}

void operator delete[](void *ptr, size_t size)
{
    (void) size;
    free(ptr);
}

/* GLOBALS */

MasterBoardComm mb;

const int trigPin = 19;    // Trigger
const int echoPin = 18;    // Echo

const int pumpPin = 8;

static unsigned long start_measure_flow;
static bool pump_running = false;

const int max_pumpin_time = 2000; // 2 * 1000 ms
const byte max_pumpin_count = 30; // * 2 because we wait for 2 seconds while pumping

static byte pumpin;
static byte hits;

using condition_func_t = bool (*)();
condition_func_t sensors_conditions[20] = { NULL };

// reset_conditions don't forget to add condition to reset_conditions()
enum class conditions_e : int
{
  REMOTE      = 0,
  HUMIDITY    = 1,
  WATER_LEVEL = 2,
  WATER_FLOW  = 3,
  COND_END
};

void reset_conditions();

void wakeup()
{
    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(20));
}

void stop_pump()
{
  // stop pump and reset pumpin counter
  digitalWrite(pumpPin, HIGH);
  pump_running = false;
  pumpin = 0;
  hits = 0;
  //delay(max_pumpin_time);
}

float get_cm()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  const unsigned long duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  const float cm = (float) duration / 58.31f;     // Divide by 29.1 or multiply by 0.0343
  return cm;
}

float get_average_cm()
{
  float cm_avg = 0;

  byte p = 0;
  byte repeats = 0;
  while (p < 4)
  {
    ++repeats;

    if (repeats >= 100)
    {
      return -1;
    }

    // sensor sometimes returns gibrish...
    const float tmp_cm = get_cm();
    if (tmp_cm >= 50)
    {
      continue;
    }
    
 
    cm_avg += tmp_cm;
    ++p;
    delay(200);
  }
 
  cm_avg /= 4;

  return cm_avg;
}

bool should_stop()
{
  static bool disable_loop = true;
  // stupid but we need it for now
  const String s = mb.work();

  if (s == "STOP")
  {
    Serial.println("Stopping");
    disable_loop = true;
    reset_conditions();

    return disable_loop;  
  }

  if (s == "START")
  {
    Serial.println("Starting");
    reset_conditions();
    disable_loop = false;
    return disable_loop;
  }

  if (s == "FORCE")
  {
    // reset of conditions are handled by external handler which sends STOP command
    Serial.println("Force");
    reset_conditions();
    sensors_conditions[static_cast<int>(conditions_e::HUMIDITY)] = NULL;
    disable_loop = false;
    return disable_loop;
  }

  if (s == "AUTO")
  {
    Serial.println("Auto");
    disable_loop = false;
    reset_conditions();
    return disable_loop;
  }

  /*Serial.print("SlaveBoard: ");
  Serial.println(s);*/

  disable_loop = true;
  return disable_loop;
}

// water flow
// interrupt handler
static volatile unsigned int water_flow_pulses = 0; 
void water_check_pulses_intr()
{
  ++water_flow_pulses;
}

static int flow_fail_count = 0;
void reset_flow_check()
{
  flow_fail_count = 0;
}
// condition
bool check_water_flow()
{
  if (flow_fail_count >= 3)
  {
    Serial.println("Fail count too high.. flow not detected. Need manual reset.");
    return true;
  }
  if (!pump_running)
  {
    return false;
  }
 
  const unsigned long now = millis();
  const unsigned int ret = water_flow_pulses;

  //start_measure_flow = millis();
  Serial.println("Water flow sensor state: " + String(ret) + " " + String(now - start_measure_flow));

  // start measure after 3 seconds
  if (pump_running && (now - start_measure_flow) >= 4000)
  {  
    water_flow_pulses = 0;

    if (ret < 10) {
      ++flow_fail_count;
    }
    return !(ret >= 10);
  }

  // if we receiving pulses, just restart measure time
  return false;
}


bool check_humidity()
{
  // check for humidity level
  const int humidity_raw = analogRead(A8);
  const int humidity_percent = map(humidity_raw, 615, 310, 0, 100);
  Serial.println("Hum: " + String(humidity_percent) + " raw: " + String(humidity_raw));

  if (humidity_percent >= 20)
  {
    Serial.println("Humidity point reached!");
    //stop_pump();
    return true;
  }

  return false;
}

void start_pump()
{
  if (pump_running) {
    return;
  }

  Serial.println("Starting pump");
  start_measure_flow = millis();
  digitalWrite(pumpPin, LOW);
  hits = 0;
  pump_running = true;
}

bool check_water_level()
{
  // check water level
  const float cm_avg = get_average_cm();
  if (cm_avg == -1)
  {
    Serial.println("Measurement failed!");
    return true;
  }

  const auto to_print_cm = String(cm_avg) + " cm";
  Serial.println(to_print_cm);

  if (ceil(cm_avg) >= 11 || pumpin >= max_pumpin_count)
  {
    Serial.println("Reseting pump" + String(pumpin));
    return true;
  }
  pumpin += 1;

// just continue
  if (cm_avg < 10 && (hits % 4) == 0)
  {
    
    // start pump
    // start_pump();

    return false;
  }
  if (cm_avg < 10)
  {
    ++hits;
  } else {
    hits = 0;
  }

  return false;
}


void reset_conditions()
{
  sensors_conditions[static_cast<int>(conditions_e::REMOTE)] = &should_stop;
  sensors_conditions[static_cast<int>(conditions_e::HUMIDITY)] = &check_humidity;
  sensors_conditions[static_cast<int>(conditions_e::WATER_LEVEL)] = &check_water_level;
  sensors_conditions[static_cast<int>(conditions_e::WATER_FLOW)] = &check_water_flow;
  reset_flow_check();
}


void power_settings()
{
  power_spi_disable();
  
  power_timer1_disable();
  power_timer2_disable();
  power_timer3_disable();
  power_timer4_disable();
  power_timer5_disable();

  clock_prescale_set(clock_div_2);
}

void deep_sleep()
{
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  noInterrupts();
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(20), &wakeup, RISING);
  // interrupts allowed now, next instruction WILL be executed
  interrupts();           
  sleep_cpu(); 
}

void setup()
{
  // power_settings();

  // pin for extarnal wake up
  pinMode(20, INPUT);
  
  Serial.begin(115200);

  // Communicate with esp
  Serial3.begin(9600);

  mb.init();

  // humidity sensor
  pinMode(A8, INPUT);

  // relay for pump
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, HIGH);

  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // flow sensor
  pinMode(2, INPUT_PULLUP);

  cli();
  attachInterrupt(digitalPinToInterrupt(2), &water_check_pulses_intr, RISING);
  sei();

  reset_conditions();
  start_measure_flow = millis();
}


static byte doing_nothing = 0;
void loop()
{
  mb.update();

  if (doing_nothing > 5)
  {
    //Serial.print("Doing sleep");
    //Serial.flush();
    doing_nothing = 0;
    //deep_sleep();
  }
  
  int conds = false;
  for (int i = 0; i < static_cast<int>(conditions_e::COND_END); ++i)
  {
    if (sensors_conditions[i] == NULL)
    {
      continue;
    }

    conds = true;

    const bool should_stop = sensors_conditions[i]();
    if (should_stop)
    {
      stop_pump();
      water_flow_pulses = 0;
 
      delay(max_pumpin_time);
      ++doing_nothing;
      return;
    } 
  }

  if (!conds) {
    // don't start pump when there is no conditions
    return;
  }

  start_pump();
  delay(max_pumpin_time);
}