#pragma once

#include "../common.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

class Preset {
public:
    std::string name;
    Preset *next_preset = nullptr;
    light::LightCall *light_call;

    explicit Preset(std::string name) : name(name) {}

    void set_light_call(light::LightCall *light_call) {
        this->light_call = light_call;
    }

    void apply() {
        light_call->perform();
    }
};

class PresetGroup {
public:
    std::string name;
    PresetGroup *next_group = nullptr;
    Preset *first_preset = nullptr;
    Preset *last_preset = nullptr;
    Preset *active_preset = nullptr;

    explicit PresetGroup(std::string g_name) : name(g_name) {}

    Preset *make_preset(std::string p_name) {
        auto p = get_preset(p_name);
        if (p == nullptr) {
            p = new Preset(p_name);
            if (first_preset == nullptr) {
                first_preset = last_preset = active_preset = p;
            } else {
                last_preset->next_preset = p;
                last_preset = p;
            }
        }
        return p;
    }

    Preset *get_preset(std::string p_name) {
        for (auto p = first_preset; p != nullptr; p = p->next_preset) {
            if (p->name == p_name) {
                return p;
            }
        }
        return nullptr;
    }
};

class PresetsContainer : public Component {
public:
    explicit PresetsContainer(light::LightState *light) : _light(light) { }

    void dump_config() {
        if (first_group_ == nullptr) {
            return;
        }
        ESP_LOGCONFIG(TAG, "Light Presets:");
        for (auto g = first_group_; g != nullptr; g = g->next_group) {
            ESP_LOGCONFIG(TAG, "  Preset group: %s", g->name.c_str());
            for (auto p = g->first_preset; p != nullptr; p = p->next_preset) {
                ESP_LOGCONFIG(TAG, "    Preset: %s", p->name.c_str());
            }
        }
    }

    void add(std::string g_name, std::string p_name, float r, float g, float b) {
        auto call = make_preset_call_(g_name, p_name);
        call->set_red(r);
        call->set_green(g);
        call->set_blue(b);
    }

    void add(std::string g_name, std::string p_name, float t) {
        auto call = make_preset_call_(g_name, p_name);
        call->set_color_temperature(t);
    }

    void activate_next_group() {
        if (active_group_ == nullptr) {
            ESP_LOGW(TAG, "activate_next_group(): no preset groups defined");
            return;
        }
        active_group_ = active_group_->next_group == nullptr
            ? first_group_ : active_group_->next_group;
        if (active_group_->active_preset == nullptr) {
            ESP_LOGW(TAG, "activate_next_group(): no presets defined for group %s",
                active_group_->name.c_str());
            return;
        }
        ESP_LOGW(TAG, "activate_next_group(): activating %s/%s",
            active_group_->name.c_str(),
            active_group_->active_preset->name.c_str());
        active_group_->active_preset->apply();
    }

    void activate_next_preset() {
        if (active_group_ == nullptr) {
            ESP_LOGW(TAG, "activate_next_preset(): no preset groups defined");
            return;
        }
        auto p = active_group_->active_preset;
        if (p == nullptr) {
            ESP_LOGW(TAG, "activate_next_preset(): no presets defined for group %s",
                active_group_->name.c_str());
            return;
        }
        active_group_->active_preset = p->next_preset == nullptr 
            ? active_group_->first_preset : p->next_preset;
        ESP_LOGW(TAG, "activate_next_preset(): activating %s/%s",
            active_group_->name.c_str(),
            active_group_->active_preset->name.c_str());
        active_group_->active_preset->apply();
    }

    void activate_group(std::string g_name) {
        ESP_LOGI(TAG, "Activate group %s", g_name.c_str());
    }

    void activate_preset(std::string g_name, std::string p_name) {
        ESP_LOGI(TAG, "Activate preset %s/%s", g_name.c_str(), p_name.c_str());
    }

protected:
    light::LightState *_light;
    PresetGroup *first_group_ = nullptr;
    PresetGroup *last_group_ = nullptr;
    PresetGroup *active_group_ = nullptr;

    PresetGroup *make_preset_group_(std::string g_name) {
        auto g = get_group_(g_name);
        if (g == nullptr) {
            g = new PresetGroup(g_name);
            if (first_group_ == nullptr) {
                first_group_ = last_group_ = active_group_ = g;
            } else {
                last_group_->next_group = g;
                last_group_ = g;
            }
        }
        return g;
    }

    PresetGroup *get_group_(std::string g_name) {
        for (auto g = first_group_; g != nullptr; g = g->next_group) {
            if (g->name == g_name) {
                return g;
            }
        }
        return nullptr;
    }


    light::LightCall *make_preset_call_(std::string g_name, std::string p_name) {
        auto g = make_preset_group_(g_name);
        auto p = g->make_preset(p_name);
        auto c = new light::LightCall(_light);
        p->set_light_call(c);
        return c;
    }
};

} // namespace bslamp2
} // namespace xiaomi
} // namespace esphome
