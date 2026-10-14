#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <utility>

namespace heritage
{
constexpr double pi = 3.14159265358979323846;

inline float dbToGain(float db) { return std::pow(10.0f, db / 20.0f); }
inline float clamp(float x, float lo, float hi) { return std::max(lo, std::min(hi, x)); }

class Biquad
{
public:
    void reset() { z1 = z2 = 0.0; }
    void bypass() { b0 = 1.0; b1 = b2 = a1 = a2 = 0.0; }

    float process(float x)
    {
        const auto y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return static_cast<float>(y);
    }

    void lowPass(double fs, double hz, double q = 0.70710678)
    {
        const auto w = 2.0 * pi * safeHz(fs, hz) / fs;
        const auto c = std::cos(w), s = std::sin(w), alpha = s / (2.0 * q);
        set((1.0 - c) * 0.5, 1.0 - c, (1.0 - c) * 0.5, 1.0 + alpha, -2.0 * c, 1.0 - alpha);
    }

    void highPass(double fs, double hz, double q = 0.70710678)
    {
        const auto w = 2.0 * pi * safeHz(fs, hz) / fs;
        const auto c = std::cos(w), s = std::sin(w), alpha = s / (2.0 * q);
        set((1.0 + c) * 0.5, -(1.0 + c), (1.0 + c) * 0.5, 1.0 + alpha, -2.0 * c, 1.0 - alpha);
    }

    void peak(double fs, double hz, double q, double gainDb)
    {
        const auto a = std::pow(10.0, gainDb / 40.0);
        const auto w = 2.0 * pi * safeHz(fs, hz) / fs;
        const auto c = std::cos(w), s = std::sin(w), alpha = s / (2.0 * q);
        set(1.0 + alpha * a, -2.0 * c, 1.0 - alpha * a,
            1.0 + alpha / a, -2.0 * c, 1.0 - alpha / a);
    }

    void lowShelf(double fs, double hz, double gainDb)
    {
        shelf(fs, hz, gainDb, false);
    }

    void highShelf(double fs, double hz, double gainDb)
    {
        shelf(fs, hz, gainDb, true);
    }

private:
    static double safeHz(double fs, double hz) { return std::min(hz, fs * 0.45); }

    void set(double nb0, double nb1, double nb2, double na0, double na1, double na2)
    {
        b0 = nb0 / na0; b1 = nb1 / na0; b2 = nb2 / na0;
        a1 = na1 / na0; a2 = na2 / na0;
    }

