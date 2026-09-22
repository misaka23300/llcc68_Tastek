import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text, uart
from esphome.const import CONF_ID, CONF_MODE, CONF_UART_ID

CODEOWNERS = ["@your_github"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["text"]

tastek_lora_ns = cg.esphome_ns.namespace("tastek_lora")
TastekLora = tastek_lora_ns.class_(
    "TastekLora", cg.Component, uart.UARTDevice, text.Text
)

CONF_WORKMODE = "workmode"
CONF_ADDRESS = "address"
CONF_TX_FREQ = "tx_freq"
CONF_RX_FREQ = "rx_freq"
CONF_AIRSPEED = "airspeed"
CONF_POWER = "power"
CONF_DELTA_TIME = "delta_time"

WORKMODE_OPTIONS = {
    "transparent": 0,    # 透传模式
    "point_to_point": 1, # 定点模式
    "master_slave": 2,   # 主从模式
    "relay": 3,          # 自组网模式
}

AIRSPEED_OPTIONS = [300, 1200, 2400, 4800, 9600, 19200, 38400, 76800]

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TastekLora),
            cv.Optional(CONF_WORKMODE, default="transparent"): cv.enum(
                WORKMODE_OPTIONS, lower=True
            ),
            # 定点模式地址：0-65535，0xFFFF 为广播
            cv.Optional(CONF_ADDRESS): cv.int_range(0, 65535),
            # 发射频率 0-100，对应 410-510MHz
            cv.Optional(CONF_TX_FREQ): cv.int_range(0, 100),
            # 接收频率 0-100，对应 410-510MHz
            cv.Optional(CONF_RX_FREQ): cv.int_range(0, 100),
            # 空中速率
            cv.Optional(CONF_AIRSPEED): cv.one_of(*AIRSPEED_OPTIONS, int=True),
            # 发射功率，仅 22dBm 系列支持，10-22
            cv.Optional(CONF_POWER): cv.int_range(10, 22),
            # 空闲分帧超时(ms)：除连续字节流多长时间算一条独立数据
            cv.Optional(CONF_DELTA_TIME, default=50): cv.int_range(20, 1000),
        }
    )
    .extend(text.TEXT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    await text.register_text_control(var, config)

    cg.add(var.set_delta_time(config[CONF_DELTA_TIME]))

    # 只要配置了可选项，就让组件在启动时通过 AT 指令初始化模块
    commands = []
    if config[CONF_WORKMODE] != "transparent":
        commands.append(f"AT+WORKMODE={WORKMODE_OPTIONS[config[CONF_WORKMODE]]}")
    if CONF_ADDRESS in config:
        commands.append(f"AT+ADDRESS={config[CONF_ADDRESS]}")
    if CONF_TX_FREQ in config:
        commands.append(f"AT+TXFREQ={config[CONF_TX_FREQ]}")
    if CONF_RX_FREQ in config:
        commands.append(f"AT+RXFREQ={config[CONF_RX_FREQ]}")
    if CONF_AIRSPEED in config:
        commands.append(f"AT+AIRSPEED={config[CONF_AIRSPEED]}")
    if CONF_POWER in config:
        commands.append(f"AT+POWER={config[CONF_POWER]}")

    for cmd in commands:
        cg.add(var.add_init_command(cmd))