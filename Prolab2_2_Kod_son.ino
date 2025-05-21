#include <LiquidCrystal.h>

// Pin Tanımlamaları
// Giriş Pinleri
const int motorBaslatPin = 2;      // Motor başlatma butonu
const int emniyetKemeriPin = 3;    // Emniyet kemeri butonu
const int sicaklikPin = A0;        // LM35 sıcaklık sensörü
const int ldrPin = A1;             // LDR ışık sensörü
const int yakitSeviyePin = A2;     // Yakıt seviyesi potansiyometresi
const int kapiDurumuPin = 4;       // Kapı anahtarı

// Çıkış Pinleri
const int emniyetKemeriLedPin = 5; // Kırmızı LED (Emniyet kemeri uyarısı)
const int farLedPin = 6;           // Mavi LED (Far durumu)
const int yakitLedPin = 7;         // Sarı LED (Yakıt seviyesi uyarısı)
const int kapiLedRPin = 8;         // RGB LED - R pini (Kapı açık uyarısı)
const int kapiLedGPin = 9;         // RGB LED - G pini
const int kapiLedBPin = 10;        // RGB LED - B pini
const int buzzerPin = 11;          // Buzzer
const int motorPin = 13;           // DC Motor (Araç motoru)
const int klimaPin = 12;           // DC Motor (Klima fanı)

// LCD Pin Tanımlamaları (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(22, 23, 24, 25, 26, 27);

// Durum Değişkenleri
bool motorDurumu = false;          // Motor çalışıyor mu?
bool emniyetKemeriTakili = false;  // Emniyet kemeri takılı mı?
bool kapiAcik = false;             // Kapı açık mı?
bool farlarAcik = false;           // Farlar açık mı?
bool klimaAcik = false;            // Klima açık mı?
float sicaklik = 0;                // Sıcaklık değeri
int yakitSeviyesi = 0;             // Yakıt seviyesi (%)
int ldrDegeri = 0;                 // LDR değeri

// Eşik Değerleri
const float sicaklikEsikDegeri = 25.0; // Klima için sıcaklık eşik değeri
const int ldrEsikDegeri = 250;        // Far açma/kapama için LDR eşik değeri
const int yakitDusukEsikDegeri = 10;  // Yakıt düşük uyarısı için eşik değeri
const int yakitKritikEsikDegeri = 5;  // Yakıt kritik uyarısı için eşik değeri

// Zamanlama Değişkenleri
unsigned long sonYakitLedDegisimi = 0;     // Yakıt LED'i için zamanlama
unsigned long sonLcdGuncelleme = 0;        // LCD güncellemesi için zamanlama
unsigned long sonButonKontrol = 0;         // Buton kontrolü için zamanlama
unsigned long sonDurumKontrol = 0;         // Genel durum kontrolü için zamanlama
unsigned long simdikiZaman = 0;            // Geçerli zaman

// Zaman Sabitleri
const unsigned long yakitLedYanipSonmeSuresi = 500;  // Yanıp sönme süresi (ms)
const unsigned long lcdGuncellemeArasi = 1000;       // LCD güncelleme aralığı (ms)
const unsigned long butonKontrolArasi = 50;          // Buton kontrol aralığı (ms)
const unsigned long durumKontrolArasi = 200;         // Durum kontrol aralığı (ms)

// Durum mesajları
bool mesajGosteriliyor = false;
String mevcutMesaj = "";
unsigned long mesajSuresi = 0;
unsigned long mesajBaslangicZamani = 0;

// Buton durum değişkenleri
bool sonMotorButonDurumu = HIGH;
bool sonEmniyetKemeriButonDurumu = HIGH;
bool sonKapiDurumu = HIGH;

