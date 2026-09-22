# ESPHome 塔石 LLCC68 (L32) LoRa 组件

针对塔石物联网 LLCC68 (L32-433) LoRa 模组的 ESPHome **无线透传收发**组件。
收到的 LoRa 报文在 Home Assistant 中以 **Text 实体**显示，向该实体写入内容即通过 LoRa 发射。

## 目录结构

```
esp_tastek_lora/
├── components/
│   └── tastek_lora/
│       ├── __init__.py      # YAML 校验 + 生成 C++ 代码
│       ├── tastek_lora.h
│       └── tastek_lora.cpp
└── example.yaml
```

## 接线

模组 `TXD` → 开发板 `rx_pin`（receive），模组 `RXD` → 开发板 `tx_pin`，共地与供电。
串口波特率必须与模组当前 `AT+UART` 配置一致（模组默认 **9600, 无校验**）。

## 用法

```yaml
external_components:
  - source: { type: local, path: components }

uart:
  - id: lora_uart
    tx_pin: GPIO16
    rx_pin: GPIO17
    baud_rate: 9600

text:
  - platform: tastek_lora
    uart_id: lora_uart
    id: lora_rx
    name: "LoRa Received"
    delta_time: 50
```

## 配置项

| 键 | 类型 | 说明 |
|----|------|------|
| `uart_id` | id | 必填，绑定的 UART |
| `workmode` | enum | `transparent` / `point_to_point` / `master_slave` / `relay`，默认 `transparent` |
| `address` | int | 定点模式地址 0-65535（0xFFFF 为广播） |
| `tx_freq` | int | 发射频率 0-100 → 410-510MHz |
| `rx_freq` | int | 接收频率 0-100 → 410-510MHz |
| `airspeed` | int | 空中速率 300~76800 |
| `power` | int | 发射功率 10-22（仅22dBm系列） |
| `delta_time` | int(ms) | 空闲分帧超时，默认 50 |

## 工作机制

- **纯透传**：除 `delta_time` 外不做任何 AT 下发，串口 ↔ LoRa 直接转发，收发双方同频即可互传。
- **需要初始化**：设置了上面任一参数时，上电后组件自动 `+++` 进配置模式 → 依次下发 AT → `AT&W` 保存 → `ATO` 退出，回到透传。
- **分帧**：LoRa 报文在空口上无分隔符，组件以"空闲 `delta_time` ms 无新字节"判定一条报文结束，作为一条文本发布。
- **发送**：通过 `id(lora_rx).publish_state(...)` 或 HA 中写入该 text 即发射（透传原文）。

## 注意

- 空中速率两端模组需一致；`point_to_point` 模式下收发双方 `address`/`tx_freq`/`rx_freq` 需对应。
- 本组件默认不改动模块波特率，改波特率请先用厂商 AT 工具单独配置。
- 若模组已由别处保存过配置，组件初始化指令会覆盖对应项。