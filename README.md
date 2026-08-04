# ESP32 + ILI9488 TFT LCD Animation Demo

Contoh program sederhana untuk membuat **animasi objek bergerak** pada **LCD TFT 3.5" ILI9488 (320×480)** menggunakan **ESP32 NodeMCU DevKit** dan library **TFT_eSPI**.

Program ini menampilkan sebuah lingkaran berwarna merah yang bergerak secara horizontal dari sisi kiri menuju sisi kanan layar. Contoh ini bertujuan untuk menunjukkan cara melakukan pembaruan (refresh) tampilan pada LCD TFT menggunakan teknik sederhana dengan menghapus posisi sebelumnya dan menggambar ulang objek pada posisi baru.

---

# Fitur

* Inisialisasi LCD ILI9488
* Animasi objek bergerak
* Penghapusan objek sebelumnya (erase)
* Penggambaran ulang objek (redraw)
* Contoh dasar pembuatan animasi pada TFT LCD

---

# Perangkat Keras

* ESP32 NodeMCU DevKit
* TFT LCD 3.5" ILI9488 SPI (320×480)
* Kabel jumper

---

# Wiring

| LCD ILI9488 | ESP32               |
| ----------- | ------------------- |
| VCC         | 3.3V                |
| GND         | GND                 |
| SCK         | GPIO18              |
| SDI (MOSI)  | GPIO23              |
| SDO (MISO)  | GPIO19 *(opsional)* |
| CS          | GPIO5               |
| DC / RS     | GPIO2               |
| RST         | GPIO4               |
| LED         | 3.3V                |

> **Catatan:** Pin touch screen tidak diperlukan untuk menjalankan contoh animasi ini.

---

# Susunan file
Ekstrak terlebih dahulu libraries.zip, kemudian move / copy folder TFT_eSPI ke folder libraries yang sudah ada
di folder tempat menyimpan projek Arduino IDE
```text
Arduino (tempat penyimpanan project)
│
├── libraries
│         └── TFT_eSPI
├── test_tft
└── test_tft_animation

```

---

# Cara Kerja Program

Program bekerja dengan prinsip **erase and redraw**, yaitu menghapus gambar lama kemudian menggambarnya kembali pada posisi yang baru.

Urutan prosesnya adalah sebagai berikut:

1. Menginisialisasi LCD menggunakan library TFT_eSPI.
2. Mengatur orientasi layar.
3. Membersihkan layar dengan warna hitam.
4. Menggambar lingkaran merah pada posisi awal.
5. Menghapus lingkaran sebelumnya dengan menggambar lingkaran berwarna hitam pada koordinat lama.
6. Menggeser posisi lingkaran sejauh 1 piksel ke kanan.
7. Menggambar kembali lingkaran merah pada posisi baru.
8. Memberikan jeda sekitar 10 ms agar gerakan terlihat halus.
9. Setelah mencapai sisi kanan layar, posisi lingkaran dikembalikan ke sisi kiri dan animasi diulang terus-menerus.

---

# Alur Program

```text
setup()
│
├── Inisialisasi LCD
├── Mengatur orientasi layar
└── Membersihkan layar

loop()
│
├── Hapus lingkaran lama
├── Geser posisi X
├── Jika mencapai batas kanan
│      └── Kembali ke posisi awal
├── Gambar lingkaran baru
├── Delay 10 ms
└── Ulangi
```

---

# Library yang Digunakan

* TFT_eSPI (Bodmer)

Instal melalui **Library Manager** pada Arduino IDE.

---

# Konfigurasi TFT_eSPI

Pastikan file **User_Setup.h** telah disesuaikan.

## Driver LCD

Aktifkan hanya driver berikut:

```cpp
#define ILI9488_DRIVER
```

Pastikan driver lain seperti `ILI9341_DRIVER` dinonaktifkan.

---

## Konfigurasi Pin

```cpp
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18

#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
```

---

## Frekuensi SPI

Disarankan menggunakan:

```cpp
#define SPI_FREQUENCY 20000000
```

---

# Hasil Program

Setelah program berhasil diunggah ke ESP32, layar TFT akan menampilkan:

* Latar belakang berwarna hitam.
* Sebuah lingkaran merah yang bergerak dari kiri ke kanan.
* Ketika mencapai ujung layar, lingkaran akan kembali ke posisi awal dan bergerak kembali secara terus-menerus.

Animasi ini merupakan contoh dasar yang dapat dikembangkan menjadi berbagai aplikasi seperti:

* Menu grafis
* Dashboard IoT
* Indikator status
* Game sederhana
* Visualisasi data sensor
* Antarmuka Human Machine Interface (HMI)

---

# Cara Menjalankan

1. Install Arduino IDE.
2. Install board ESP32 melalui Board Manager.
3. Install library TFT_eSPI.
4. Sesuaikan file `User_Setup.h` dengan konfigurasi pin yang digunakan.
5. Hubungkan ESP32 ke komputer.
6. Pilih board **ESP32 Dev Module**.
7. Upload program.
8. Setelah proses upload selesai, animasi lingkaran akan langsung berjalan pada layar TFT.

---

# Troubleshooting

### Layar hanya berwarna putih

Kemungkinan penyebab:

* Driver LCD belum menggunakan `ILI9488_DRIVER`.
* Konfigurasi pin SPI tidak sesuai.
* Wiring salah.
* Modul LCD bukan versi SPI.
* Konfigurasi `User_Setup.h` belum benar.

---

### Animasi tidak bergerak

Periksa hal berikut:

* Fungsi `loop()` berjalan dengan benar.
* Variabel posisi (`x`) diperbarui setiap iterasi.
* Nilai `delay()` tidak terlalu besar.
* Tidak ada fungsi lain yang menghambat proses refresh layar.

---

# Lisensi

Proyek ini dibuat sebagai media pembelajaran mengenai penggunaan LCD TFT ILI9488 dengan ESP32 dan dapat digunakan, dimodifikasi, maupun dikembangkan lebih lanjut untuk keperluan pendidikan maupun penelitian.
