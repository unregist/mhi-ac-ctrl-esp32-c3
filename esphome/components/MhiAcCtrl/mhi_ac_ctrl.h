#include "MHI-AC-Ctrl-core.h"
#include "esphome/core/gpio.h"

#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif


using namespace esphome;
using namespace esphome::climate;
using namespace esphome::sensor;
#ifdef USE_SWITCH
using namespace esphome::switch_;
#endif

static const char* TAG = "mhi_ac_ctrl";

class MhiFrameErrors : public Sensor {
public:
    MhiFrameErrors() {
        this->publish_state(mhi_ac::frame_errors_get());
    }

    void loop() {
        if(mhi_ac::frame_errors_get() != this->get_raw_state()) {
            this->publish_state(mhi_ac::frame_errors_get());
        }
    }
};

class MhiTotalEnergy : public Sensor {
public:
    MhiTotalEnergy() {
        this->total_energy_ = mhi_ac::energy.total_energy.load();
        publish_total();
    }

    void loop() {
        uint64_t new_energy = mhi_ac::energy.total_energy.load();
        if(new_energy != total_energy_) {
            total_energy_ = new_energy;
            publish_total();
        }
    }

    void publish_total() {
        this->publish_state(total_energy_ * (14.0f/(51000000.0f*3600)));
    }

protected:
    uint64_t total_energy_;
};

class MhiPower : public Sensor {
public:
    MhiPower() {
        if(mhi_ac::active_mode_get()) {
            last_power = mhi_ac::energy.get_power();
            this->publish_power();
        }
    }

    void loop() {
        if(mhi_ac::active_mode_get()) {
            uint32_t new_power = mhi_ac::energy.get_power();
            if(new_power != last_power || std::isnan(get_raw_state())) {
                last_power = new_power;
                publish_power();
            }
        } else if(!std::isnan(get_raw_state()) && !mhi_ac::active_mode_get()) {
            this->publish_state(std::numeric_limits<float>::quiet_NaN());
        }
    }

    void publish_power() {
        this->publish_state(last_power * (14.0f/51.0f));
    }

protected:
    uint32_t last_power;
};

#ifdef USE_SWITCH
class MhiActiveMode : public switch_::Switch {
protected:
    virtual void write_state(bool state) {
        mhi_ac::active_mode_set(state);
        this->publish_state(state);
    }
};
#endif

#ifdef USE_SELECT
class MhiVanesUD : public select::Select {
public:
  // Select options must match the ones in the select.py Python code
  virtual void control(const std::string &value) {
    if(value == "3D Auto") {
      mhi_ac::spi_state.three_d_auto_set(true);
    } else {
      mhi_ac::spi_state.three_d_auto_set(false);
      if(value == "Swing") {
        mhi_ac::spi_state.vanes_updown_set(mhi_ac::ACVanesUD::Swing);
      } else if(value == "Up") {
        mhi_ac::spi_state.vanes_updown_set(mhi_ac::ACVanesUD::Up);
      } else if(value == "Up/Center") {
        mhi_ac::spi_state.vanes_updown_set(mhi_ac::ACVanesUD::UpCenter);
      } else if(value == "Center/Down") {
        mhi_ac::spi_state.vanes_updown_set(mhi_ac::ACVanesUD::CenterDown);
      } else if(value == "Down") {
        mhi_ac::spi_state.vanes_updown_set(mhi_ac::ACVanesUD::Down);
      } else {
        ESP_LOGW(TAG, "Unknown vanes_ud mode received: %s", value.c_str());
      }
    }
  }

