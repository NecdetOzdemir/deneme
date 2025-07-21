#include <iostream>     // Ekrana yazdırma ve hata mesajları için
#include <fstream>      // Dosya okumak için
#include <sstream>      // Satır içi veriyi parçalayıp ayrıştırmak için
#include <vector>       // Dinamik dizi (vector) kullanmak için
#include <string>       // String sınıfı için
#include <cmath>        // Matematik işlemleri için (abs vb.)
#include <ctime>
using namespace std;

// Veri yapısı: her satırdaki record_id ve temp değerlerini tutar
struct Veri {
    int record_id;   // Satırdaki kayıt numarası
    double temp;     // Sıcaklık değeri
};
int isolateDepth(const vector<double>& data, double target, int depth = 0) {
    // Eğer veri küçükse (izole edilmişse) derinliği döndür
    if (data.size() <= 1) return depth;

    // Rastgele bir bölme değeri seç
    int randIndex = rand() % data.size();
    double splitValue = data[randIndex];

    // Veriyi sola ve sağa ayır
    vector<double> left, right;
    for (double val : data) {
        if (val < splitValue) left.push_back(val);
        else if (val > splitValue) right.push_back(val);
    }

    // Hedef hangi tarafta yer alıyor?
    if (target < splitValue) return isolateDepth(left, target, depth + 1);
    else if (target > splitValue) return isolateDepth(right, target, depth + 1);
    else return depth + 1; // Tam eşleştiyse izole edilmiş say

}

int main(){
    srand(time(0)); 
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

    vector<int> ortalamaDerinlikler;
    int numTrees = 50;
    for (const auto& v : veriSeti) {
        int toplamDerinlik = 0;
        for (int i = 0; i < numTrees; ++i) {
            toplamDerinlik += isolateDepth(sicaklikVerisi, v.temp);
        }
        ortalamaDerinlikler.push_back(toplamDerinlik / numTrees);
    }
    cout << "Anomali Tespiti:\n";
    for (size_t i = 0; i < veriSeti.size(); ++i) {
        cout << "ID: " << veriSeti[i].record_id
             << " | Temp: " << veriSeti[i].temp
             << " | Derinlik: " << ortalamaDerinlikler[i];

        if (ortalamaDerinlikler[i] < 3)
            cout << " <-- ANOMALI!";
        cout << endl;
    }


  

    return 0;
}
