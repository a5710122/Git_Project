const int buttonPin = 9 ;     // ขา pin ที่เราจะใช้อ่านค่าการกด push button
const int ledPin =  16;      // ขา pin ที่เราใช้ส่งออกไฟไปยัง LED

int currentButtonState = LOW; // ค่าสถานะปัจจุบนของปุ่ม
int previousButtonState = LOW;        // ค่าสถานะของปุ่มกดครั้งที่แล้ว
bool isLedOn = false;                 // ค่าสถานะของ led เริ่มต้นเป้นหลอดปิด

int stage;

void setup() {
  Serial.begin(115200);
  // บอกระบบว่าเราจะใช้ขา 13 เป้นตัวส่งค่าออก
  pinMode(ledPin, OUTPUT);
  // บอกระบบว่า เราจะใช้ขา 8 เป็นตัวรับค่าเข้ามา
  pinMode(buttonPin, INPUT);
  attachInterrupt(9, change_stage, RISING); //interrupt
}

void loop() {
   Serial.println(stage);
}

void change_stage() {
  currentButtonState = digitalRead(buttonPin);
  
  if ((currentButtonState != previousButtonState) && previousButtonState == HIGH) {
    
    stage ++;
//    if (stage == 3) {
//      stage = 0;
//    }
    delay(100);
  }

  // จำค่าการกดปุ่ม ณ ปัจจุบันไว้ เพื่อนำไปใช้เปรียบเทียบครั้งต่อไป
  previousButtonState = currentButtonState;
  
}