  void loop() {
    if(!mhi_ac::spi_state.three_d_auto_changed() && !mhi_ac::spi_state.vanes_updown_changed() && has_state()) {
      return;
    }
    if(mhi_ac::spi_state.three_d_auto_get()) {
      publish_state("3D Auto");
    } else {
      switch (mhi_ac::spi_state.vanes_updown_get()) {
        case mhi_ac::ACVanesUD::Swing:
          publish_state("Swing");
          break;
        case mhi_ac::ACVanesUD::Up:
          publish_state("Up");
          break;
        case mhi_ac::ACVanesUD::UpCenter:
          publish_state("Up/Center");
          break;
        case mhi_ac::ACVanesUD::CenterDown:
          publish_state("Center/Down");
          break;
        case mhi_ac::ACVanesUD::Down:
          publish_state("Down");
          break;
        case mhi_ac::ACVanesUD::SeeIRRemote:
          publish_state("See IR Remote");
          break;
      }
    }
  }
};
class MhiVanesLR : public select::Select {
public:
  // Select options must match the ones in the select.py Python code
  virtual void control(const std::string &value) {
    if(value == "3D Auto") {
      mhi_ac::spi_state.three_d_auto_set(true);
    } else {
      mhi_ac::spi_state.three_d_auto_set(false);
      if(value == "Left") {
        mhi_ac::spi_state.vanes_leftright_set(mhi_ac::ACVanesLR::Left);
      } else if(value == "Left/Center") {
        mhi_ac::spi_state.vanes_leftright_set(mhi_ac::ACVanesLR::LeftCenter);
      } else if(value == "Center") {
        mhi_ac::spi_state.vanes_leftright_set(mhi_ac::ACVanesLR::Center);
      } else if(value == "Center/Right") {
        mhi_ac::spi_state.vanes_leftright_set(mhi_ac::ACVanesLR::CenterRight);
      } else if(value == "Right") {
        mhi_ac::spi_state.vanes_leftright_set(mhi_ac::ACVanesLR::Right);
      } else if(value == "Wide") {
        mhi_ac::spi_state.vanes_leftright_set(mhi_ac::ACVanesLR::Wide);
      } else if(value == "Spot") {
        mhi_ac::spi_state.vanes_leftright_set(mhi_ac::ACVanesLR::Spot);
      } else if(value == "Swing") {
        mhi_ac::spi_state.vanes_leftright_set(mhi_ac::ACVanesLR::Swing);
      } else {
        ESP_LOGW(TAG, "Unknown vanes_lr mode received: %s", value.c_str());
      }
    }
  }

  void loop() {
    if(!mhi_ac::spi_state.three_d_auto_changed() && !mhi_ac::spi_state.vanes_leftright_changed() && has_state()) {
      return;
    }
    if(mhi_ac::spi_state.three_d_auto_get()) {
      publish_state("3D Auto");
    } else {
      switch (mhi_ac::spi_state.vanes_leftright_get()) {
        case mhi_ac::ACVanesLR::Left:
          publish_state("Left");
          break;
        case mhi_ac::ACVanesLR::LeftCenter:
          publish_state("Left/Center");
          break;
        case mhi_ac::ACVanesLR::Center:
          publish_state("Center");
          break;
        case mhi_ac::ACVanesLR::CenterRight:
          publish_state("Center/Right");
          break;
        case mhi_ac::ACVanesLR::Right:
          publish_state("Right");
          break;
        case mhi_ac::ACVanesLR::Wide:
          publish_state("Wide");
          break;
        case mhi_ac::ACVanesLR::Spot:
          publish_state("Spot");
          break;
        case mhi_ac::ACVanesLR::Swing:
          publish_state("Swing");
          break;
      }
    }
  }
};
#endif

