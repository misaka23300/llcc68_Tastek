#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/text/text.h"

namespace esphome {
namespace tastek_lora {

/**
 * 塔石物联网 LLCC68 (L32 系列) LoRa 模组 ESPHome 组件。
 *
 * 运行在透传数据模式：串口收到的字节经 LoRa 空中转发；LoRa 收到的字节
 * 从串口输出到本组件，按"空闲超时"分帧后作为一条文本发布到 Text 实体。
 * 向 Text 实体写入内容即触发一次 LoRa 发送（透传原文）。
 */
class TastekLora : public Component, public uart::UARTDevice, public text::Text {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_delta_time(uint32_t delta_time_ms) { this->delta_time_ms_ = delta_time_ms; }

  /// 收集一条启动时的 AT 配置指令，在进入配置模式后依次发送
  void add_init_command(const std::string &command) { this->init_commands_.push_back(command); }

 protected:
  text::TextTraits get_traits() override;
  void control(const std::string &value) override;

  /// 发送一行 AT 指令并等待模块处理
  void send_command(const std::string &command);

  /// 若配置了任何 AT 指令，则进入配置模式 → 发送 → 保存 → 退出配置模式
  void apply_init_config();

  void write_str_ln(const std::string &data);

  uint32_t delta_time_ms_{50};
  std::vector<std::string> init_commands_;
  std::string buffer_;
  uint32_t last_rx_{0};
};

}  // namespace tastek_lora
}  // namespace esphome