
Smart Greenhouse Monitoring System (ESP32 + Supabase)

Loyiha haqida

Ushbu loyiha ESP32 mikrokontrolleri yordamida issiqxona yoki o‘simliklarni avtomatik monitoring va boshqarish uchun yaratilgan.

Qurilma quyidagi sensorlardan ma'lumotlarni yig‘adi:

- DHT11 (harorat va havo namligi)
- Tuproq namligi sensori
- Yorug‘lik sensori
- Gaz sensori

Olingan ma'lumotlar Supabase ma'lumotlar bazasiga yuboriladi va veb-ilova orqali kuzatib boriladi.

---

Asosiy imkoniyatlar

Monitoring

- Haroratni o‘lchash
- Havo namligini o‘lchash
- Tuproq namligini o‘lchash
- Yorug‘lik darajasini aniqlash
- Gaz konsentratsiyasini kuzatish

Avtomatik boshqaruv

Tizim quyidagi qurilmalarni avtomatik boshqaradi:

- Ventilyator (Cooler/Fan)
- Suv nasosi (Pump)

Rejimlar

Automatic Mode

Tizim Supabase bazasidan olingan chegaraviy qiymatlar asosida avtomatik ishlaydi.

Ventilyator yoqiladi agar:

- Harorat max_temp dan yuqori bo‘lsa
- Gaz miqdori max_gas dan yuqori bo‘lsa

Nasos yoqiladi agar:

- Tuproq namligi min_soil_moisture dan past bo‘lsa
- Havo namligi min_air_humidity dan past bo‘lsa

Manual Mode

Foydalanuvchi Supabase orqali:

- Ventilyatorni yoqishi/o‘chirishi
- Nasosni yoqishi/o‘chirishi

mumkin.

---

Qurilma ulanishlari

Qurilma| ESP32 Pin
DHT11| GPIO4
Soil Sensor| GPIO34
Light Sensor| GPIO32
Gas Sensor| GPIO35
Fan Relay| GPIO12
Pump Relay| GPIO14

---

Kutubxonalar

Loyiha quyidagi Arduino kutubxonalaridan foydalanadi:

WiFi.h
HTTPClient.h
ArduinoJson.h
DHT.h

Arduino IDE orqali o‘rnatish:

- ArduinoJson
- DHT sensor library

---

Supabase jadvallari

device_settings

Ustun
id
max_temp
min_soil_moisture
min_air_humidity
max_gas
cooler_status
pump_status
is_automated

sensor_logs

Ustun
temperature
humidity
soil_moisture
light_level
gas_level
created_at

---

Ishlash prinsipi

1. ESP32 WiFi tarmog‘iga ulanadi.
2. Supabase'dan joriy sozlamalarni oladi.
3. Sensorlardan ma'lumotlarni o‘qiydi.
4. Avtomatik yoki qo‘lda boshqaruv rejimini aniqlaydi.
5. Ventilyator va nasosni boshqaradi.
6. Sensor ma'lumotlarini sensor_logs jadvaliga yuboradi.
7. Avtomatik rejimda relay holatlarini device_settings jadvaliga yangilaydi.
8. Jarayon har 15 soniyada takrorlanadi.

---

WiFi sozlamalari

Kod ichida quyidagi parametrlarni o‘zgartiring:

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

---

Muallif

ESP32 + Supabase asosidagi IoT Smart Greenhouse Monitoring System.

Texnologiyalar:

- ESP32
- Arduino Framework
- Supabase REST API
- PostgreSQL
- WiFi IoT Monitoring
- DHT11 Sensor