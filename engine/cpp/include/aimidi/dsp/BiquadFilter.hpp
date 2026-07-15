#pragma once
#include <array>
#include <cstddef>

namespace aimidi::dsp {

enum class BiquadType {
    LowPass,
    HighPass,
    BandPass,
    Notch,
    Peak,
    LowShelf,
    HighShelf,
    AllPass
};

class BiquadFilter {
public:
    BiquadFilter() = default;
    ~BiquadFilter() = default;

    void setLowPass(double sampleRate, double cutoff, double q = 0.707);
    void setHighPass(double sampleRate, double cutoff, double q = 0.707);
    void setBandPass(double sampleRate, double cutoff, double q = 0.707);
    void setNotch(double sampleRate, double cutoff, double q = 0.707);
    void setPeak(double sampleRate, double cutoff, double gain, double q = 0.707);
    void setLowShelf(double sampleRate, double cutoff, double gain, double q = 0.707);
    void setHighShelf(double sampleRate, double cutoff, double gain, double q = 0.707);

    void reset();
    float process(float sample);

    void processBlock(float* samples, std::size_t count);

private:
    void setCoefficients(double b0, double b1, double b2, double a1, double a2);

    double b0_ = 1.0, b1_ = 0.0, b2_ = 0.0;
    double a1_ = 0.0, a2_ = 0.0;
    double s1_ = 0.0, s2_ = 0.0;
};

class OnePoleSmoother {
public:
    OnePoleSmoother() = default;

    void setTimeConstant(double tau, double sampleRate);
    void setTarget(double target);
    float process();

private:
    double a_ = 1.0;
    double state_ = 0.0;
    double target_ = 0.0;
};

} // namespace aimidi::dsp
