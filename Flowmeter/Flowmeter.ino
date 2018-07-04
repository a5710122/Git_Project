#define HALLSERSON  D5  //Flowmeter

volatile int NbTopsFan; //measuring the rising edges of the signal
volatile int Calc; // Flow meter

void setup() {
  Serial.begin(115200); //This is the setup function where the serial port is initialised,
  pinMode(HALLSERSON, INPUT); //initializes digital pin 2 as an input
  attachInterrupt(D5, rpm, RISING); //and the interrupt is attached
}

void loop() {
  flowMeter();

}

void rpm() {   //This is the function that the interupt calls
  NbTopsFan++; //This function measures the rising and falling edge of the hall effect sensors signal
}

void flowMeter() {
  NbTopsFan = 0;     //Set NbTops to 0 ready for calculations
  sei();           //Enables interrupts
  delay (1000);     //Wait 1 second
  cli();           //Disable interrupts
  Calc = (NbTopsFan  / 7.5); //(Pulse frequency x 60) / 7.5Q, = flow rate in L/hour
  Serial.print (Calc, DEC); //Prints the number calculated above
  Serial.print (" L/hour\r\n"); //Prints "L/hour" and returns a new line
}