void setup() {
   

  // Pin Modlarını Ayarla
  pinMode(motorBaslatPin, INPUT_PULLUP);    // Buton için pull-up direnç
  pinMode(emniyetKemeriPin, INPUT_PULLUP);  // Buton için pull-up direnç
  pinMode(kapiDurumuPin, INPUT_PULLUP);     // Anahtar için pull-up direnç
  
  pinMode(emniyetKemeriLedPin, OUTPUT);
  pinMode(farLedPin, OUTPUT);
  pinMode(yakitLedPin, OUTPUT);
  pinMode(kapiLedRPin, OUTPUT);
  pinMode(kapiLedGPin, OUTPUT);
  pinMode(kapiLedBPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(klimaPin, OUTPUT);
  
  // Tüm LED'leri test etmek için kısa süre yak
  digitalWrite(emniyetKemeriLedPin, HIGH);
  digitalWrite(farLedPin, HIGH);
  digitalWrite(yakitLedPin, HIGH);
  digitalWrite(kapiLedRPin, HIGH);
  digitalWrite(kapiLedGPin, LOW);
  digitalWrite(kapiLedBPin, HIGH);
  digitalWrite(buzzerPin, HIGH);
  
  // LCD Başlat
  lcd.begin(16, 2);
  lcd.print("Arac Guvenlik");
  lcd.setCursor(0, 1);
  lcd.print("Sistemi Hazir");
  
  delay(1000);  // LED test süresi
  
  // LED'leri kapat
  digitalWrite(emniyetKemeriLedPin, LOW);
  digitalWrite(farLedPin, LOW);
  digitalWrite(yakitLedPin, LOW);
  digitalWrite(kapiLedRPin, LOW);
  digitalWrite(kapiLedGPin, LOW);
  digitalWrite(kapiLedBPin, LOW);
  digitalWrite(buzzerPin, LOW);
  
  delay(1000);
  lcd.clear();
}

void loop() {
  // LCD'de yakıt seviyesini daima göster
lcd.setCursor(0, 0);
lcd.print("Yakit: ");
lcd.print(yakitSeviyesi);
lcd.print("% ");

  simdikiZaman = millis();
  
  // Sensör Değerlerini Oku
  emniyetKemeriTakili = !digitalRead(emniyetKemeriPin); // Buton basılı = kemer takılı
  kapiAcik = !digitalRead(kapiDurumuPin);              // Anahtar kapalı = kapı açık
  
  // Analog Sensörleri Oku
  int sicaklikOkuma = analogRead(sicaklikPin);
  sicaklik = (sicaklikOkuma * 5.0 * 100.0) / 1024.0; // LM35 değerini Celsius'a çevir
  
  ldrDegeri = analogRead(ldrPin);
  yakitSeviyesi = map(analogRead(yakitSeviyePin), 0, 1023, 0, 100); // 0-100% arası değer
  
  // Buton kontrolleri (debounce ile)
  if (simdikiZaman - sonButonKontrol >= butonKontrolArasi) {
    sonButonKontrol = simdikiZaman;
    butonKontrol();
  }
  
  // Periyodik durum kontrolü
  if (simdikiZaman - sonDurumKontrol >= durumKontrolArasi) {
    sonDurumKontrol = simdikiZaman;
    
    // Tüm kontrolleri sırayla yap
    kapininDurumunuKontrolEt();
    emniyetKemeriDurumunuKontrolEt();
    sicaklikDurumunuKontrolEt();
    farDurumunuKontrolEt();
    yakitDurumunuKontrolEt();
  }
  
  // LCD Mesaj Yönetimi
  lcdMesajYonetimi();
  
  // Yakıt LED'inin yanıp sönmesi
  yakitLedYanipSonmeKontrol();
}

// Butonların durumunu kontrol et
void butonKontrol() {
  bool simdikiMotorButonDurumu = digitalRead(motorBaslatPin);
  
  // Butona yeni basıldıysa
  if (simdikiMotorButonDurumu == LOW && sonMotorButonDurumu == HIGH) {
    motorEmniyetKontrol();  // Emniyet kemeri kontrolüyle motoru başlat
  }

  sonMotorButonDurumu = simdikiMotorButonDurumu;
}


// 1. Motor Çalıştırma ve Emniyet Kemeri Kontrolü
void motorEmniyetKontrol() {
  emniyetKemeriTakili = !digitalRead(emniyetKemeriPin);  // D3 → buton takılıysa LOW

  if (emniyetKemeriTakili) {
    // Emniyet kemeri takılıysa motor çalışabilir
    motorDurumu = !motorDurumu;

    if (motorDurumu) {
      digitalWrite(motorPin, HIGH);
      mesajGoster("Motor", "Calistirildi", 2000);
    } else {
      digitalWrite(motorPin, LOW);
      mesajGoster("Motor", "Durduruldu", 2000);
    }

    // LED ve buzzer'ı kapat
    digitalWrite(emniyetKemeriLedPin, LOW);
    digitalWrite(buzzerPin, LOW);
  } else {
    // Emniyet kemeri takılı değilse motor çalışamaz
    digitalWrite(motorPin, LOW);  // motoru durdur
    digitalWrite(emniyetKemeriLedPin, HIGH);
    digitalWrite(buzzerPin, HIGH);

    mesajGoster("Emniyet Kemeri", "Takili Degil!", 2000);

    delay(1000);  // buzzer kısa süre çalsın
    digitalWrite(buzzerPin, LOW);
  }
}


// Kapı durumunu kontrol et
void kapininDurumunuKontrolEt() {
  static bool oncekiKapiDurumu = !kapiAcik;
  
  if (kapiAcik != oncekiKapiDurumu) {
    // Kapı durumu değiştiğinde çalışır
    oncekiKapiDurumu = kapiAcik;
    
    if (kapiAcik) {
      // Kapı açık, pembe ışık yak (R + B)
      digitalWrite(kapiLedRPin, HIGH);
      digitalWrite(kapiLedGPin, LOW);
      digitalWrite(kapiLedBPin, HIGH);
      
      mesajGoster("Uyari: Kapi Acik", "Motor Calismaz", 2000);
    } else {
      // Kapı kapalı, LED'i kapat
      digitalWrite(kapiLedRPin, LOW);
      digitalWrite(kapiLedGPin, LOW);
      digitalWrite(kapiLedBPin, LOW);
      
      if (!mesajGosteriliyor) {
        lcd.clear();
      }
    }
  }
}

// Emniyet kemeri durumunu kontrol et
void emniyetKemeriDurumunuKontrolEt() {
  static bool oncekiEmniyetKemeriDurumu = emniyetKemeriTakili;
  
  if (emniyetKemeriTakili != oncekiEmniyetKemeriDurumu) {
    oncekiEmniyetKemeriDurumu = emniyetKemeriTakili;
    
    // LED'i güncelle
    digitalWrite(emniyetKemeriLedPin, !emniyetKemeriTakili);
    
    if (emniyetKemeriTakili) {
      // Kemer takıldı
      if (!mesajGosteriliyor) {
        lcd.clear(); // Eğer başka bir mesaj gösterilmiyorsa ekranı temizle
      }
    }
  } else {
    // Her durumda LED'in doğru durumda olduğundan emin ol
    digitalWrite(emniyetKemeriLedPin, !emniyetKemeriTakili);
  }
}

// Sıcaklık durumunu kontrol et
void sicaklikDurumunuKontrolEt() {
  static float sicaklikSimulasyon = 0.0;
  static bool ilkDefa = true;
  static bool klimaAcik = false;
  static bool klimaKapandi = false;
  static unsigned long sonDusmeZamani = 0;

  // İlk çalıştırmada gerçek sıcaklıkla başla
  if (ilkDefa) {
    int okuma = analogRead(sicaklikPin);
    sicaklikSimulasyon = (okuma * 5.0 * 100.0) / 1024.0;
    ilkDefa = false;

    // LCD ilk yazı
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sicaklik: ");
    lcd.print(sicaklikSimulasyon, 1);
    lcd.print((char)223); lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.print("Klima Acildi");

    digitalWrite(klimaPin, HIGH);
    klimaAcik = true;
  }

  // Sıcaklık henüz 25 değilse düşürmeye devam
  if (klimaAcik && !klimaKapandi && sicaklikSimulasyon > 25.0) {
    sicaklik -= 0.2;
    if (millis() - sonDusmeZamani >= 2000) {
      sicaklikSimulasyon -= 0.5;
      sonDusmeZamani = millis();

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sicaklik: ");
      lcd.print(sicaklikSimulasyon, 1);
      lcd.print((char)223); lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Klima Acildi");
    }
  }

  // Sıcaklık 25.0°C'ye geldiğinde klima kapatılsın
  if (klimaAcik && !klimaKapandi && sicaklikSimulasyon <= 25.0) {
    klimaKapandi = true;
    digitalWrite(klimaPin, LOW);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sicaklik: 25.0C");
    lcd.setCursor(0, 1);
    lcd.print("Klima Kapandi");

    delay(2000); // 2 saniye ekranda kalsın
    lcd.clear();
  }

  // Değeri güncelle
  sicaklik = sicaklikSimulasyon;
}










// Far durumunu kontrol et
void farDurumunuKontrolEt() {
  static bool farlarAcik = false;
  static bool mesajGosterildi = false;
  static bool farKontroluBaslasin = false;
  static unsigned long sonDegisimZamani = 0;
  static unsigned long lcdGosterimZamani = 0;
  static int ldrDegerSimulasyon = -1;

  if (!farKontroluBaslasin && sicaklik <= 25.0) {
    farKontroluBaslasin = true;
   // analogRead yerine simülasyon değeri kullanalım:
int ldrDegeri = ldrDegerSimulasyon;
 // gerçek değeri oku
    Serial.print("Baslangic LDR: "); Serial.println(ldrDegerSimulasyon);
  }

  if (!farKontroluBaslasin) return;

  // Her 1 saniyede bir LDR değerini yükselt
  if (millis() - sonDegisimZamani >= 1000 && ldrDegerSimulasyon < 300) {
    sonDegisimZamani = millis();
    ldrDegerSimulasyon += 15;  // yavaş yavaş artır
  }

  // Farlar Açık senaryosu
  if (ldrDegeri <= 250 && !farlarAcik) {
    farlarAcik = true;
    digitalWrite(farLedPin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Farlar Acik");
    mesajGosterildi = true;
    lcdGosterimZamani = millis();
}
else if (ldrDegeri > 250 && farlarAcik) {
    farlarAcik = false;
    digitalWrite(farLedPin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Farlar Kapandi");
    mesajGosterildi = true;
    lcdGosterimZamani = millis();
}


  // LCD temizle
  if (mesajGosterildi && millis() - lcdGosterimZamani >= 2000) {
    lcd.clear();
    mesajGosterildi = false;
  }
}


// Yakıt durumunu kontrol et
void yakitDurumunuKontrolEt() {
  static int oncekiYakitDurumu = -1;
  int yakitDurumu;

  // Yakıt durumu belirle
  if (yakitSeviyesi == 0) yakitDurumu = 3;
  else if (yakitSeviyesi <= 5) yakitDurumu = 2;
  else if (yakitSeviyesi <= 10) yakitDurumu = 1;
  else yakitDurumu = 0;

  if (yakitDurumu != oncekiYakitDurumu) {
    oncekiYakitDurumu = yakitDurumu;

    switch (yakitDurumu) {
      case 0:
        digitalWrite(yakitLedPin, LOW);
        break;

      case 1:
        digitalWrite(yakitLedPin, HIGH);
        mesajGoster("Uyari: Yakit", "Seviye Dusuk - %" + String(yakitSeviyesi), 2000);
        break;

      case 2:
        mesajGoster("Kritik: Yakit", "Cok Az - %" + String(yakitSeviyesi), 2000);
        break;

      case 3:
        if (motorDurumu) {
          motorDurumu = false;
          digitalWrite(motorPin, LOW);
        }

        // Tüm LED’leri kapat
        digitalWrite(yakitLedPin, LOW);
        digitalWrite(emniyetKemeriLedPin, LOW);
        digitalWrite(farLedPin, LOW);
        mesajGoster("Yakit Bitti", "Motor Durdu", 2000);
        break;
    }
  }
}


// Yakıt LED'inin yanıp sönmesini kontrol et
void yakitLedYanipSonmeKontrol() {
  if (yakitSeviyesi <= yakitKritikEsikDegeri && yakitSeviyesi > 0) {
    if (simdikiZaman - sonYakitLedDegisimi >= yakitLedYanipSonmeSuresi) {
      sonYakitLedDegisimi = simdikiZaman;
      digitalWrite(yakitLedPin, !digitalRead(yakitLedPin));
    }
  }
}


// LCD mesajlarını yönet
void lcdMesajYonetimi() {
  // Mesaj süresi dolduysa ekranı temizle
  if (mesajGosteriliyor && (simdikiZaman - mesajBaslangicZamani >= mesajSuresi)) {
    mesajGosteriliyor = false;
    lcd.clear();
  }
}

// LCD'de mesaj göster
void mesajGoster(String satir1, String satir2, unsigned long sure) {
  lcd.clear();
  lcd.print(satir1);
  
  if (satir2 != "") {
    lcd.setCursor(0, 1);
    lcd.print(satir2);
  }
  
  mesajGosteriliyor = true;
  mesajSuresi = sure;
  mesajBaslangicZamani = simdikiZaman;
}