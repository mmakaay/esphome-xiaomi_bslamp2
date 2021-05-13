#pragma once

#include "../common.h"
#include "esphome/core/optional.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

class Preset : public Component {
 public:
  std::string group_name;
  std::string name;
  Preset *next_preset = nullptr;

  explicit Preset(light::LightState *light, std::string group_name, std::string name)
      : group_name(group_name), name(name), light_state_(light) {}

  void set_transition_length(uint32_t t) { transition_length_ = t; }
  void set_brightness(float t) { brightness_ = t; }
  void set_red(float t) { red_ = t; }
  void set_green(float t) { green_ = t; }
  void set_blue(float t) { blue_ = t; }
  void set_color_temperature(float t) { color_temperature_ = t; }
  void set_effect(const std::string &effect) { effect_ = effect; }

  void apply() {
    ESP_LOGI(TAG, "Activating light preset: %s/%s", group_name.c_str(), name.c_str());
    auto call = light_state_->make_call();
    call.set_state(true);
    if (transition_length_.has_value())
      call.set_transition_length(*transition_length_);
    if (brightness_.has_value())
      call.set_brightness(*brightness_);
    if (red_.has_value())
      call.set_red(*red_);
    if (green_.has_value())
      call.set_green(*green_);
    if (blue_.has_value())
      call.set_blue(*blue_);
    if (color_temperature_.has_value())
      call.set_color_temperature(*color_temperature_);
    if (effect_.has_value())
      call.set_effect(*effect_);
    call.perform();
  }

 protected:
  light::LightState *light_state_;
  optional<uint32_t> transition_length_;
  optional<float> brightness_;
  optional<float> red_;
  optional<float> green_;
  optional<float> blue_;
  optional<float> color_temperature_;
  optional<std::string> effect_;
};

class PresetGroup {
 public:
  std::string name;
  PresetGroup *next_group = nullptr;
  Preset *first_preset = nullptr;
  Preset *last_preset = nullptr;
  Preset *active_preset = nullptr;

  explicit PresetGroup(std::string g_name) : name(g_name) {}

  void add_preset(Preset *p) {
    if (first_preset == nullptr) {
      first_preset = last_preset = active_preset = p;
    } else {
      last_preset->next_preset = p;
      last_preset = p;
    }
  }

  Preset *get_preset(std::string p_name) {
    for (auto p = first_preset; p != nullptr; p = p->next_preset)
      if (p->name == p_name)
        return p;
    return nullptr;
  }
};

class PresetsContainer : public Component {
 public:
  PresetGroup *first_group = nullptr;
  PresetGroup *last_group = nullptr;
  PresetGroup *active_group = nullptr;

  void dump_config() {
    if (first_group != nullptr) {
      ESP_LOGCONFIG(TAG, "Light Presets:");
      for (auto g = first_group; g != nullptr; g = g->next_group) {
        ESP_LOGCONFIG(TAG, "  Preset group: %s", g->name.c_str());
        for (auto p = g->first_preset; p != nullptr; p = p->next_preset)
          ESP_LOGCONFIG(TAG, "    Preset: %s", p->name.c_str());
      }
    }
  }

  void add_preset(Preset *preset) {
    auto g = make_preset_group_(preset->group_name);
    g->add_preset(preset);
  }

  PresetGroup *get_group(std::string g_name) {
    for (auto g = first_group; g != nullptr; g = g->next_group)
      if (g->name == g_name)
        return g;
    return nullptr;
  }

  void activate_next_group() {
    if (active_group == nullptr) {
      ESP_LOGW(TAG, "activate_next_group(): no preset groups defined");
      return;
    }
    active_group = active_group->next_group == nullptr ? first_group : active_group->next_group;
    if (active_group->active_preset == nullptr) {
      ESP_LOGW(TAG, "activate_next_group(): no presets defined for group %s", active_group->name.c_str());
      return;
    }
    active_group->active_preset->apply();
  }

  void activate_next_preset() {
    if (active_group == nullptr) {
      ESP_LOGW(TAG, "activate_next_preset(): no preset groups defined");
      return;
    }
    auto p = active_group->active_preset;
    if (p == nullptr) {
      ESP_LOGW(TAG, "activate_next_preset(): no presets defined for group %s", active_group->name.c_str());
      return;
    }
    active_group->active_preset = p->next_preset == nullptr ? active_group->first_preset : p->next_preset;
    active_group->active_preset->apply();
  }

  void activate_group(std::string g_name) {
    auto g = get_group(g_name);
    if (g == nullptr) {
      ESP_LOGE(TAG, "activate_group(%s): preset group does not exist", g_name.c_str());
      return;
    }
    auto p = g->active_preset;
    if (p == nullptr) {
      ESP_LOGW(TAG, "activate_group(%s): no presets defined for group", g_name.c_str());
      return;
    }
    p->apply();
  }

  void activate_preset(std::string g_name, std::string p_name) {
    auto g = get_group(g_name);
    if (g == nullptr) {
      ESP_LOGE(TAG, "activate_preset(%s, %s): preset group '%s' does not exist", g_name.c_str(), p_name.c_str(),
               g_name.c_str());
      return;
    }
    auto p = g->get_preset(p_name);
    if (p == nullptr) {
      ESP_LOGE(TAG, "activate_preset(%s, %s): preset '%s' does not exist in group '%s'", g_name.c_str(), p_name.c_str(),
               p_name.c_str(), g->name.c_str());
      return;
    }
    p->apply();
  }

 protected:
  PresetGroup *make_preset_group_(std::string g_name) {
    auto g = get_group(g_name);
    if (g == nullptr) {
      g = new PresetGroup(g_name);
      if (first_group == nullptr) {
        first_group = last_group = active_group = g;
      } else {
        last_group->next_group = g;
        last_group = g;
      }
    }
    return g;
  }
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
