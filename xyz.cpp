#include <iostream>     // Ekrana yazdırma ve hata mesajları için
#include <fstream>      // Dosya okumak için
#include <sstream>      // Satır içi veriyi parçalayıp ayrıştırmak için
#include <vector>       // Dinamik dizi (vector) kullanmak için
#include <string>       // String sınıfı için
#include <cmath>        // Matematik işlemleri için (abs vb.)

using namespace std;

// Veri yapısı: her satırdaki record_id ve temp değerlerini tutar
struct Veri {
    int record_id;   // Satırdaki kayıt numarası
    double temp;     // Sıcaklık değeri
};

// Anomali tespit fonksiyonu
// Girilen sıcaklık verileri içindeki yüzde değişim %esikYuzde'den büyükse o indeks anomalidir
vector<int> tespitEtAnomali(const vector<double>& veriler, double esikYuzde) {
    vector<int> anomaliIndeksleri;  // Anomali bulunan indeksleri tutacak

    // 1. elemandan başla çünkü 0. elemanın önceki yok
    for (size_t i = 1; i < veriler.size(); ++i) {
        double fark = veriler[i] - veriler[i - 1];           // Şimdiki ve önceki değer farkı
        double yuzdeDegisim = fark / veriler[i - 1];         // Yüzde değişim oranı

        if (abs(yuzdeDegisim) > esikYuzde) {                 // Eşikten büyükse anomali
            anomaliIndeksleri.push_back(i);                  // İndeksi listeye ekle
        }
    }

    return anomaliIndeksleri; // Anomali indekslerini döndür
}

int main() {
    ifstream dosya("temp.csv");      // CSV dosyasını aç
    string satir;                    // Dosyadan okunan her satır için geçici değişken
    bool baslikAtlandi = false;      // Başlık satırını atlamak için kontrol değişkeni

    vector<Veri> veriSeti;           // record_id ve temp değerlerini saklayacağımız vektör
    vector<double> sicaklikVerisi;   // Sadece sıcaklık değerlerinin saklanacağı vektör

    // Dosya açma kontrolü
    if (!dosya.is_open()) {
        cerr << "❌ Dosya acilamadi. 'temp.csv' dosyasini kontrol et." << endl;
        return 1;                    // Dosya açılamazsa programı hata ile bitir
    }

    // Başlık satırını oku ve atla
    if (!getline(dosya, satir)) {
        cerr << "❌ Dosya boş veya erişim problemi." << endl;
        return 1;                    // Dosya boşsa veya okunamazsa hata ver
    }

    // Dosyada satır satır ilerle
    while (getline(dosya, satir)) {
        stringstream ss(satir);      // Okunan satırı ',' bazlı parçalara ayırmak için stringstream
        string hucre;                // Her sütundaki değeri tutacak geçici string

        Veri veri;                   // Yeni Veri objesi oluştur
        bool satirGecerli = true;   // Satırın doğru formatta olduğunu varsay

        // 1. sütun: record_id (int olarak al)
        if (!getline(ss, hucre, ',')) {   // İlk sütunu al
            satirGecerli = false;          // Sütun yoksa satır geçersiz
        } else {
            try {
                veri.record_id = stoi(hucre);  // string'i int'e çevir
            } catch (...) {
                satirGecerli = false;          // Çevirme hatası varsa satır geçersiz
            }
        }

        // 2-4. sütunlar: month, day, year (atla)
        for (int i = 0; i < 3; i++) {
            getline(ss, hucre, ',');          // Bu sütunları okumadan geçiyoruz
        }

        // 5. sütun: temp (double)
        if (!getline(ss, hucre, ',')) {       // 5. sütunu oku
            satirGecerli = false;             // Okunamazsa satır geçersiz
        } else {
            if (hucre == "NA") {               // "NA" ise geçersiz say
                satirGecerli = false;
            } else {
                try {
                    veri.temp = stod(hucre);  // String'i double'a çevir
                } catch (...) {
                    satirGecerli = false;     // Çevirme hatası varsa geçersiz
                }
            }
        }

        // Geri kalan sütunları okumuyoruz çünkü kullanmıyoruz

        // Satır geçerli ise veriSeti ve sicaklikVerisi'ne ekle
        if (satirGecerli) {
            veriSeti.push_back(veri);           // record_id ve temp
            sicaklikVerisi.push_back(veri.temp);// sadece temp
        }
    }

    dosya.close();  // Dosya okuma tamamlandı, kapat

    // Anomali tespiti için %25 eşik koyduk
    double esik = 0.25;
    vector<int> anomaliIndeksleri = tespitEtAnomali(sicaklikVerisi, esik);

    // Toplam veri sayısını yazdır
    cout << "\n📈 Toplam veri sayisi: " << sicaklikVerisi.size() << endl;
    cout << "🚨 Anomali tespit edilen satirlar:\n";

    // Eğer anomali yoksa yaz
    if (anomaliIndeksleri.empty()) {
        cout << "YOK\n";
    } else {
        // Bulunan anomali indekslerini yazdır
        for (int idx : anomaliIndeksleri) {
            // CSV dosyasında satır numarası başlık + 1 tabanlı olduğu için +2 eklenir
            cout << "CSV Satir (data index): " << (idx + 2)
                 << ", record_id: " << veriSeti[idx].record_id
                 << ", sicaklik: " << veriSeti[idx].temp << endl;
        }
    }

    cout << "\n✅ Tespit tamamlandi.\n";

    return 0;
}
