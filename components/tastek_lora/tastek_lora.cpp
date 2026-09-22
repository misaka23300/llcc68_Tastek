#include "tastek_lora.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tastek_lora {

static const char *const TAG = "tastek_lora";

void TastekLora::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Tastek LoRa (LLCC68) component");
  this->buffer_.reserve(256);
  this->last_rx_ = millis();
  this->apply_init_config();
}

void TastekLora::apply_init_config() {
  if (this->init_commands_.empty())
    return;

  ESP_LOGI(TAG, "Applying %d AT init command(s)", this->init_commands_.size());

  // 进入配置模式
  this->write_str_ln("+++");
  delay(500);

  for (const auto &cmd : this->init_commands_) {
    ESP_LOGD(TAG, "> %s", cmd.c_str());
    this->write_str_ln(cmd);
    delay(100);
  }

  // 保存配置并退出配置模式，回到透传
  this->write_str_ln("AT&W");
  delay(300);
  this->write_str_ln("ATO");
  delay(200);
}

void TastekLora::loop() {
  char c;
  while (this->available()) {
    this->read_byte(&c);
    this->buffer_.push_back(c);
    this->last_rx_ = millis();
  }

  // 空闲超时判定一帧数据结束
  if (!this->buffer_.empty() &&
      (millis() - this->last_rx_) > this->delta_time_ms_) {
    this->publish_state(this->buffer_);
    this->buffer_.clear();
  }
}

void TastekLora::control(const std::string &value) {
  ESP_LOGD(TAG, "TX via LoRa: %s", value.c_str());
  this->write_str(value.c_str());
}

text::TextTraits TastekLora::get_traits() {
  auto traits = text::TextTraits();
  traits.set_mode(text::TEXT_MODE_TEXT);
  return traits;
}

void TastekLora::send_command(const std::string &command) {
  this->write_str_ln(command);
}

void TastekLora::write_str_ln(const std::string &data) {
  this->write_str(data.c_str());
  this->write_str("\r\n");
}

void TastekLora::dump_config() {
  ESP_LOGCONFIG(TAG, "Tastek LoRa:");
  ESP_LOGCONFIG(TAG, "  Delta time: %u ms", this->delta_time_ms_);
  for (const auto &cmd : this->init_commands_) {
    ESP_LOGCONFIG(TAG, "  Init: %s", cmd.c_str());
  }
}

}  // namespace tastek_lora
}  // namespace esphome