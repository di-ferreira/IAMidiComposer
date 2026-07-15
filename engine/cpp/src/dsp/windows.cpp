#include <aimidi/dsp/Windows.hpp>
#include <algorithm>

namespace aimidi::dsp {

Window::Window(std::size_t size, WindowType type, double beta)
    : size_(size), type_(type), beta_(beta) {
    coeffs_.resize(size);
    compute();
}

void Window::compute() {
    std::size_t N = size_;
    for (std::size_t n = 0; n < N; ++n) {
        double w = 1.0;
        double a0, a1, a2, a3;

        switch (type_) {
        case WindowType::Rectangular:
            w = 1.0;
            break;
        case WindowType::Hann:
            w = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / (N - 1)));
            break;
        case WindowType::Hamming:
            w = 0.53836 - 0.46164 * std::cos(2.0 * M_PI * n / (N - 1));
            break;
        case WindowType::Blackman:
            a0 = 0.42; a1 = 0.5; a2 = 0.08;
            w = a0
                - a1 * std::cos(2.0 * M_PI * n / (N - 1))
                + a2 * std::cos(4.0 * M_PI * n / (N - 1));
            break;
        case WindowType::BlackmanHarris:
            a0 = 0.35875; a1 = 0.48829; a2 = 0.14128; a3 = 0.01168;
            w = a0
                - a1 * std::cos(2.0 * M_PI * n / (N - 1))
                + a2 * std::cos(4.0 * M_PI * n / (N - 1))
                - a3 * std::cos(6.0 * M_PI * n / (N - 1));
            break;
        case WindowType::Kaiser:
            w = kaiserBeta(beta_);
            break;
        }
        coeffs_[n] = static_cast<float>(w);
    }
}

double Window::kaiserBeta(double beta) {
    std::size_t N = size_;
    auto I0 = [](double x) -> double {
        double sum = 1.0;
        double term = 1.0;
        for (int k = 1; k <= 50; ++k) {
            term *= (x / (2.0 * k));
            term *= (x / (2.0 * k));
            sum += term;
            if (std::abs(term) < 1e-15) break;
        }
        return sum;
    };

    double denom = I0(beta);
    for (std::size_t n = 0; n < size_; ++n) {
        double t = 2.0 * n / (size_ - 1.0) - 1.0;
        double arg = beta * std::sqrt(1.0 - t * t);
        coeffs_[n] = static_cast<float>(I0(arg) / denom);
    }
    return denom;
}

void Window::apply(float* data) const {
    for (std::size_t i = 0; i < size_; ++i) {
        data[i] *= coeffs_[i];
    }
}

} // namespace aimidi::dsp
