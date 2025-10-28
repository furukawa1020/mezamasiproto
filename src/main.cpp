#include <Arduino.h>

// シャープ測定モジュール（GP2Y0A21YK）の設定
const int SHARP_PIN = 1;  // GPIO1にセンサーを接続
const int LED_PIN = 8;    // ESP32-C3 SuperMiniの内蔵LED（GPIO8）

// 関数の前方宣言
void controlLEDByDistance(float distance);

void setup() {
  // シリアル通信を開始
  Serial.begin(115200);
  
  // USB CDC接続を待つ（最大3秒）
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime < 3000)) {
    delay(10);
  }
  delay(500);
  
  // アナログピンの設定
  pinMode(SHARP_PIN, INPUT);
  
  // LEDピンの設定
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("GP2Y0A21YK シャープ測定モジュール + LED制御 テスト開始");
  Serial.println("GPIO1に接続されたセンサーの値を読み取り、LEDで距離を表示します");
  
  // 起動時のLEDテスト
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  delay(1000);
}

void loop() {
  // アナログ値を読み取り
  int sensorValue = analogRead(SHARP_PIN);
  
  // 電圧に変換 (ESP32-C3は3.3V、12bit ADC)
  float voltage = sensorValue * (3.3 / 4095.0);
  
  // GP2Y0A21YKの特性式を使用して距離を計算
  // 距離(cm) = 27.728 * (voltage ^ -1.2045)
  float distance = 27.728 * pow(voltage, -1.2045);
  
  // 有効範囲チェック (GP2Y0A21YKは10-80cm)
  if (distance > 80) {
    distance = 80;
  } else if (distance < 10) {
    distance = 10;
  }
  
  // 距離に応じたLED制御
  controlLEDByDistance(distance);
  
  // シリアルモニタに出力
  Serial.print("Raw ADC: ");
  Serial.print(sensorValue);
  Serial.print(", Voltage: ");
  Serial.print(voltage, 3);
  Serial.print("V, Distance: ");
  Serial.print(distance, 1);
  Serial.print(" cm - LED: ");
  
  // LED状態の表示
  if (distance <= 15) {
    Serial.println("高速点滅（危険）");
  } else if (distance <= 30) {
    Serial.println("中速点滅（注意）");
  } else if (distance <= 50) {
    Serial.println("低速点滅（警告）");
  } else {
    Serial.println("常時点灯（安全）");
  }
  
  // 100ms待機（LEDの点滅制御のため短縮）
  delay(100);
}

// 距離に応じてLEDを制御する関数
void controlLEDByDistance(float distance) {
  static unsigned long lastBlinkTime = 0;
  static bool ledState = false;
  unsigned long currentTime = millis();
  
  if (distance <= 15) {
    // 15cm以下：高速点滅（100ms間隔）- 危険
    if (currentTime - lastBlinkTime >= 100) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlinkTime = currentTime;
    }
  } else if (distance <= 30) {
    // 15-30cm：中速点滅（300ms間隔）- 注意
    if (currentTime - lastBlinkTime >= 300) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlinkTime = currentTime;
    }
  } else if (distance <= 50) {
    // 30-50cm：低速点滅（800ms間隔）- 警告
    if (currentTime - lastBlinkTime >= 800) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlinkTime = currentTime;
    }
  } else {
    // 50cm以上：常時点灯 - 安全
    digitalWrite(LED_PIN, HIGH);
  }
}