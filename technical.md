# Nasıl çalışacak sistem?

## Setup

> Web sayfasının terapi cihazlarıyla çalışması için bir bağlantı kurabilmesi lazım. Web standartları gereği bir web sayfası sunucu açamaz sadece client olabilir, bu sebeple her esp32c3 cihazı bir web sunucusu çalıştıracak. Esp32c3 cihazlarına web sayfası üzerinden terapistin bağlanabilmesi için esp32c3'ün adreslerini ve portlarını bilmesi lazım. Portlar zaten önceden belirlenmiş olacak(port: 32406). Ip adresi bulmak için ise hem esp32c3 hem de web sayfası aynı Wi-Fi'de olması lazım. IP adresi almak için ya mdns aracılığıyla ip adresini alacağız [therapy-<deviceId>.local] adresiden, ya da mdns yoksa veya engelli ise web uygulamasının bağlı olduğu wifi'nin subnet'i neyse(192.168.x.x) orada ip adres araması yapacak(en kötü elle ip adresi girmesine izin veririm). 

Esp32c3 fabrika ayarlarında iken herhangi bir wifi bilgisine sahip olmayacak, bu yüzden softAP ile kendi wifi'sini oluşturacak. Web sayfasında "kurulum" sayfasından terapist cihazın üzerindeki qr kodu okutacak kamerayla, okuttuğu qr kodla birlikte işte web sayfası onu "TherapySetup-[DeviceID]" adlı wifiye bağlanmasını söyleyecek. Bağlanınca da 192.168.x.x böyle bir adresten esp32c3'e wifi şifresini girebilecek.

Girilen wifi şifresini esp32c3 NVS(non-volatile-storage) kısmına kaydedecek. Cihaz güç kaybetse bile kalıcı olarak bu saklanacak.


## Usage

> Esp32c3'ler şu anda doğru wifi içerisinde.

Aynı ağ içerisinde birden fazla terapist varsa kafa karışıklığı olmasın diye, ve aynı wifide olup başkalarının kullandığı terapi cihazlarını insanların bağlantısını koparmasını vs. engellemek için "pair"leme yapacağız.

Bunun için de şöyle olacak: Web uygulamadan "cihaz ekle" kısmından qr kod okutacak. Bu qr kod içerisinde [Setup] kısmında bahsettiğim deviceID ve pairing secret olacak. Terapist arkadaş ise bir "terapi session" başlattığı zaman rastgele bir _sessionId_ oluşturacak web sayfası, ayrıca bir de _sessionKey_ oluşturacak. QR kod okunduğunda ise esp32c3'e http üzerinden /pair adresi için istek atıcak ve deviceId, deviceSecret, sessionId ve sessionKey gönderecek.

Not: esp32c3 cihazı o anda wifiye bağlı ama pairlenmemiş bir halde. Pairlenmediyse http sunucusu açık olacak [ipAdresi:32406] adresinde. Ip adresi almak için [mdns] kullan. Mdns olmazsa ip subnet araması yap, en kötü ihtimal elle ip adresi gir(it technician fln varsa).

Sonrasında bu /pair adresine istek gelecek, esp32c3 arkadaşımız bu gelen isteğe bakacak veriler doğru mu diye, doğru ise tamamdır diyecek ve pairlenmiş olacak. Pairlenmiş ise pair endpoint'ini kapat.

Pairlenmiş esp32c3 bu sefer web sayfasından gelecek web socket bağlantısını bekleyecek. Websocket ilk açıldığı zaman _sessionKey_ gönderilmesi lazım ve doğru olmalı, doğru değilse yeniden göndermesini iste. Doğruluğu ispatlandıktan sonra o websocket bağlantısına güven.

Daha sonrasında ise çok basit, her 1 veya 2 saniyede bir zaman eşitlemesi için mesaj gönderilecek. Terapist web uygulaması üzerinden esp32c3'lere titreşim motorlarını başlatmasını söyleyince de esp32c3 gelen web socket mesajının _sessionKey_'i doğru mu ona bakacak, doğru ise de motoru belirtilen zamanda başlatacak/durduracak. 


zaman senkronizasyonu için:
Web → ESP32: t_web_now

ESP32:

offset = t_web_now - esp_millis

Motor komutları:

startAt = t_web_now + 120ms


session bittiğinde ise websocket ile "unpair" veya "session finished" komutu ver.