    void shelf(double fs, double hz, double gainDb, bool high)
    {
        const auto a = std::pow(10.0, gainDb / 40.0);
        const auto w = 2.0 * pi * safeHz(fs, hz) / fs;
        const auto c = std::cos(w), s = std::sin(w);
        const auto beta = 2.0 * std::sqrt(a) * s / std::sqrt(2.0);
        if (high)
            set(a * ((a + 1.0) + (a - 1.0) * c + beta),
                -2.0 * a * ((a - 1.0) + (a + 1.0) * c),
                a * ((a + 1.0) + (a - 1.0) * c - beta),
                (a + 1.0) - (a - 1.0) * c + beta,
                2.0 * ((a - 1.0) - (a + 1.0) * c),
                (a + 1.0) - (a - 1.0) * c - beta);
        else
            set(a * ((a + 1.0) - (a - 1.0) * c + beta),
                2.0 * a * ((a - 1.0) - (a + 1.0) * c),
                a * ((a + 1.0) - (a - 1.0) * c - beta),
                (a + 1.0) + (a - 1.0) * c + beta,
                -2.0 * ((a - 1.0) + (a + 1.0) * c),
                (a + 1.0) + (a - 1.0) * c - beta);
    }

    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
    double z1 = 0.0, z2 = 0.0;
};

class FirstOrderHighPass
{
public:
    void reset() { x1 = y1 = 0.0f; }
    void set(double fs, double hz)
    {
        const auto rc = 1.0 / (2.0 * pi * hz);
        const auto dt = 1.0 / fs;
        a = static_cast<float>(rc / (rc + dt));
    }
    float process(float x)
    {
        const auto y = a * (y1 + x - x1);
        x1 = x; y1 = y; return y;
    }
private:
    float a = 0.0f, x1 = 0.0f, y1 = 0.0f;
};

struct Parameters
{
    float inputDriveDb = 6.0f;
    float inputTrimDb = 0.0f;
    bool preampEnabled = true;
    bool eqEnabled = true;
    int hpfIndex = 0;
    int lowFreqIndex = 1;
    float lowGainDb = 0.0f;
    int midFreqIndex = 2;
    int midQ = 0;
    int eqCharacter = 0;
    float midGainDb = 0.0f;
    float highGainDb = 0.0f;
    bool compressorEnabled = true;
    float compThresholdDb = -10.0f;
    float compAmount = 55.0f;
    int timeConstant = 4;
    float compMix = 100.0f;
    float compMakeupDb = 0.0f;
    bool tapeEnabled = true;
    int tapeType = 1;
    int tapeSpeed = 1;
    float tapeSaturation = 35.0f;
    float tapeBias = 0.0f;
    bool tapeNoise = false;
    float masterMix = 100.0f;
    bool channelEnabled = true;
    float outputDb = 0.0f;
};

class NeveStage
{
public:
    void prepare(double newSampleRate)
    {
        fs = newSampleRate;
        reset();
        update(Parameters{});
    }

    void reset()
    {
        hpf1.reset(); hpf2.reset(); low.reset(); mid.reset(); high.reset();
        transformerMemory = 0.0f;
    }

    void update(const Parameters& p)
    {
        const std::array<double, 5> hp { 0.0, 50.0, 80.0, 160.0, 300.0 };
        const std::array<double, 4> lf { 35.0, 60.0, 110.0, 220.0 };
        const std::array<double, 6> mf { 360.0, 700.0, 1600.0, 3200.0, 4800.0, 7200.0 };
        hpHz = hp[static_cast<size_t>(std::clamp(p.hpfIndex, 0, 4))];
        if (hpHz > 0.0) { hpf1.set(fs, hpHz); hpf2.highPass(fs, hpHz, 1.0); }
        low.lowShelf(fs, lf[static_cast<size_t>(std::clamp(p.lowFreqIndex, 0, 3))], p.lowGainDb);
        // Broad gives the musical inductor-style bell; Tight is useful for corrective work.
        const auto mi = std::clamp(p.midFreqIndex, 0, 5);
        const auto q = p.midQ == 0 ? 0.72 + 0.05 * mi : 1.65 + 0.08 * mi;
        mid.peak(fs, mf[static_cast<size_t>(mi)], q, p.midGainDb);
        high.highShelf(fs, 12000.0, p.highGainDb);
    }

    float process(float x, const Parameters& p)
    {
        if (p.preampEnabled)
        {
            const auto drive = dbToGain(p.inputDriveDb);
            const auto driven = x * drive;
            // Slight asymmetry plus a slow transformer memory term gives predominantly low-order colour.
            transformerMemory += 0.0007f * (driven - transformerMemory);
            const auto classic = p.eqCharacter == 0;
            const auto bias = classic ? 0.035f : 0.010f;
            const auto memory = classic ? 0.045f : 0.015f;
            const auto curve = classic ? 0.92f : 0.72f;
            const auto biased = driven + bias + memory * transformerMemory;
            x = (std::tanh(curve * biased) - std::tanh(curve * bias))
                / std::max(0.001f, drive * (classic ? 0.82f : 0.70f));
        }
        if (p.eqEnabled)
        {
            if (hpHz > 0.0) x = hpf2.process(hpf1.process(x));
            x = low.process(x);
            x = mid.process(x);
            x = high.process(x);
        }
        return x * dbToGain(p.inputTrimDb);
    }

private:
    double fs = 48000.0, hpHz = 0.0;
    FirstOrderHighPass hpf1;
    Biquad hpf2, low, mid, high;
    float transformerMemory = 0.0f;
};

class FairchildStage
{
public:
    void prepare(double newSampleRate) { fs = newSampleRate; reset(); }
    void reset() { fastEnv = slowEnv = gain = 0.0f; }
    float gainReductionDb() const { return -gain; }

