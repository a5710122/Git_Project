#include <Scheduler.h>
#include <Task.h>

#include <FlowMeter.h>
#include <Ultrasonic.h>

// ------------------------------ Call library ------------------------------- // 
FlowMeter Meter = FlowMeter(2);
Ultrasonic ultrasonic(0, 2);
// -------------------------------------------------------------------------- //

const unsigned long period = 1000;

void setup() {
  Serial.begin(115200); 

  // enable a call to the 'interrupt service handler' (ISR) on every rising edge at the interrupt pin
  // do this setup step for every ISR you have defined, depending on how many interrupts you use
  attachInterrupt(2, MeterISR, RISING);
  // sometimes initializing the gear generates some pulses that we should ignore
  Meter.reset();

}

void loop() {

  // wait between output updates
  delay(period);
  // process the (possibly) counted ticks
  Meter.tick(period);
  // output some measurement result
  Serial.println("Currently Flow: " + String(Meter.getCurrentFlowrate()) + " l/min, " + "Total Flow Volume: " + String(Meter.getTotalVolume()));

  ultrasonic.calUltra();

}

// -------------------------------- Function Flow meter --------------------------------- //
// define an 'interrupt service handler' (ISR) for every interrupt pin you use //
void MeterISR() {
  // let our flow meter count the pulses
  Meter.count();
}
// -------------------------------------------------------------------------------------- //
