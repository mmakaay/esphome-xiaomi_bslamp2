#pragma once

#include "../common.h"

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

class Preset {
public:
    std::string name;
    Preset *next_preset = nullptr;
    std::string group_name;
    light::LightCall *light_call;

    explicit Preset(std::string name, std::string group_name) :
        name(name), group_name(group_name) {}

    void set_light_call(light::LightCall *light_call) {
        this->light_call = light_call;
    }

    void apply() {
        ESP_LOGI(TAG, "Activating light preset: %s/%s",
            group_name.c_str(), name.c_str());
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
            p = new Preset(p_name, this->name);
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
    PresetGroup *first_group = nullptr;
    PresetGroup *last_group = nullptr;
    PresetGroup *active_group = nullptr;

    explicit PresetsContainer(light::LightState *light) : _light(light) { }

    void dump_config() {
        if (first_group == nullptr) {
            return;
        }
        ESP_LOGCONFIG(TAG, "Light Presets:");
        for (auto g = first_group; g != nullptr; g = g->next_group) {
            ESP_LOGCONFIG(TAG, "  Preset group: %s", g->name.c_str());
            for (auto p = g->first_preset; p != nullptr; p = p->next_preset) {
                ESP_LOGCONFIG(TAG, "    Preset: %s", p->name.c_str());
            }
        }
    }

    void add(std::string g_name, std::string p_name, float r, float g, float b) {
        auto call = make_preset_call_(g_name, p_name);
        call->set_rgb(r, g, b);
    }

    void add(std::string g_name, std::string p_name, float t) {
        auto call = make_preset_call_(g_name, p_name);
        call->set_color_temperature(t);
    }

    PresetGroup *get_group(std::string g_name) {
        for (auto g = first_group; g != nullptr; g = g->next_group) {
            if (g->name == g_name) {
                return g;
            }
        }
        return nullptr;
    }

    void activate_next_group() {
        if (active_group == nullptr) {
            ESP_LOGW(TAG, "activate_next_group(): no preset groups defined");
            return;
        }
        active_group = active_group->next_group == nullptr
            ? first_group : active_group->next_group;
        if (active_group->active_preset == nullptr) {
            ESP_LOGW(TAG, "activate_next_group(): no presets defined for group %s",
                active_group->name.c_str());
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
            ESP_LOGW(TAG, "activate_next_preset(): no presets defined for group %s",
                active_group->name.c_str());
            return;
        }
        active_group->active_preset = p->next_preset == nullptr 
            ? active_group->first_preset : p->next_preset;
        active_group->active_preset->apply();
    }

    void activate_group(std::string g_name) {
        auto g = get_group(g_name);
        if (g == nullptr) {
            ESP_LOGE(TAG, "activate_group(%s): preset group does not exist",
                g_name.c_str());
            return;
        }
        auto p = g->active_preset;
        if (p == nullptr) {
            ESP_LOGW(TAG, "activate_group(%s): no presets defined for group",
                g_name.c_str());
            return;
        }
        p->apply();
    }

    void activate_preset(std::string g_name, std::string p_name) {
        auto g = get_group(g_name);
        if (g == nullptr) {
            ESP_LOGE(TAG, "activate_preset(%s, %s): preset group %s does not exist",
                g_name.c_str(), p_name.c_str(), g_name.c_str());
            return;
        }
        auto p = g->get_preset(p_name);
        if (p == nullptr) {
            ESP_LOGE(TAG, "activate_preset(%s, %s): preset %s does not exist in group %s",
                g_name.c_str(), p_name.c_str(), p_name.c_str(), g_name.c_str());
            return;
        }
        p->apply();
    }

protected:
    light::LightState *_light;

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
