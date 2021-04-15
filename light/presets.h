#pragma once

#include <map>
#include <vector>
#include "../common.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

using Order = std::vector<std::string>;
using Presets = std::map<std::string, light::LightCall *>;
using PresetOrder = std::map<std::string, Order>;
using PresetGroups = std::map<std::string, Presets>;
using PresetGroupOrder = Order;

class PresetsContainer : public Component {
public:
    explicit PresetsContainer(light::LightState *light) : _light(light) { }

    void dump_config() {
        if (map_.empty()) {
            return;
        }
        ESP_LOGCONFIG(TAG, "Light Presets:");
        for (auto group : group_order_) {
            ESP_LOGCONFIG(TAG, "  Preset group: %s", group.c_str());
            for (auto name : preset_order_[group]) {
                ESP_LOGCONFIG(TAG, "    Preset: %s", name.c_str());
            }
        }
    }

    void add(std::string group, std::string name, float red, float green, float blue) {
        auto call = make_preset_slot_(group, name);
        call->set_red(red);
        call->set_green(green);
        call->set_blue(blue);
    }

    void add(std::string group, std::string name, float color_temperature) {
        auto call = make_preset_slot_(group, name);
        call->set_color_temperature(color_temperature);
    }

protected:
    light::LightState *_light;
    PresetGroups map_;
    PresetGroupOrder group_order_;
    PresetOrder preset_order_;

    light::LightCall *make_preset_slot_(std::string group, std::string name) {
        // Check if the group already exists. If not, then create it.
        if (map_.find(group) == map_.end()) {
            map_[group] = Presets();
            group_order_.push_back(group);
            preset_order_[group] = Order();
        }
        if (map_[group].find(name) == map_[group].end()) {
            map_[group][name] = new light::LightCall(_light);
            preset_order_[group].push_back(name);
        }

        return map_[group][name];
    }
};

} // namespace bslamp2
} // namespace xiaomi
} // namespace esphome