class MhiAcCtrl : public climate::Climate,
                  public Component {
public:
  MhiAcCtrl(const mhi_ac::Config &config) {
    this->ac_config_ = config;
  }

    void setup() override
    {
        //current_power.set_icon("mdi:current-ac");
        //current_power.set_unit_of_measurement("A");
        //current_power.set_accuracy_decimals(2);

        mhi_ac::init(this->ac_config_);
    }

    void loop() override
    {
        bool publish_self_state = false;
        if(!mhi_ac::spi_state.update_snapshot(100)) {
          this->status_set_warning("No MHI AC communication");
          return;
        }
        this->status_clear_warning();

        if(total_energy_sensor_)
            total_energy_sensor_->loop();
        if(power_sensor_)
            power_sensor_->loop();
        if(frame_errors_sensor_)
            frame_errors_sensor_->loop();
#ifdef USE_SELECT
        if(vanes_ud_select_)
            vanes_ud_select_->loop();
        if(vanes_lr_select_)
            vanes_lr_select_->loop();
#endif
        bool first_time = std::isnan(this->target_temperature);

        if(mhi_ac::spi_state.target_temperature_changed() || first_time) {
          publish_self_state = true;
          this->target_temperature = mhi_ac::spi_state.target_temperature_get();
        }

        if(mhi_ac::spi_state.power_changed()
                || mhi_ac::spi_state.mode_changed()
                || mhi_ac::spi_state.compressor_changed()
                || mhi_ac::spi_state.heatcool_changed()
                || first_time) {
            publish_self_state = true;

          if(mhi_ac::spi_state.power_get() == mhi_ac::ACPower::power_off) {
            this->mode = climate::CLIMATE_MODE_OFF;
            this->action = climate::CLIMATE_ACTION_OFF;
          } else {
            if(!mhi_ac::spi_state.compressor_get()) {
                this->action = climate::CLIMATE_ACTION_IDLE;
            }

            switch(mhi_ac::spi_state.mode_get()) {
              case mhi_ac::ACMode::mode_cool:
                this->mode = climate::CLIMATE_MODE_COOL;
                if(mhi_ac::spi_state.compressor_get())
                    this->action = climate::CLIMATE_ACTION_COOLING;
                break;
              case mhi_ac::ACMode::mode_heat:
                this->mode = climate::CLIMATE_MODE_HEAT;
                if(mhi_ac::spi_state.compressor_get())
                    this->action = climate::CLIMATE_ACTION_HEATING;
                break;
              case mhi_ac::ACMode::mode_dry:
                this->mode = climate::CLIMATE_MODE_DRY;
                if(mhi_ac::spi_state.compressor_get())
                    this->action = climate::CLIMATE_ACTION_DRYING;
                break;
              case mhi_ac::ACMode::mode_fan:
                this->mode = climate::CLIMATE_MODE_FAN_ONLY;
                this->action = climate::CLIMATE_ACTION_FAN;
                break;
              case mhi_ac::ACMode::mode_auto:
                this->mode = climate::CLIMATE_MODE_HEAT_COOL;
                if(mhi_ac::spi_state.compressor_get()) {
                    if(mhi_ac::spi_state.heatcool_get())
                        this->action = climate::CLIMATE_ACTION_HEATING;
                    else
                        this->action = climate::CLIMATE_ACTION_COOLING;
                }
                break;
              default:
                break;
            }
          }
        }

        if(mhi_ac::spi_state.fan_changed() || first_time) {
          publish_self_state = true;
          switch(mhi_ac::spi_state.fan_get()) {
            case mhi_ac::ACFan::speed_1:
              this->set_custom_fan_mode_(custom_fan_ultra_low);
              break;
            case mhi_ac::ACFan::speed_2:
              this->set_fan_mode_(climate::CLIMATE_FAN_LOW);
              break;
            case mhi_ac::ACFan::speed_3:
              this->set_fan_mode_(climate::CLIMATE_FAN_MEDIUM);
              break;
            case mhi_ac::ACFan::speed_4:
              this->set_fan_mode_(climate::CLIMATE_FAN_HIGH);
              break;
            case mhi_ac::ACFan::speed_auto:
              this->set_fan_mode_(climate::CLIMATE_FAN_AUTO);
              break;
            default:
                break;
          }
        }

        if(climate_current_temperature_sensor_) {
          if(mhi_ac::spi_state.current_temperature_changed() || std::isnan(climate_current_temperature_sensor_->get_raw_state())) {
            climate_current_temperature_sensor_->publish_state(mhi_ac::spi_state.current_temperature_get());
          }

          const float new_temperature = climate_current_temperature_sensor_->get_state();
          publish_self_state |= current_temperature != new_temperature && !(std::isnan(current_temperature) && std::isnan(new_temperature));
          current_temperature = new_temperature;
        } else if(mhi_ac::spi_state.current_temperature_changed() || std::isnan(current_temperature)) {
          // Use raw value directly
          current_temperature = mhi_ac::spi_state.current_temperature_get();
          publish_self_state |= !std::isnan(current_temperature);
        }

        if(this->mode != climate::CLIMATE_MODE_HEAT && this->target_temperature < 18) {
            mhi_ac::spi_state.target_temperature_set(18);
            this->target_temperature = 18;
        }

        if(publish_self_state) {
            this->publish_state();
        }
    }

protected:
    /// Transmit the state of this climate controller.
    void control(const climate::ClimateCall& call) override
    {
        if (call.get_target_temperature().has_value()) {
          mhi_ac::spi_state.target_temperature_set(*call.get_target_temperature());
        }

        if (call.get_mode().has_value()) {
          auto mode = *call.get_mode();

          if(mode == climate::CLIMATE_MODE_OFF) {
            mhi_ac::spi_state.power_set(mhi_ac::ACPower::power_off);
          } else {
            mhi_ac::spi_state.power_set(mhi_ac::ACPower::power_on);
          }

          switch (mode) {
          case climate::CLIMATE_MODE_COOL:
            mhi_ac::spi_state.mode_set(mhi_ac::ACMode::mode_cool);
            break;
          case climate::CLIMATE_MODE_HEAT:
            mhi_ac::spi_state.mode_set(mhi_ac::ACMode::mode_heat);
            break;
          case climate::CLIMATE_MODE_DRY:
            mhi_ac::spi_state.mode_set(mhi_ac::ACMode::mode_dry);
            break;
          case climate::CLIMATE_MODE_FAN_ONLY:
            mhi_ac::spi_state.mode_set(mhi_ac::ACMode::mode_fan);
            break;
          case climate::CLIMATE_MODE_HEAT_COOL:
            mhi_ac::spi_state.mode_set(mhi_ac::ACMode::mode_auto);
            break;
          default:
            break;
          }
        }

        if (call.get_custom_fan_mode().has_value()) {
          auto fan_mode = *call.get_custom_fan_mode();
          if(fan_mode == custom_fan_ultra_low) {
            mhi_ac::spi_state.fan_set(mhi_ac::ACFan::speed_1);
          } else {
              ESP_LOGW(TAG, "Unsupported custom fan mode: %s", fan_mode.c_str());
          }
        } else if (call.get_fan_mode().has_value()) {
          auto fan_mode = *call.get_fan_mode();
          switch(fan_mode) {
            case climate::CLIMATE_FAN_AUTO:
              mhi_ac::spi_state.fan_set(mhi_ac::ACFan::speed_auto);
              break;
            case climate::CLIMATE_FAN_HIGH:
              mhi_ac::spi_state.fan_set(mhi_ac::ACFan::speed_4);
              break;
            case climate::CLIMATE_FAN_MEDIUM:
              mhi_ac::spi_state.fan_set(mhi_ac::ACFan::speed_3);
              break;
            case climate::CLIMATE_FAN_LOW:
              mhi_ac::spi_state.fan_set(mhi_ac::ACFan::speed_2);
              break;
            default:
              ESP_LOGW(TAG, "Unrecognised fan mode received");
          }
        }
    }

    /// Return the traits of this controller.
    climate::ClimateTraits traits() override
    {
        auto traits = climate::ClimateTraits();
        traits.set_supports_current_temperature(true);
        traits.set_supported_modes({ CLIMATE_MODE_OFF, CLIMATE_MODE_HEAT_COOL, CLIMATE_MODE_COOL, CLIMATE_MODE_HEAT, CLIMATE_MODE_DRY, CLIMATE_MODE_FAN_ONLY });
        traits.set_supports_action(true);
        traits.set_supports_two_point_target_temperature(false);
        traits.set_visual_min_temperature(this->minimum_temperature_);
        traits.set_visual_max_temperature(this->maximum_temperature_);
        traits.set_visual_temperature_step(this->temperature_step_);
        traits.add_supported_custom_fan_mode(custom_fan_ultra_low);
        traits.set_supported_fan_modes({ CLIMATE_FAN_LOW, CLIMATE_FAN_MEDIUM, CLIMATE_FAN_HIGH, CLIMATE_FAN_AUTO });
        //traits.set_supported_swing_modes({ CLIMATE_SWING_VERTICAL });
        return traits;
    }

    const float minimum_temperature_ { 18.0f };
    const float maximum_temperature_ { 30.0f };
    // Although the hardware accepts temperatures in steps of 0.5, it
    // effectively is per 1 degree:
    // https://github.com/absalom-muc/MHI-AC-Ctrl/issues/81
    const float temperature_step_ { 1.0f };
    const std::string custom_fan_ultra_low = std::string("Ultra Low");

    SUB_SENSOR(climate_current_temperature)

protected:
#ifdef USE_SELECT
    MhiVanesUD *vanes_ud_select_ = nullptr;
    MhiVanesLR *vanes_lr_select_ = nullptr;
#endif
    MhiFrameErrors *frame_errors_sensor_ = nullptr;
    MhiTotalEnergy *total_energy_sensor_ = nullptr;
    MhiPower *power_sensor_ = nullptr;
    mhi_ac::Config ac_config_;

public:
#ifdef USE_SELECT
    void set_vanes_ud_select(MhiVanesUD *select) {
        this->vanes_ud_select_ = select;
    }

    void set_vanes_lr_select(MhiVanesLR *select) {
        this->vanes_lr_select_ = select;
    }
#endif

    void set_frame_errors_sensor(MhiFrameErrors *sensor) {
        this->frame_errors_sensor_ = sensor;
    }

    void set_total_energy_sensor(MhiTotalEnergy *sensor) {
        this->total_energy_sensor_ = sensor;
    }

    void set_power_sensor(MhiPower *sensor) {
        this->power_sensor_ = sensor;
    }


};
