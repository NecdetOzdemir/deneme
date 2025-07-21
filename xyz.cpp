#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
using namespace std;

struct Veri {
    int record_id;
    double temp;
};

// İzolasyon derinliği hesaplama (iteratif, maxDepth sınırıyla)
int isolateDepthSingleTree(const vector<double>& data, double target, int maxDepth) {
    int depth = 0;
    vector<double> currentData = data;

    while (currentData.size() > 1 && depth < maxDepth) {
        int randIndex = rand() % currentData.size();
        double splitValue = currentData[randIndex];

        vector<double> left, right;
        for (double val : currentData) {
            if (val < splitValue)
                left.push_back(val);
            else
                right.push_back(val);
        }

        if (left.empty() || right.empty())
            break;

        if (target < splitValue)
            currentData = left;
        else
            currentData = right;

        depth++;
    }

    return depth;
}

int main() {
    srand(time(0));

    ifstream dosya("temp.csv");
    if (!dosya.is_open()) {
        cerr << "❌ Dosya acilamadi. 'temp.csv' kontrol et." << endl;
        return 1;
    }

    string satir;
    getline(dosya, satir); // Başlık satırını atla

    vector<Veri> veriSeti;
    vector<double> sicaklikVerisi;

    // Dosyadan veri oku
    while (getline(dosya, satir)) {
        stringstream ss(satir);
        string hucre;
        Veri veri;
        bool satirGecerli = true;

        if (!getline(ss, hucre, ',')) satirGecerli = false;
        else {
            try { veri.record_id = stoi(hucre); }
            catch (...) { satirGecerli = false; }
        }

        for (int i = 0; i < 3; i++) getline(ss, hucre, ',');

        if (!getline(ss, hucre, ',')) satirGecerli = false;
        else {
            if (hucre == "NA") satirGecerli = false;
            else {
                try { veri.temp = stod(hucre); }
                catch (...) { satirGecerli = false; }
            }
        }

        if (satirGecerli) {
            veriSeti.push_back(veri);
            sicaklikVerisi.push_back(veri.temp);
        }
    }
    dosya.close();

    cout << "✅ Toplam veri sayisi: " << veriSeti.size() << endl;
    cout << "✅ Sicaklik verisi sayisi: " << sicaklikVerisi.size() << endl;

    int numTrees = 300;      // Ağaç sayısı
    int maxDepth = 12;      // Maks derinlik
    int subsampleSize = 512; // Her ağaç için örneklem büyüklüğü

    random_device rd;
    mt19937 g(rd());

    vector<int> ortalamaDerinlikler;
    ortalamaDerinlikler.reserve(veriSeti.size());

    for (size_t i = 0; i < veriSeti.size(); ++i) {
        if (i % 200 == 0) { // Her 200 adımda ilerleme göster
            cout << "İşleniyor: " << i << "/" << veriSeti.size() << endl;
        }

        int toplamDerinlik = 0;

        for (int j = 0; j < numTrees; ++j) {
            vector<double> sample = sicaklikVerisi;

            if (sample.size() > subsampleSize) {
                shuffle(sample.begin(), sample.end(), g);
                sample.resize(subsampleSize);
            }

            toplamDerinlik += isolateDepthSingleTree(sample, veriSeti[i].temp, maxDepth);
        }

        ortalamaDerinlikler.push_back(toplamDerinlik / numTrees);
    }

    cout << "\n📊 Anomali Tespiti:\n";
    for (size_t i = 0; i < veriSeti.size(); ++i) {
        if (ortalamaDerinlikler[i] < 7){
            cout << "ID: " << veriSeti[i].record_id
             << " | Temp: " << veriSeti[i].temp
             << " | Derinlik: " << ortalamaDerinlikler[i];
            cout << " <-- ⚠️ ANOMALI!";
            cout << endl;
        }
    }

    return 0;
}