    std::pair<float, float> process(float left, float right, const Parameters& p)
    {
        if (!p.compressorEnabled) { gain = 0.0f; return { left, right }; }
        static constexpr std::array<float, 6> attackMs { 0.20f, 0.20f, 0.40f, 0.80f, 0.20f, 0.40f };
        static constexpr std::array<float, 6> releaseMs { 300.0f, 800.0f, 2000.0f, 5000.0f, 2000.0f, 300.0f };
        const auto tc = std::clamp(p.timeConstant, 0, 5);
        const auto detector = std::max(std::abs(left), std::abs(right));
        const auto levelDb = 20.0f * std::log10(detector + 1.0e-9f);
        const auto over = std::max(0.0f, levelDb - p.compThresholdDb);
        const auto ratio = 1.6f + 18.0f * std::pow(clamp(p.compAmount / 100.0f, 0.0f, 1.0f), 1.35f);
        const auto target = -over * (1.0f - 1.0f / ratio);
        const auto attack = coeff(attackMs[static_cast<size_t>(tc)]);
        auto release = coeff(releaseMs[static_cast<size_t>(tc)]);
        const auto c = target < fastEnv ? attack : release;
        fastEnv = c * fastEnv + (1.0f - c) * target;

        if (tc >= 4)
        {
            const auto slowReleaseMs = tc == 4 ? 10000.0f : 25000.0f;
            const auto sc = target < slowEnv ? attack : coeff(slowReleaseMs);
            slowEnv = sc * slowEnv + (1.0f - sc) * target;
            const auto sustained = clamp(over / 14.0f, 0.0f, 1.0f);
            gain = fastEnv * (1.0f - sustained) + slowEnv * sustained;
        }
        else gain = fastEnv;

        const auto g = dbToGain(gain);
        const auto wetL = tube(left * g);
        const auto wetR = tube(right * g);
        const auto mix = clamp(p.compMix / 100.0f, 0.0f, 1.0f);
        const auto makeup = dbToGain(p.compMakeupDb);
        return { (left + (wetL - left) * mix) * makeup,
                 (right + (wetR - right) * mix) * makeup };
    }

private:
    float coeff(float ms) const { return std::exp(-1.0f / static_cast<float>(0.001 * ms * fs)); }
    static float tube(float x)
    {
        const auto y = std::tanh(x * 1.08f + 0.018f) - std::tanh(0.018f);
        return y / 0.80f;
    }
    double fs = 48000.0;
    float fastEnv = 0.0f, slowEnv = 0.0f, gain = 0.0f;
};

class TapeStage
{
public:
    void prepare(double newSampleRate) { fs = newSampleRate; reset(); update(Parameters{}); }
    void reset()
    {
        recordEq.reset(); bump.reset(); replayLoss.reset(); dcBlock.reset();
        magnetisation = 0.0f; rng = 0x91e10da5u;
    }

    void update(const Parameters& p)
    {
        static constexpr std::array<double, 3> bumpHz { 48.0, 62.0, 78.0 };
        static constexpr std::array<double, 3> rolloff { 13500.0, 20500.0, 28500.0 };
        static constexpr std::array<double, 3> bumpDb { 2.8, 1.6, 0.7 };
        const auto s = std::clamp(p.tapeSpeed, 0, 2);
        bump.peak(fs, bumpHz[static_cast<size_t>(s)], 0.72, bumpDb[static_cast<size_t>(s)]);
        replayLoss.lowPass(fs, rolloff[static_cast<size_t>(s)], 0.70710678);
        // Record pre-emphasis and replay loss do not cancel perfectly when tape is driven.
        recordEq.highShelf(fs, s == 0 ? 3500.0 : 5000.0, s == 0 ? 2.4 : (s == 1 ? 1.5 : 0.8));
        dcBlock.highPass(fs, 18.0, 0.70710678);
    }

