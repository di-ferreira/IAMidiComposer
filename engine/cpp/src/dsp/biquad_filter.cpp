#include <aimidi/dsp/BiquadFilter.hpp>
#include <cmath>

namespace aimidi::dsp {

void BiquadFilter::setCoefficients(double b0, double b1, double b2, double a1, double a2) {
    b0_ = b0; b1_ = b1; b2_ = b2;
    a1_ = a1; a2_ = a2;
}

void BiquadFilter::reset() {
    s1_ = 0.0;
    s2_ = 0.0;
}

void BiquadFilter::setLowPass(double sampleRate, double cutoff, double q) {
    double w0 = 2.0 * M_PI * cutoff / sampleRate;
    double cosW = std::cos(w0);
    double sinW = std::sin(w0);
    double alpha = sinW / (2.0 * q);
    double norm = 1.0 / (1.0 + alpha);
    setCoefficients(
        (1.0 - cosW) * 0.5 * norm,
        (1.0 - cosW) * norm,
        (1.0 - cosW) * 0.5 * norm,
        -2.0 * cosW * norm,
        (1.0 - alpha) * norm
    );
}

void BiquadFilter::setHighPass(double sampleRate, double cutoff, double q) {
    double w0 = 2.0 * M_PI * cutoff / sampleRate;
    double cosW = std::cos(w0);
    double sinW = std::sin(w0);
    double alpha = sinW / (2.0 * q);
    double norm = 1.0 / (1.0 + alpha);
    setCoefficients(
        (1.0 + cosW) * 0.5 * norm,
        -(1.0 + cosW) * norm,
        (1.0 + cosW) * 0.5 * norm,
        -2.0 * cosW * norm,
        (1.0 - alpha) * norm
    );
}

void BiquadFilter::setBandPass(double sampleRate, double cutoff, double q) {
    double w0 = 2.0 * M_PI * cutoff / sampleRate;
    double cosW = std::cos(w0);
    double sinW = std::sin(w0);
    double alpha = sinW / (2.0 * q);
    double norm = 1.0 / (1.0 + alpha);
    setCoefficients(
        alpha * norm,
        0.0,
        -alpha * norm,
        -2.0 * cosW * norm,
        (1.0 - alpha) * norm
    );
}

void BiquadFilter::setNotch(double sampleRate, double cutoff, double q) {
    double w0 = 2.0 * M_PI * cutoff / sampleRate;
    double cosW = std::cos(w0);
    double sinW = std::sin(w0);
    double alpha = sinW / (2.0 * q);
    double norm = 1.0 / (1.0 + alpha);
    setCoefficients(
        norm,
        -2.0 * cosW * norm,
        norm,
        -2.0 * cosW * norm,
        (1.0 - alpha) * norm
    );
}

void BiquadFilter::setPeak(double sampleRate, double cutoff, double gain, double q) {
    double w0 = 2.0 * M_PI * cutoff / sampleRate;
    double cosW = std::cos(w0);
    double sinW = std::sin(w0);
    double alpha = sinW / (2.0 * q);
    double A = std::pow(10.0, gain / 40.0);
    double norm = 1.0 / (1.0 + alpha / A);
    setCoefficients(
        (1.0 + alpha * A) * norm,
        -2.0 * cosW * norm,
        (1.0 - alpha * A) * norm,
        -2.0 * cosW * norm,
        (1.0 - alpha / A) * norm
    );
}

void BiquadFilter::setLowShelf(double sampleRate, double cutoff, double gain, double q) {
    double w0 = 2.0 * M_PI * cutoff / sampleRate;
    double cosW = std::cos(w0);
    double sinW = std::sin(w0);
    double A = std::pow(10.0, gain / 40.0);
    double alpha = sinW / 2.0 * std::sqrt((A + 1.0 / A) * (1.0 / q - 1.0) + 2.0);
    double beta = 2.0 * std::sqrt(A) * alpha;
    double norm = 1.0 / ((A + 1.0) + (A - 1.0) * cosW + beta);
    setCoefficients(
        A * ((A + 1.0) - (A - 1.0) * cosW + beta) * norm,
        2.0 * A * ((A - 1.0) - (A + 1.0) * cosW) * norm,
        A * ((A + 1.0) - (A - 1.0) * cosW - beta) * norm,
        -2.0 * ((A - 1.0) + (A + 1.0) * cosW) * norm,
        ((A + 1.0) + (A - 1.0) * cosW - beta) * norm
    );
}

void BiquadFilter::setHighShelf(double sampleRate, double cutoff, double gain, double q) {
    double w0 = 2.0 * M_PI * cutoff / sampleRate;
    double cosW = std::cos(w0);
    double sinW = std::sin(w0);
    double A = std::pow(10.0, gain / 40.0);
    double alpha = sinW / 2.0 * std::sqrt((A + 1.0 / A) * (1.0 / q - 1.0) + 2.0);
    double beta = 2.0 * std::sqrt(A) * alpha;
    double norm = 1.0 / ((A + 1.0) - (A - 1.0) * cosW + beta);
    setCoefficients(
        A * ((A + 1.0) + (A - 1.0) * cosW + beta) * norm,
        -2.0 * A * ((A - 1.0) + (A + 1.0) * cosW) * norm,
        A * ((A + 1.0) + (A - 1.0) * cosW - beta) * norm,
        2.0 * ((A - 1.0) - (A + 1.0) * cosW) * norm,
        ((A + 1.0) - (A - 1.0) * cosW - beta) * norm
    );
}

float BiquadFilter::process(float sample) {
    double x = static_cast<double>(sample);
    double y = b0_ * x + s1_;
    s1_ = b1_ * x - a1_ * y + s2_;
    s2_ = b2_ * x - a2_ * y;
    return static_cast<float>(y);
}

void BiquadFilter::processBlock(float* samples, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        samples[i] = process(samples[i]);
    }
}

void OnePoleSmoother::setTimeConstant(double tau, double sampleRate) {
    a_ = 1.0 - std::exp(-1.0 / (tau * sampleRate));
}

void OnePoleSmoother::setTarget(double target) {
    target_ = target;
}

float OnePoleSmoother::process() {
    state_ = state_ + a_ * (target_ - state_);
    return static_cast<float>(state_);
}

} // namespace aimidi::dsp
