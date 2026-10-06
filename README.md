# Compiz Plugins Experimental for Linux Mint MATE

專為 **Linux Mint 22.3 MATE**（基於 Ubuntu 24.04 Noble、Compiz 0.9.14.2 核心）打造的 Compiz 實驗性與經典 3D 特效外掛整合專案。

本專案整合了經典的 `compiz-plugins-experimental` 與現代 Compiz 0.9.x C++ 架構的 `compiz_elements`，解決了長年以來在 Compiz 0.9 系列上貼圖無法載入、顯示清單衝突、CCSM 卡死、GSettings Schemas 遺失等重大 Bug，並提供開箱即用的一鍵編譯與 DEB 打包腳本。

---

## 🌟 特色與收錄外掛 (Included Plugins)

本套件收錄了 **25 個** 經修復與相容性測試的 Compiz 特效外掛：

### 1. Elements 家族系列 (多元桌面動態特效)
- **Elements (`elements`)**：核心引擎，支援在畫面上繪製落下、飄浮的自訂物件。
- **秋天落葉 (`autumn`)**：秋季落葉隨風飄落的浪漫特效。
- **桌面氣泡 (`bubbles`)**：彩虹漸層透明氣泡從底部緩緩升起。
- **夏夜螢火蟲 (`fireflies`)**：具發光尾跡的螢火蟲在桌面上自由飛舞。
- **冬季下雪 (`snow`)**：逼真的桌面飄雪特效，支援風向與搖擺幅度設定。
- **星空背景 (`stars`)**：旋轉星芒效果。

### 2. 3D 桌面立方體增強 (3D Cube Addons)
- **海底水族館 (`atlantis`)**：在透明旋轉立方體內部呈現 3D 水底世界，包含游動的魚群、鯊魚、水草與光影。
- **即時 3D 地球 (`earth`)**：在立方體中央即時渲染具大氣層、雲層與日夜光影交替的旋轉 3D 地球（GLSL Shader 著色器支援）。
- **方塊雪球 (`snowglobe`)**：在旋轉立方體內部下雪的立體雪球特效。
- **3D 模型渲染 (`cubemodel`)**：在立方體內載入並渲染自訂的 3D OBJ 幾何模型。

### 3. 視窗管理與進階視覺效果
- **紅藍 3D 立體 (`anaglyph`)**：3D 紅藍立體眼鏡立體視角支援。
- **幽靈視窗 (`ghost`)**：將視窗半透明化並穿透滑鼠事件。
- **視窗置頂切換 (`mswitch`)**：多視窗快速狀態切換。
- **相片輪盤 (`photo`)**：如相片展示架般的 3D 檢視。
- **智慧視窗放置 (`putplus` / `smartput`)**：強化的數字鍵盤視窗吸附定位與排列。
- **螢幕保護特效 (`screensaver`)**：Compiz 原生 3D 動畫螢幕保護。
- **堆疊切換 (`stackswitch`)**：經典立體堆疊式視窗切換器。
- **視窗平鋪 (`tile`)**：自動平鋪排列視窗。
- **視窗對調 (`swap`)**：快速對換兩個視窗的位置與大小。
- **滑鼠甩出 (`throw`)**：支援物理慣性拖曳拋出視窗。
- **視窗裝飾開關 (`toggledeco`)**：快速切換視窗邊框與陰影。
- **偽透明 (`fakeargb`)** / **靜態效果 (`static`)** / **對話框聚焦 (`dialog`)**。

---

## 🛡️ 系統衝突保護 (Excluded System Plugins)

為保護 Linux Mint 22.3 系統穩定性，避免 `dpkg` 覆蓋系統既有套件檔案，本套件**嚴格排除了**下列 4 個官方 `compiz-plugins` 已自帶的外掛：
- `animationplus`
- `animationsim`
- `freewins`
- `wizard`

安裝本套件**絕不覆蓋**任何系統既有檔案，安全可靠。

---

## 🛠️ 重大錯誤修復記錄 (Bug Fixes & Modernization)

相較於舊版原始碼，本專案完整融入以下重大修正：

1. **修復 Elements 特效無法顯示（OpenGL DisplayList 衝突）**：
   - 舊版程式碼在編譯 Quad 顯示清單時寫死為清單編號 `1`，造成不同貼圖的 display list 相互覆蓋。修復為動態配置之 `glNewList(displayList, GL_COMPILE)`。
2. **修復背面剔除與視窗遮擋問題**：
   - 加入 `glDisable(GL_CULL_FACE)`，避免貼圖旋轉時被 OpenGL 當作背面剔除。
   - 修正 `over_windows` 為 false 時在視窗下方繪製的 handler 呼叫。
3. **貼圖路徑查找與格式回退機制**：
   - Compiz 核心強制在 `images/` 子目錄查找貼圖，本專案補齊 `/usr/share/compiz/elements/images/`，並加入全路徑與副檔名（`.svg` ↔ `.png`）互轉自動回退機制。