    float process(float x, const Parameters& p)
    {
        if (!p.tapeEnabled) return x;
        struct Stock { float baseDrive, saturation, memory, noiseDb, hfTrim; };
        static constexpr std::array<Stock, 3> stocks {{
            { 1.32f, 0.76f, 0.032f, -66.0f, -0.7f }, // 3M 250
            { 1.18f, 0.88f, 0.026f, -69.0f,  0.0f }, // Ampex 456
            { 1.03f, 1.08f, 0.019f, -73.0f,  0.5f }  // BASF SM900
        }};
        const auto st = stocks[static_cast<size_t>(std::clamp(p.tapeType, 0, 2))];
        const auto amount = clamp(p.tapeSaturation / 100.0f, 0.0f, 1.0f);
        const auto bias = clamp(p.tapeBias / 100.0f, -1.0f, 1.0f);
        const auto biasDrive = 1.0f - 0.16f * bias;
        auto driven = recordEq.process(x) * (st.baseDrive * (0.78f + 2.75f * amount) * biasDrive);
        const auto effective = driven + st.memory * magnetisation;
        const auto target = st.saturation * std::tanh((effective + 0.012f * bias) / st.saturation)
                            - st.saturation * std::tanh(0.012f * bias / st.saturation);
        const auto slew = 0.58f + 0.28f * (1.0f - amount);
        magnetisation += slew * (target - magnetisation);
        auto y = magnetisation / std::max(0.001f, st.baseDrive * (0.78f + 1.25f * amount));
        y = replayLoss.process(bump.process(y));
        if (st.hfTrim != 0.0f)
            y += st.hfTrim * 0.0015f * (y - previous);
        previous = y;
        if (p.tapeNoise) y += randomBipolar() * dbToGain(st.noiseDb);
        return dcBlock.process(y);
    }

private:
    float randomBipolar()
    {
        rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
        return static_cast<float>((rng & 0x00ffffffu) / 8388607.5 - 1.0);
    }
    double fs = 48000.0;
    Biquad recordEq, bump, replayLoss, dcBlock;
    float magnetisation = 0.0f, previous = 0.0f;
    uint32_t rng = 0x91e10da5u;
};

class ChannelStrip
{
public:
    void prepare(double sampleRate)
    {
        fs = sampleRate;
        for (auto& s : neve) s.prepare(sampleRate);
        fairchild.prepare(sampleRate);
        for (auto& s : tape) s.prepare(sampleRate);
        last = Parameters{};
    }

    void reset()
    {
        for (auto& s : neve) s.reset();
        fairchild.reset();
        for (auto& s : tape) s.reset();
    }

    void update(const Parameters& p)
    {
        for (auto& s : neve) s.update(p);
        for (auto& s : tape) s.update(p);
        last = p;
    }

    std::pair<float, float> process(float left, float right, const Parameters& p)
    {
        left = neve[0].process(left, p);
        right = neve[1].process(right, p);
        auto compressed = fairchild.process(left, right, p);
        left = tape[0].process(compressed.first, p);
        right = tape[1].process(compressed.second, p);
        const auto out = dbToGain(p.outputDb);
        return { left * out, right * out };
    }

    float gainReductionDb() const { return fairchild.gainReductionDb(); }

private:
    double fs = 48000.0;
    Parameters last;
    std::array<NeveStage, 2> neve;
    FairchildStage fairchild;
    std::array<TapeStage, 2> tape;
};
}
