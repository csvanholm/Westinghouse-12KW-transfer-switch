#include "pico/stdlib.h"


//#undef USING_LED_RESISTORS     
#define USING_LED_RESISTORS

const uint GENERATOR_RUNNING    = 14;
const uint LINE_POWER_FAIL      = 16;
const uint START_STOP_RELAY     = 18;

// led port
const uint LED_PIN    = PICO_DEFAULT_LED_PIN;  // onboard led
const uint LED_BLUE   = 15;
const uint LED_RED    = 11;
const uint LED_YELLOW = 9;
const uint LED_WHITE  = 7;


class Generator
{
 public:
  Generator() { InitIoPins(); }

 private:
  bool m_GeneartorRunning=false;
  bool m_GenPowerRequested=false;
  uint m_CoolDownCounter = 0;
  uint m_StartCounter = 0;
  uint m_FailedStartRetryCounter = 0;
  uint m_TicksSinceStart = 0;
  uint m_GeneratorPowerStabel = 0;
  
  void PulseStartButton();
  void PulseStopButton();
  void AliveBlink();
  void InitIoPins();
  void ReadInputs();
  void Led(uint id,bool state);
  void BlinkFailure();
 
 public:
 void RunOneTick();
};


void Generator::PulseStartButton()
{
 Led(LED_WHITE,1);
 gpio_put(START_STOP_RELAY, 1);
 sleep_ms(1200);  // 1200 ms to start
 gpio_put(START_STOP_RELAY, 0);
 Led(LED_WHITE,0);
}

void Generator::PulseStopButton()
{
 Led(LED_WHITE,1);
 gpio_put(START_STOP_RELAY, 1);
 sleep_ms(2500);  // 2500 ms to stop  
 gpio_put(START_STOP_RELAY, 0);
 Led(LED_WHITE,0);
}


void Generator::Led(uint id,bool state)
{
 #ifdef USING_LED_RESISTORS
  gpio_put(id,state); 
 #else
 if (state) 
 {
   gpio_pull_up(id); 
 } else  
 {  
  gpio_pull_down(id);  
 }
 #endif
}


void Generator::AliveBlink()  
{
 gpio_put(LED_PIN, 1);
 sleep_ms(5);             // blink onboard LED for 5 ms to indicate the main loop is running 
 gpio_put(LED_PIN, 0);
 if (m_CoolDownCounter>0) // update status LEDS (if you installed them, they are not required)
 {
  Led(LED_BLUE,!gpio_get(LED_BLUE)); // make cool down led blink if we are in cool-down mode before stopping generator
 } else
 {
   Led(LED_BLUE,0);
 }
 if (m_GeneratorPowerStabel > 0)
 {
   Led(LED_YELLOW,!gpio_get(LED_YELLOW));
 } else
 {
  Led(LED_YELLOW,m_GeneartorRunning);
 }
 Led(LED_RED,m_GenPowerRequested);
}


void Generator::BlinkFailure()
{
 bool state = true;
 for (int a=0; a<10; a++)
 {
  Led(LED_BLUE,state);
  Led(LED_RED,state);
  Led(LED_YELLOW,state);
  Led(LED_WHITE,state);
  sleep_ms(40);
  state=!state;
 }
}



