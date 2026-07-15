#pragma once
#include <vector>
#include <cstddef>
#include <cmath>

namespace aimidi::dsp {

enum class WindowType {
    Rectangular,
    Hann,
    Hamming,
    Blackman,
    BlackmanHarris,
    Kaiser
};

class Window {
public:
    explicit Window(std::size_t size, WindowType type = WindowType::Hann, double beta = 6.0);
    ~Window() = default;

    void apply(float* data) const;

    std::size_t size() const { return size_; }
    WindowType type() const { return type_; }
    const float* coefficients() const { return coeffs_.data(); }

private:
    void compute();
    double kaiserBeta(double beta);

    std::size_t size_;
    WindowType type_;
    double beta_;
    std::vector<float> coeffs_;
};

} // namespace aimidi::dsp