4. **修復 CCSM 啟用時所有特效同時噴發的問題（Autostart 行為修正）**：
   - 將所有子外掛的 `autostart` 預設值修正為 `false`。
   - 在 C++ 實作了 `optionSetAutostartNotify`，在 CCSM 中勾選或取消勾選「Autostart」時即時播放/停止該特效。
   - 外掛停用時自動停止動畫，避免殘留或崩潰。
5. **快捷鍵統一調整**：
   - 「切換選中元素」快捷鍵調整為統一風格的 <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>E</kbd>。
6. **Compiz 0.9.x GSettings Schemas 完整支援**：
   - 修正 `ghost.xml` 中布林值首字母大寫（`True`/`False` 改為 `true`/`false`），消除 `glib-compile-schemas` 錯誤。
   - 為全部 25 個外掛生成標準 GSettings Schema，並在套件 `postinst`/`postrm` 自動觸發編譯，徹底解決舊版在 CCSM 中點選會卡死或無法儲存設定的問題。
7. **全面移植 6 個經典 3D / 視窗特效至 Compiz 0.9.x C++ 架構**：
   - **Stack Window Switcher (`stackswitch`)**：立體堆疊視窗切換器全面重構適配 Compiz 0.9 渲染管線與文字繪製介面，修復缺少 Compiz 0.9 ABI VTable 無法啟動的問題。
   - **Cube Atlantis (`atlantis`)**：海底水族館 3D 立方體特效，完整適配 `CubeScreenInterface` 介面，在立方體內部進行 3D 魚群游動與動態光影水波渲染。
   - **Cube Snow Globe (`snowglobe`)**：方塊雪球特效，移植適配 `CubeScreenInterface`，修復立方體內部下雪與水底景觀動態效果。
   - **Photowheel (`photo`)**：相片輪盤 3D 檢視外掛，完整重寫為 Compiz 0.9 C++ 物件模型。
   - **Cube 3D Models (`cubemodel`)**：立方體 3D 模型渲染外掛，建立 `GLTexture` 與 `CubeScreenInterface` 相容層，支援在立方體內部載入與渲染 3D OBJ 模型。
   - **Earth (`earth`)**：即時 3D 地球外掛，適配 Compiz 0.9 C++ 架構，移除舊版過時 GLEW 依賴改採標準 OpenGL 2.0+ Core 著色器與 `GLTexture::readImageToTexture`，並連結 `libcurl` 支援即時雲圖下載。
8. **修復 Stack Window Switcher (`stackswitch`) 視窗切換時 Compiz 崩潰問題**：
   - **解決記憶體野指針（Segmentation Fault）**：修正視窗列表加入時陣列前置遞增導致 `mWindows[0]` 未初始化的嚴重 Bug（`mWindows[++mNWindows]` → `mWindows[mNWindows++]`），徹底消除 `compareWindows` 進行 `qsort` 排序時因解引用未初始化指標導致 Compiz 崩潰的問題。
   - **修正焦點查詢副作用**：將 `isStackswitchable()` 中誤用的 `window->focus()`（此方法會主動聚焦視窗）修正為 Compiz 0.9 標準唯讀查詢 `window->isFocussable()`，避免遍歷視窗時產生不必要的焦點變更事件。
   - **增強防禦性程式碼與防除零機制**：在 `layoutThumbs()` 加入 `mNWindows <= 0` 提早返回與 `cols < 1` 保底防護（避免除以零產生浮點異常）；並在 `drawWindowTitle()` 修復縮圖中心 Y 座標計算與投影矩陣安全檢查。

---

## ⌨️ 快捷鍵一覽 (Keybindings Table)

| 外掛名稱 | 功能說明 | 預設快捷鍵 |
| :--- | :--- | :--- |
| **Stack Window Switcher** | 呼叫堆疊視窗切換器 | <kbd>Super</kbd> + <kbd>Tab</kbd> / <kbd>Alt</kbd> + <kbd>Tab</kbd> |
| **Photowheel** | 呼叫相片輪盤視窗切換器 | <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>P</kbd> |
| **Elements** | 切換目前選中的元素特效 | <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>E</kbd> |
| **Elements** | 切換至下一個元素定義 | <kbd>Alt</kbd> + <kbd>→</kbd> |
| **Elements** | 切換至上一個元素定義 | <kbd>Alt</kbd> + <kbd>←</kbd> |
| **Autumn** | 開啟 / 關閉秋天落葉 | <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>A</kbd> |
| **Bubbles** | 開啟 / 關閉桌面氣泡 | <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>B</kbd> |
| **Fireflies** | 開啟 / 關閉夏夜螢火蟲 | <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>F</kbd> |
| **Snow** | 開啟 / 關閉桌面飄雪 | <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>S</kbd> |
| **Stars** | 開啟 / 關閉星空特效 | <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>T</kbd> |
| **Ghost** | 切換符合條件之視窗為幽靈狀態 | <kbd>none</kbd> (可於 CCSM 自訂) |