void Generator::InitIoPins()
{
  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  gpio_init(GENERATOR_RUNNING);
  gpio_set_dir(GENERATOR_RUNNING, GPIO_IN);
  gpio_pull_up(GENERATOR_RUNNING);

  gpio_init(LINE_POWER_FAIL);
  gpio_set_dir(LINE_POWER_FAIL, GPIO_IN);
  gpio_pull_up(LINE_POWER_FAIL);

  gpio_init(START_STOP_RELAY);
  gpio_set_dir(START_STOP_RELAY, GPIO_OUT);
  
  // CRITICAL!!! The indicator LEDs can be connected directly to the RP2040 ports without a resistor if we use the internal pullup resistor 
  // in the chip.
  // LED ports need to be set as inputs if we are using the internal pullup resistor to drive the led by switching between 
  // pull_up to turn it on and pull_down to turn it off
  // this will draw just enough current to turn the LED's on but technically too weak to drive green LED's.
  // if you want brighter LED's you need to use a 380 to 680 ohm resistor between the RP2040 port and the LED's and define USING_LED_RESISTORS
  // at the top of this file  
  #ifndef USING_LED_RESISTORS
  gpio_init(LED_BLUE);
  gpio_set_dir(LED_BLUE,GPIO_IN);
  gpio_pull_down(LED_BLUE);

  gpio_init(LED_RED);
  gpio_set_dir(LED_RED,GPIO_IN);
  gpio_pull_down(LED_RED);
  
  gpio_init(LED_YELLOW);
  gpio_set_dir(LED_YELLOW,GPIO_IN);
  gpio_pull_down(LED_YELLOW);

  gpio_init(LED_WHITE);
  gpio_set_dir(LED_WHITE,GPIO_IN);
  gpio_pull_down(LED_WHITE);
 
 #else
  gpio_init(LED_BLUE);
  gpio_set_dir(LED_BLUE,GPIO_OUT);

  gpio_init(LED_RED);
  gpio_set_dir(LED_RED,GPIO_OUT);
  
  gpio_init(LED_YELLOW);
  gpio_set_dir(LED_YELLOW,GPIO_OUT);
  
  gpio_init(LED_WHITE);
  gpio_set_dir(LED_WHITE,GPIO_OUT);
 
 #endif
}

void Generator::ReadInputs()
{
 // The starter cranking the engine might energize the low voltage coil so we dont want to assume the generator is running yet so just wait a few seconds.   
 // transition from no power to power state but assume a 5 second delay before we act on the change input to ensure engine is running
 if (!gpio_get(GENERATOR_RUNNING) && !m_GeneartorRunning) // if current state is different than last state
 {
  if (m_GeneratorPowerStabel > 6)
  {
    m_GeneartorRunning   = !gpio_get(GENERATOR_RUNNING);  // inputs are active low or inverted 
  } else
  {
   m_GeneratorPowerStabel++;
  }
 } else
 {
  m_GeneartorRunning   = !gpio_get(GENERATOR_RUNNING);    // active low
  m_GeneratorPowerStabel = 0;
 }
 m_GenPowerRequested  = !gpio_get(LINE_POWER_FAIL);
}


void Generator::RunOneTick()
{
 m_TicksSinceStart++;
 if (m_FailedStartRetryCounter >= 3)  // lockup the controler to prevent repeated restarts attempts (on out of gas) to avoid draining the battery     
 {
  BlinkFailure();
  return;
 }
 ReadInputs();                 // get the current state 
 AliveBlink();                 // blink indicators
 if (m_GeneartorRunning)       // if generator is running and power is requested just keep running and reset the cool down counter
 {
  m_StartCounter=0;
  m_FailedStartRetryCounter=0; 
  if (m_GenPowerRequested)
  {
   m_CoolDownCounter = 0;
  } else
  {
   m_CoolDownCounter++;         // if generator is running but power is no longer requested allow the generator to cool down for 40 seconds before stopping it. 
   if (m_CoolDownCounter >= 80) // this will also take care of power flickering in the first seconds after utility power returns
   {
    PulseStopButton();          // signale the generator to stop
    m_CoolDownCounter = 0;
   }
  }
 } else                         // Generator not running.  
 {
  m_CoolDownCounter = 0;
  if (m_GenPowerRequested && m_TicksSinceStart > 15 && m_GeneratorPowerStabel==0) // we have to wait for the generator controller to become active
  {
    if (m_StartCounter==0)       // Only send one start Pulse to generator and wait for the generator to start
    {
      PulseStartButton();
    }
    m_StartCounter++;
    if (m_StartCounter > 60 && m_FailedStartRetryCounter<3)    // if the Generator failed to start we will try again after 30 seconds after the 3rd attempt we goto into fail state (blink all the leds)
    {
      m_StartCounter=0;         
      m_FailedStartRetryCounter++;
    }
  } else
  {
    m_StartCounter=0;       
    m_FailedStartRetryCounter=0; 
  }
 }
}


int main() 
{
 Generator gen;
 while (true) 
 {
   gen.RunOneTick();
   sleep_ms(500);
 };
 return 0; 
}

