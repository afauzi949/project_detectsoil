#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// WiFi credentials
const char* ssid = "xyz";
const char* password = "muntilan";

// Pin definitions
#define DMSpin 13       // pin output untuk DMS
#define indikator 2     // pin output untuk indikator pembacaan sensor
#define adcPin 32       // pin input sensor pH tanah
#define soilMoisturePin 35 // pin untuk sensor kelembaban tanah
#define raindropPin 34  // pin untuk sensor raindrop
#define DHTPIN 27       // pin data DHT22 terhubung ke GPIO 27

// Sensor types
#define DHTTYPE DHT22   // Tipe sensor DHT 22

// Sensor objects
DHT dht(DHTPIN, DHTTYPE);

// Variables for sensors
int ADC;
float lastReading = -1.0;  // Inisialisasi dengan nilai yang tidak mungkin untuk pH
float pH;

// Nilai minimum dan maksimum dari sensor kelembaban tanah
const int dryValue = 650;    // Nilai saat tanah sangat kering
const int wetValue = 230;    // Nilai saat tanah sangat basah

// Variables for random values
int n, p, k;

void setup() {
  // Memulai serial communication untuk debugging
  Serial.begin(115200);

  // Memulai koneksi WiFi
  WiFi.begin(ssid, password);
  Serial.println("Menghubungkan ke WiFi...");

  // Tunggu hingga terhubung ke WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan...");
  }

  // Jika sudah terhubung, cetak alamat IP
  Serial.println("Terhubung ke WiFi!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());

  // Set pin mode untuk sensor pH tanah dan DMS
  analogReadResolution(10);      // setting resolusi pembacaan ADC menjadi 10 bit
  pinMode(DMSpin, OUTPUT);
  pinMode(indikator, OUTPUT);
  digitalWrite(DMSpin, HIGH);    // non-aktifkan DMS

  // Inisialisasi DHT22
  dht.begin();

  // Inisialisasi nilai acak
  randomSeed(analogRead(0));
}

void loop() {
  // Membaca nilai dari sensor raindrop
  int raindropValue = analogRead(raindropPin);
  int rainfallIntensity = map(raindropValue, 1050, 375, 18, 299);
  rainfallIntensity = constrain(rainfallIntensity, 18, 299);

  // Membaca nilai dari sensor pH tanah
  digitalWrite(DMSpin, LOW);      // aktifkan DMS
  digitalWrite(indikator, HIGH);  // led indikator built-in ESP32 menyala
  delay(10000);               // wait DMS capture data
  ADC = analogRead(adcPin);

  if (ADC != 0) {  // Tambahkan pengecekan untuk nilai ADC
    pH = (-0.0333 * ADC) + 9.8340;  // ini adalah rumus regresi linier yang wajib anda ganti!
    if (pH != lastReading) {
      lastReading = pH;
    }
  }

  digitalWrite(DMSpin, HIGH);
  digitalWrite(indikator, LOW);
  delay(3000);                 // wait for DMS ready

  // Membaca nilai dari sensor DHT22
  float t = dht.readTemperature(); 
  float h = dht.readHumidity(); // Membaca kelembaban dari DHT22

  if (isnan(t) || isnan(h)) { 
    Serial.println("Failed to read from DHT sensor!");
  }

  // Mengacak nilai n, p, dan k
  n = random(0, 141);
  p = random(5, 146);
  k = random(5, 206);

  // Membaca nilai dari sensor kelembapan tanah
  int soilMoistureValue = analogRead(soilMoisturePin);
  Serial.print("Nilai mentah kelembapan tanah: ");
  Serial.println(soilMoistureValue);
  int soilMoisturePercent = map(soilMoistureValue, dryValue, wetValue, 0, 100);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
  Serial.print("Kelembapan Tanah (%): ");
  Serial.println(soilMoisturePercent);

  // Menampilkan semua data sensor
  Serial.println("Pembacaan Sensor:");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C"); 

  Serial.print("Kelembaban Tanah: ");
  Serial.println(soilMoisturePercent);

  Serial.print("Nilai mentah sensor hujan: ");
  Serial.println(raindropValue);

  Serial.print("Intensitas Hujan: ");
  Serial.print(rainfallIntensity);
  Serial.println(" mm");

  Serial.print("ADC=");
  Serial.print(ADC);
  Serial.print(" pH=");
  Serial.println(lastReading, 1);

  Serial.print("n: ");
  Serial.println(n);
  Serial.print("p: ");
  Serial.println(p);
  Serial.print("k: ");
  Serial.println(k);

  Serial.print("Humidity: ");
  Serial.println(h);

  // Mengirim data ke server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://192.168.85.49:5000/sensor-data");
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{";
    jsonData += "\"n\":" + String(n) + ",";
    jsonData += "\"p\":" + String(p) + ",";
    jsonData += "\"k\":" + String(k) +",";
    jsonData += "\"temperature\":" + String(t) + ",";
    jsonData += "\"humidity\":" + String(h) + ",";
    jsonData += "\"ph\":" + String(lastReading, 1);
    jsonData += "\"rainfall\":" + String(rainfallIntensity) + ",";
    jsonData += "}";

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Error in WiFi connection");
  }

  Serial.println("------------------------");
  // Menunggu selama 10 detik sebelum pembacaan berikutnya
  delay(5000);
}
