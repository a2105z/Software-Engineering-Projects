#include "smartsignal/gpio.hpp"

namespace smartsignal {

RawInputs GPIO::read_inputs() const {
  return input_reg_;
}

void GPIO::write_inputs(const RawInputs& r) {
  input_reg_ = r;
}

void GPIO::write_outputs(const Outputs& out) {
  output_reg_ = out;
}

Outputs GPIO::read_outputs() const {
  return output_reg_;
}

}  // namespace smartsignal
