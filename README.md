# Araç kontrol sistemi simülasyonu

Arduino ve Proteus ile araçtaki basit sensör/uyarı davranışlarını modellediğim ders projesi. Emniyet kemeri ve kapı durumu, sıcaklık, ışık ve yakıt seviyesi girdilerini motor, fan, LED, buzzer ve LCD çıkışlarına bağlıyor.

## Dosyalar

- [Prolab2_2_Kod_son.ino](Prolab2_2_Kod_son.ino): sensör okuma, eşikler ve kontrol döngüsü.
- [Prolab2_2.pdsprj](Prolab2_2.pdsprj): Proteus devre projesi.
- [AKS_sunum.pdf](AKS_sunum.pdf): çalışma sunumu.

## Çalıştırma

Arduino IDE'de `.ino` dosyasını aç. LCD pinleri 22–27 kullanıldığı için Arduino Mega ile uyumlu pin düzeni gerekiyor; `LiquidCrystal` kütüphanesi de gerekli. Devreyi Proteus'ta açıp derlenen firmware'i simülasyondaki karta tanımla. Kod başındaki pin eşlemeleri bağlantıları gösteriyor.

Bu bir eğitim simülasyonu. Gerçek araç kontrolü veya güvenlik sistemi olarak kullanılmak üzere geliştirilmedi. Otomatik Proteus yedekleri ve kişisel çalışma alanı dosyaları repodan çıkarıldı; ana devre ve kaynak korunuyor.
