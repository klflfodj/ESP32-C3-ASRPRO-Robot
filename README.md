# ESP32 语音交互桌面机器人

基于 ESP32 / ESP32-C3 的语音交互桌面小机器人。OLED 显示动态眼睛表情，通过 ASR 语音模块接收语音指令，可查询时间、日期、星期和天气，并切换到数字信息页。

## 功能

- 18 种动态眼睛表情，支持随机眨眼、视线移动和表情切换
- 语音控制：唤醒、时间、天气、日期、星期、睡眠
- SSD1306 128x64 OLED 信息页，显示日期、星期、时间、天气和温度
- 天气通过 HTTP API 获取，默认城市为北京，城市代码 `101010100`
- FreeRTOS 多任务：时钟任务、OLED 刷新任务、ASR 指令任务
- 使用 U8g2 硬件 I2C 驱动，整帧缓冲刷新，表情显示更流畅

## 硬件清单

| 模块 | 说明 |
|---|---|
| ESP32-C3 DevKitM-1 | 默认开发板，也可换成普通 ESP32 |
| ASR Pro 语音模块 | UART 通信，115200 8N1 |
| SSD1306 OLED | 128x64，I2C 接口 |

## 接线

| OLED | ESP32-C3 | 普通 ESP32 |
|---|---|---|
| VCC | 3V3 | 3V3 |
| GND | GND | GND |
| SDA | GPIO8 | GPIO21 |
| SCL | GPIO9 | GPIO22 |

| ASR 模块 | ESP32 |
|---|---|
| ASR TX | GPIO5（ESP32 RX） |
| ASR RX | GPIO4（ESP32 TX） |
| GND | GND |

注意：ASR 串口必须使用 3.3V 电平，不要直接接 5V 的串口模块。

## 软件环境

- VS Code + PlatformIO
- Platform：`espressif32`
- Framework：`arduino`
- 依赖库：`ArduinoJson`、`U8g2`

当前 [platformio.ini](platformio.ini) 默认配置为：

```ini
[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200
lib_deps =
    bblanchon/ArduinoJson
    olikraus/U8g2
```

如果使用普通 ESP32 开发板，把 `board` 改为 `esp32dev`：

```ini
board = esp32dev
```

## 目录结构

```text
src/
├── ross.ino                  # 入口，初始化 WiFi、OLED、RTOS
├── RTOS.cpp / RTOS.h         # FreeRTOS 任务和语音指令处理
├── OLED.cpp / OLED.h         # U8g2 OLED 驱动封装
├── OLED_DrawWeatherIcon.cpp  # 天气图标和数字信息页
├── WiFie.cpp / WiFie.h       # WiFi 初始化
└── EyeExpression.cpp / .h    # 动态眼睛表情模块
```

## 快速开始

1. 使用 VS Code 打开本工程。
2. 修改 [WiFie.cpp](src/WiFie.cpp) 里的 WiFi 名称和密码。
3. 根据实际开发板选择 `esp32-c3-devkitm-1` 或 `esp32dev`。
4. 编译上传：

```bash
pio run -t upload
```

5. 打开串口监视器：

```bash
pio device monitor -b 115200
```

## 语音指令

| 指令 | 效果 |
|---|---|
| `WAKEUP` | 切换到唤醒状态，并刷新天气 |
| `TIME` | 显示当前时间，并向 ASR 模块回传小时、分钟 |
| `WEATHER` | 显示天气和温度 |
| `DATE` | 显示日期 |
| `WEEK` | 显示星期 |
| `SLEEP` | 回到表情待机页 |

指令通过 `ASRSerial`（UART1，GPIO4/GPIO5）接收，USB `Serial` 主要用于调试输出。

如果暂时没有语音模块，可以用 USB-TTL 接到 GPIO4/GPIO5 发送指令，或者临时在 `TaskASR` 里同时监听 `Serial`。

## 设计说明

- 天气只在收到 `WAKEUP` 时查询，没有询问天气时一直显示眼睛表情。
- 时间使用 `ntp.aliyun.com` 校时，时区为 UTC+8。
- 天气 API 当前写死北京城市代码 `101010100`，需要改城市时修改 `RTOS.cpp` 里的 URL。
- OLED 刷新任务在信息页每秒刷新一次，表情页约 25 FPS 刷新。
- 使用互斥锁保护 `timeinfo`，避免多个任务同时访问时间数据。

## 注意事项

- 上传前一定要修改 WiFi 配置，当前默认是占位符，不修改会卡在 WiFi 连接。
- 普通 ESP32 与 ESP32-C3 的默认 I2C 引脚不同，接线前先确认板型。
- 天气接口是免费公开接口，可能不稳定，生产环境建议换成自己的服务。
- 当前固件 Flash 占用约 88%，继续增加大字库、图片或音频功能时要注意容量。

## 学习点

这是一个比较适合嵌入式入门的完整项目，可以学到：

- ESP32 Arduino 开发流程
- FreeRTOS 多任务和互斥锁
- I2C 驱动 OLED 显示
- UART 与语音模块通信
- U8g2 图形库和自绘帧缓冲
- HTTP 请求和 JSON 解析
- 状态机思路：待机、唤醒、信息页

## Acknowledgements

- 动态眼睛表情模块（`EyeExpression.cpp/.h`）的矢量绘制算法，
  移植改编自 GitHub 上的开源项目 `eye_all_in_one_esp32`（原作者在仓库历史中，
  具体用户名暂未找回，后续补全）。
  本项目将其从 U8g2 绘图调用改写为原生 128x64 帧缓冲操作，
  并整合进 FreeRTOS 多任务框架。