---

## 🚀 快速安裝 (Quick Installation)

如果您只需要安裝並立即使用：

```bash
# 1. 複製本專案
git clone https://github.com/sophAi/compiz-plugins-experimental-mate.git
cd compiz-plugins-experimental-mate

# 2. 執行一鍵安裝腳本（會自動建立並安裝 deb 套件）
./install.sh
```

或手動建立並安裝：

```bash
./build_deb.sh
sudo dpkg -i compiz-plugins-experimental_0.8.18-4+mint22.3_amd64.deb
```

---

## ⚙️ 在 CCSM 中啟用特效

1. 開啟 **CompizConfig 設置管理器 (CCSM)**（終端機指令：`ccsm`）。
2. 在 **「額外功能 (Extras)」** 分類中：
   - 勾選啟用 **「Elements」**。
   - 依個人喜好勾選想要的效果，例如 **「Autumn」**、**「Snow」**、**「Fireflies」**、**「Bubbles」**、**「Stars」**、**「Atlantis」** 等。
3. 按下快捷鍵（如 <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>A</kbd>）即可隨時開啟或關閉該效果！
4. 若希望某個效果開機自動啟動，請進入該外掛的設定分頁中勾選 **「Autostart」** 即可。

---

## 🔨 自行編譯原始碼 (Build from Source)

本專案內建了 `build_deb.sh` 自動化打包腳本，支援直接從源碼重新編譯 `elements` 家族與 6 個現代化移植外掛 (`stackswitch`, `photo`, `snowglobe`, `atlantis`, `cubemodel`, `earth`)：

```bash
# 從 C++ 源碼重新編譯並打包：
./build_deb.sh --build-source

# 編譯、打包並自動執行安裝：
./build_deb.sh --build-source --install
```

### 編譯依賴需求：
- `cmake` (>= 3.10)
- `g++` (支援 C++14/C++17)
- `compiz-dev` / Compiz 0.9.14 標頭檔
- `libgl-dev`, `libglew-dev`, `libglu1-mesa-dev`
- `libboost-all-dev`
- `xsltproc`
- `libglib2.0-bin` (`glib-compile-schemas`)

---

## 🗑️ 解除安裝 (Uninstallation)

若要完全移除本套件：

```bash
./uninstall.sh
```

或使用 apt：

```bash
sudo apt-get remove --purge compiz-plugins-experimental
```

---

## 📂 目錄結構 (Repository Layout)

```
compiz-plugins-experimental-mate/
├── build_deb.sh              # 一鍵自動編譯與 DEB 打包腳本
├── install.sh                # 快速安裝腳本
├── uninstall.sh              # 快速移除腳本
├── README.md                 # 專案完整說明文件
├── .gitignore                # Git 忽略配置
├── cmake/                    # 內建 Compiz 0.9.x 外掛 CMake 模組與 XSLT 工具
│   ├── CompizPlugin.cmake
│   ├── FindCompiz.cmake
│   └── xslt/compiz_gsettings_schemas.xslt
├── src/                      # 原始程式碼
│   ├── elements/             # Compiz 0.9.x C++ Elements 核心與 5 個子外掛源碼
│   ├── stackswitch/          # Stack Window Switcher 堆疊切換 (Compiz 0.9 C++ 移植版)
│   ├── photo/                # Photowheel 相片輪盤 (Compiz 0.9 C++ 移植版)
│   ├── snowglobe/            # Cube Snow Globe 方塊雪球 (Compiz 0.9 C++ 移植版)
│   ├── atlantis/             # Cube Atlantis 海底水族館 (Compiz 0.9 C++ 移植版)
│   ├── cubemodel/            # Cube 3D Models 模型渲染 (Compiz 0.9 C++ 移植版)
│   ├── earth/                # Earth 即時 3D 地球 (Compiz 0.9 C++ 移植版)
│   └── plugins-classic/      # 歷史經典 C 外掛原始碼
├── prebuilt/                 # 經測試最佳化的 25 個外掛編譯二進位函式庫 (.so)
├── metadata/                 # 25 個外掛之 Compiz 0.9 XML 詮釋資料
├── schemas/                  # 25 個外掛之 GSettings XML 配置規範
├── data/                     # 圖片貼圖 (elements, earth, snowglobe...)
├── icons/                    # 外掛向量圖示 (.svg)
├── po/                       # 多國語言語系翻譯 (.po 與編譯後的 .mo)
└── debian/                   # Debian 套件定義檔 (control, postinst, postrm...)
```

---

## 📄 授權條款 (License)

本專案承襲 Compiz 及 Compiz Reloaded，採 **GNU General Public License v2 (GPL-2.0)** 授權。
各外掛原創作者版權歸其各自所有，詳見 [`debian/copyright`](file:///home/xanadu/rsync_data/geminicli/compiz-plugins-experimental-mate/debian/copyright)。
