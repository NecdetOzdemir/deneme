import pandas as pd
import numpy as np
from sklearn.linear_model import LinearRegression
from sklearn.metrics import r2_score

# ==================== AYARLAR ====================
ESIK_DUSEN = -0.10      # Düşen trendde kapı açılımı için eşik (°C) - POZİTİF!
ESIK_YUKSELEN = 0.20   # Yükselen trendde kapı açılımı için eşik (°C) - POZİTİF!

# CSV dosya yolu
CSV_PATH = "/home/necdet/Masaüstü/dolap/temp.csv"

# ==================== 1. VERİ OKUMA ====================
def veri_oku():
    """CSV dosyasını okur ve temizler"""
    print("=== 1. VERİ OKUMA ===")
    try:
        df = pd.read_csv(CSV_PATH)
        print("✅ CSV dosyası başarıyla okundu")
        
        # Tarih sütununu datetime formatına çevir
        df['Tarih'] = pd.to_datetime(df['Tarih'])
        
        # Value sütununu sayısal formata çevir
        df['Value'] = pd.to_numeric(df['Value'])
        
        # Verileri sırala
        df_sorted = df.sort_values('Tarih').reset_index(drop=True)
        
        print(f"Toplam veri noktası: {len(df_sorted)}")
        print(f"Tarih aralığı: {df_sorted['Tarih'].min()} - {df_sorted['Tarih'].max()}")
        print(f"Sıcaklık aralığı: {df_sorted['Value'].min():.2f}°C - {df_sorted['Value'].max():.2f}°C")
        
        return df_sorted
        
    except Exception as e:
        print(f"❌ Hata oluştu: {e}")
        return None

# ==================== 2. TREND ANALİZİ ====================
def trend_belirle(index, df_data, window=5):
    """Belirli bir noktanın trendini belirle"""
    if index < window:
        return "bilinmiyor"
    
    # Önceki window sayıda noktaya bak
    prev_values = df_data['Value'].iloc[index-window:index]
    if len(prev_values) < 3:  # En az 3 nokta gerekli
        return "bilinmiyor"
    
    try:
        # Linear regresyon ile eğim hesapla
        x = np.array(range(len(prev_values))).reshape(-1, 1)
        y = np.array(prev_values)
        
        model = LinearRegression()
        model.fit(x, y)
        trend_slope = model.coef_[0]
        r2 = r2_score(y, model.predict(x))
        
        # Toplam değişim
        total_change = prev_values.iloc[-1] - prev_values.iloc[0]
        
        # Kriterler:
        min_r2 = 0.6
        min_change = 0.03
        
        if r2 < min_r2 or abs(total_change) < min_change:
            return "sabit"  # Yeterli trend yok
        elif trend_slope < -0.003 and total_change < -min_change:
            return "dusen"   # Açık düşen trend
        elif trend_slope > 0.003 and total_change > min_change:
            return "yukselen"  # Açık yükselen trend
        else:
            return "sabit"  # Belirsiz
        
    except:
        return "bilinmiyor"

# ==================== 3. DÜŞEN TREND ANOMALİ KONTROLÜ ====================
def dusen_trend_anomali_kontrol(index, df_sorted):
    """
    Düşen trendde anomali kontrolü:
    - Eğer (şimdi - önceki) > eşik (0.10) ise 
    - Sonraki 7 değeri kontrol et
    - En az 3 tanesi bir öncekinden küçükse → anomali (kapı açılmış)
    - 2 veya daha azı düşüş gösteriyorsa → normal (soğutucu kapanmış)
    """
    if index >= len(df_sorted) - 7:  # Son 7 değer yoksa kontrol edemeyiz
        return False
    
    # Şu anki ve önceki değerleri al
    onceki_deger = df_sorted['Value'].iloc[index-1]
    simdiki_deger = df_sorted['Value'].iloc[index]
    fark = simdiki_deger - onceki_deger
    
    # Önce eşik kontrolü
    if fark > ESIK_DUSEN:  # POZİTİF artış ve eşikten büyük
        # Sonraki 7 değeri kontrol et
        dusus_sayisi = 0  # Düşüş gösteren değer sayısı
        
        for i in range(1, 8):  # 1, 2, 3, 4, 5, 6, 7 (sonraki 7 değer)
            if index + i >= len(df_sorted):
                break
                
            onceki_kontrol = df_sorted['Value'].iloc[index + i - 1]
            simdiki_kontrol = df_sorted['Value'].iloc[index + i]
            kontrol_fark = simdiki_kontrol - onceki_kontrol
            
            # Eğer fark <= 0 ise düşüş var
            if kontrol_fark <= 0:
                dusus_sayisi += 1
        
        # En az 3 düşüş varsa anomali
        if dusus_sayisi >= 3:
            return True  # Anomali (kapı açılmış)
        else:
            return False  # Normal (soğutucu kapanmış)
    
    return False
