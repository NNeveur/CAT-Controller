# Kenwood TS-2000 CAT Controller for MaTouch ESP32-S3 2.1" Display

Une application complète sous **Arduino IDE** permettant de contrôler un émetteur-récepteur **Kenwood TS-2000** via le protocole serie **CAT (Computer Aided Transceiver)**, en utilisant une carte **MaTouch ESP32-S3 Rotary IPS Display (2.1" ST7701)**.

---

## 🌟 Fonctionnalités

- 📺 **Affichage dynamique VFO A** sur écran IPS 480x480 (ST7701).
- 🎛️ **Boutons tactiles de Mode** : `LSB`, `USB`, `AM`, `FM`.
- 📡 **Boutons tactiles de Bande** : `80m`, `40m`, `20m`, `10m`.
- 🔄 **Contrôle de fréquence par encodeur rotatif** avec pas de réglage sélectionnables (`10 Hz`, `100 Hz`, `1 kHz`, `10 kHz`, `100 kHz`).
- ⚡ **Indicateur de statut CAT en temps réel** (détection de réponse du transceiver).
- 🔗 **Protocole Kenwood TS-2000 ASCII native** (`FA`, `MD`, `IF`).

---

## 🛠️ Matériel requis

1. **MaTouch ESP32-S3 Rotary IPS Display 2.1" (ST7701 / CST816D / GT911)**
2. **Module convertisseur RS232 vers TTL (MAX3232)** : Nécessaire pour adapter les niveaux de tension RS-232 du Kenwood TS-2000 (DB9 femelle) aux niveaux 3.3V TTL de l'ESP32-S3.
3. **Câble série RS-232 D-SUB 9 broches droit (Female-to-Male / Female-to-Female selon votre adaptateur)**.

---

## 🔌 Câblage Hardware

### ESP32-S3 vers Module RS232 (MAX3232) & Kenwood TS-2000

| ESP32-S3 Pin | MAX3232 TTL Pin | MAX3232 RS232 | Kenwood TS-2000 COM Port (DB9) |
|---|---|---|---|
| **GPIO 20** (TX1) | RXD | TXD (Pin 2 DB9) | **Pin 2 (RXD)** |
| **GPIO 19** (RX1) | TXD | RXD (Pin 3 DB9) | **Pin 3 (TXD)** |
| **GND** | GND | GND (Pin 5 DB9) | **Pin 5 (GND)** |
| **3.3V / 5V** | VCC | - | - |

*(Note: Assurez-vous d'avoir relié les GND du montage et du transcepteur).*

---

## 💻 Configuration Arduino IDE

### 1. Cartes et Bibliothèques requises
Dans le gestionnaire de cartes et de bibliothèques d'Arduino IDE, installez :
- **ESP32 Board Support** : `esp32` par *Espressif Systems* (v2.0.x ou v3.0.x).
- **GFX Library for Arduino** : `GFX Library for Arduino` par *Moon On Our Nation*.

### 2. Paramètres de compilation
- **Board** : `ESP32S3 Dev Module`
- **PSRAM** : `OPI PSRAM` (ou `Enabled`)
- **Flash Size** : `8MB (64Mb)` / `16MB`
- **Partition Scheme** : `Huge APP (3MB No OTA/1MB SPIFFS)`

---

## 📂 Structure du Projet

```text
.
├── Config.h               # Configuration des broches (ST7701, Encodeur, UART), bandes et pas de réglage
├── KenwoodCAT.h           # Header du pilote protocole CAT Kenwood TS-2000
├── KenwoodCAT.cpp         # Implémentation du pilote protocole CAT
├── RotaryEncoderDriver.h  # Header du gestionnaire d'encodeur rotatif et de bouton
├── RotaryEncoderDriver.cpp# Implémentation du gestionnaire d'encodeur
├── DisplayGUI.h           # Header de l'interface graphique ST7701 et boutons tactiles
├── DisplayGUI.cpp         # Implémentation du rendu UI et gestion des contacts tactiles
└── TS2000_MaTouch_CAT.ino # Sketch principal Arduino
```

---

## 🚀 Utilisation

1. Connectez le port COM du TS-2000 au MAX3232 puis à l'ESP32-S3.
2. Téléversez le code `TS2000_MaTouch_CAT.ino` via l'Arduino IDE.
3. À l'allumage :
   - L'écran affiche la fréquence active VFO A du Kenwood TS-2000.
   - Tournez le bouton rotatif pour modifier la fréquence.
   - Appuyez sur le bouton rotatif pour changer le pas de réglage (`10Hz`, `100Hz`, `1kHz`, `10kHz`, `100kHz`).
   - Touchez les boutons à l'écran `LSB`, `USB`, `AM`, `FM` pour basculer de mode de modulation.
   - Touchez les boutons `80m`, `40m`, `20m`, `10m` pour sauter directement sur la bande souhaitée.
