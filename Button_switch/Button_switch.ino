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
}

void loop() {
  // อ่านค่าสถานะของ push switch (ว่ากด หรือ ไม่กด)
  currentButtonState = digitalRead(buttonPin);

  // ตรวจสอบกรณีที่สถานะปุ่มกดไม่เหมือนครั้งที่แล้วและครั้งนี้สถานะเป็นปล่อย
  // ซึ่งก็คือปุ่มถูกกดค้างไว้และได้ถูกปล่อยออกนั่นเอง
  if ((currentButtonState != previousButtonState) && previousButtonState == HIGH) {
    // สั่งให้ค่า led เป็นตรงกันข้าม (ถ้าเปิดให้ปิด ถ้าปิดให้เปิด)
    isLedOn = !isLedOn;
    // ส่งค่าไปยังขา 13 เพื่อให้เปิดหรือปิด led ตามสถานะปัจจุบัน
    digitalWrite(ledPin, isLedOn);
    // หน่วงเวลา 0.1 วินาทีหน่วงค่าความคลาดเคลื่อนตอนเริ่มต้นกด switch

    stage = stage + 1;
    if (stage == 3) {
      stage = 0;
    }
    delay(100);
  }

  // จำค่าการกดปุ่ม ณ ปัจจุบันไว้ เพื่อนำไปใช้เปรียบเทียบครั้งต่อไป
  previousButtonState = currentButtonState;
  
}