# ==================== 4. KAPAK AÇILIMI ANALİZİ ====================
def kapi_acilimi_analiz(df_sorted):
    """Kapak açılımı tespiti yapar"""
    print("\n=== 3. KAPAK AÇILIMI ANALİZİ ===")
    
    # Zaman farkları (dakika cinsinden) ve sıcaklık farklarını hesapla
    time_diffs = df_sorted['Tarih'].diff().dt.total_seconds() / 60  # dakika cinsinden
    temp_diffs = df_sorted['Value'].diff()
    
    # Değişim hızı (°C/dakika)
    change_rates = temp_diffs / time_diffs
    change_rates.iloc[0] = 0  # İlk satır NaN olacağı için 0 yap
    
    # Yeni sütunları ekle
    df_sorted['change_rate'] = change_rates
    df_sorted['time_diff_min'] = time_diffs
    df_sorted['temp_diff'] = temp_diffs
    
    print(f"Eşik değerler:")
    print(f"  Düşen trend: fark > {ESIK_DUSEN}°C (pozitif artış!)")
    print(f"  Yükselen trend: fark > {ESIK_YUKSELEN}°C (pozitif artış!)")
    
    # KAPAK AÇILIMI TESPİTİ
    kapi_acilimlar = []
    
    for i in range(1, len(df_sorted)):
        onceki_deger = df_sorted['Value'].iloc[i-1]
        simdiki_deger = df_sorted['Value'].iloc[i]
        fark = simdiki_deger - onceki_deger
        
        # Trendi belirle
        trend = trend_belirle(i-1, df_sorted)
        
        # DÜŞEN TRENDDE KAPAK AÇILIMI
        if trend == "dusen" and fark > ESIK_DUSEN:
            # Detaylı anomali kontrolü yap
            if dusen_trend_anomali_kontrol(i, df_sorted):
                print(f"\n🔍 Düşen trendde ANOMALİ tespit edildi:")
                print(f"   Tarih: {df_sorted['Tarih'].iloc[i]}")
                print(f"   {onceki_deger:.2f}°C → {simdiki_deger:.2f}°C (fark: {fark:.2f}°C)")
                print(f"   → 🚪 KAPI AÇILDI!")
                
                kapi_acilimlar.append({
                    'tarih': df_sorted['Tarih'].iloc[i],
                    'tip': 'kapi_acilimi',
                    'aciklama': 'Düşen trendde anomali',
                    'fark': fark,
                    'trend': 'dusen'
                })
        
        # YÜKSELEN TRENDDE KAPAK AÇILIMI
        elif trend == "yukselen" and fark > ESIK_YUKSELEN:
            print(f"\n🔍 Yükselen trendde ani artış tespit edildi:")
            print(f"   Tarih: {df_sorted['Tarih'].iloc[i]}")
            print(f"   {onceki_deger:.2f}°C → {simdiki_deger:.2f}°C (fark: {fark:.2f}°C)")
            print(f"   → 🚪 KAPI AÇILDI")
            
            kapi_acilimlar.append({
                'tarih': df_sorted['Tarih'].iloc[i],
                'tip': 'kapi_acilimi',
                'aciklama': 'Yükselen trendde ani artış',
                'fark': fark,
                'trend': 'yukselen'
            })
    
    print(f"\n📊 TOPLAM KAPI AÇILIMI: {len(kapi_acilimlar)}")
    
    return kapi_acilimlar

# ==================== 5. ANA FONKSİYON ====================
def main():
    """Ana çalışma fonksiyonu"""
    print("🏠 DOLAP SICAKLIK ANALİZİ")
    print("=" * 50)
    
    # 1. Veriyi oku
    df = veri_oku()
    if df is None:
        return
    
    # 2. Kapak açılımı analizi yap
    sonuclar = kapi_acilimi_analiz(df)
    
    # 3. Detaylı sonuçları göster
    if sonuclar:
        print("\n📋 DETAYLI SONUÇLAR:")
        print("=" * 50)
        for i, olay in enumerate(sonuclar, 1):
            print(f"{i:2d}. {olay['tarih']} | {olay['tip']} | Fark: {olay['fark']:.2f}°C")
            print(f"    Trend: {olay['trend']} | {olay['aciklama']}")

# ==================== PROGRAMI ÇALIŞTIR ====================
if __name__ == "__main__":
    main()