#pragma once

namespace esphome {
namespace xiaomi {
namespace bslamp2 {

/**
 * This is an interface definition that is used to extend the LightState
 * class with functionality for disco actions (immediate light updates,
 * not publishing or saving the light state).
 *
 * This interface is required by the DiscoAction class.
 */
class LightStateDiscoSupport {
 public:
  /**
   * Stop the disco, by restoring the previously remembered light state. 
   */
  virtual void disco_stop() = 0;

  /**
   * Do not wait until the next loop() call for the light to write the
   * requested state to the light output, but write the new state
   * right away.
   *
   * This allows us to update the state of the light, even when we are
   * being called in the middle of another component's loop().
   */
  virtual void disco_apply() = 0;

  /**
   * Create a light::LightCall object, with some properties already
   * configured for using it as a disco call.
   */
  virtual light::LightCall make_disco_call(bool save_and_publish) = 0;
};

}  // namespace bslamp2
}  // namespace xiaomi
}  // namespace esphome
