# 基於 FreeRTOS 的雙核邊緣智慧消防監控系統

採用 **Raspberry Pi Pico 2 W (RP2350)** + **FreeRTOS (雙核 SMP)**，整合溫濕度、煙霧、聲音感測與 Wi-Fi 網頁主控台的固定式邊緣智慧消防監控系統。具備「軟體級隱私鎖」：平時鎖死畫面，火災觸發才解鎖顯示現場影像。

## 📌 核心功能
- **環境防災監控**：DHT22 每秒讀溫濕度、MQ-2 偵測煙霧；高溫(>35°C)或偵測到煙 → 判定火災。
- **即時警報**：火災觸發紅燈閃爍後常亮、正常亮綠燈；網頁同步變紅。
- **聲音偵測**：KY-037 麥克風（AO 類比 + ADC 音量分析）偵測異常聲響。
- **歷史紀錄**：環形緩衝記錄最多 50 筆，Software Timer 每 60 秒輸出並清空。
- **Wi-Fi 網頁主控台**：瀏覽器即時顯示溫濕度/煙霧/聲音/火災狀態，局部刷新（警報 0.5 秒、溫濕度 2 秒）。
- **軟體級隱私鎖**：平時畫面鎖死，火災解鎖才顯示現場影像（本版以圖片代替相機）。

## 🛠️ 硬體與接線
| 元件 | 型號 | 接到 Pico 2 W |
| :--- | :--- | :--- |
| 主控板 | Raspberry Pi Pico 2 W (RP2350) | — |
| 溫濕度 | DHT22 / AM2302 | DAT → **GP0**，VCC → 3V3，GND → GND |
| 煙霧 | MQ-2 | DO → **GP16**，VCC → 3V3，GND → GND |
| 聲音 | KY-037 (LM393) | AO → **GP26 (ADC0)**，+ → 3V3，G → GND |
| 綠燈（正常） | LED | **GP14** → 220Ω → GND |
| 紅燈（警報） | LED | **GP15** → 220Ω → GND |

> ⚠️ MQ-2 建議用 3V3 供電，避免 5V 的 DO 輸出超過 Pico 的 3.3V GPIO 上限。
> 感測器腳位與閾值可在 [`include/shared.h`](include/shared.h) 調整。

## 🏗️ FreeRTOS 任務架構
| Task | 優先權 | 職責 |
| :--- | :--- | :--- |
| Alarm | 4 | 監聽 Queue，火災(高溫/煙)觸發紅燈+Semaphore 通知 Logger |
| Sensor | 3 | 每秒讀 DHT22(PIO) + MQ-2，送 Queue 與共享資料 |
| Sound | 3 | ADC 取樣 KY-037 AO，分析音量偵測聲響 |
| Network | 2 | cyw43 Wi-Fi 連線 + lwIP httpd 網頁伺服器 |
| Led | 1 | 板載 LED 顯示 Wi-Fi 連線狀態 |
| Logger | 1 | 環形緩衝紀錄 + Software Timer 定時清理 |

行程間通訊：Queue（Sensor→Alarm）、Binary Semaphore（Alarm→Logger）、Mutex（保護共享資料）、Software Timer（定時清理）。

## 📦 外部依賴（不納入版控，請自行取得）
1. **Pico SDK 2.2.0** — 設環境變數 `PICO_SDK_PATH`（或用官方 VS Code 擴充）。
2. **FreeRTOS-Kernel（Raspberry Pi 官方 fork）** — RP2350 必須用這個 fork：
   ```bash
   git clone https://github.com/raspberrypi/FreeRTOS-Kernel.git
   # 設環境變數 FREERTOS_KERNEL_PATH 指向它
   ```
3. **pico_dht（PIO 驅動）** — clone 到本專案根目錄：
   ```bash
   git clone https://github.com/vmilea/pico_dht.git
   ```

## 🔧 建置與燒錄
```bash
# 於專案根目錄（已 clone pico_dht、設好 PICO_SDK_PATH 與 FREERTOS_KERNEL_PATH）
cmake -S . -B build -G Ninja
cmake --build build
```
產物：`build/Fire-Camera-System.uf2`。按住 BOOTSEL 接 USB，把 `.uf2` 拖進 `RPI-RP2` 磁碟即可燒錄。

## 🌐 使用
1. 改 [`src/network_task.c`](src/network_task.c) 的 Wi-Fi 帳密（Pico 2 W 只支援 2.4GHz）。
2. 燒錄後序列埠（USB, 115200）會印出取得的 IP。
3. 瀏覽器開該 IP → 即時消防監控主控台。對感測器吹熱風/靠近煙霧 → 火災警報 + 隱私鎖解鎖顯示影像。
