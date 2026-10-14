#include "../dsp/HeritageDSP.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

using heritage::ChannelStrip;
using heritage::Parameters;

static float rms(const std::vector<float>& v)
{
    double sum = 0.0;
    for (auto x : v) sum += x * x;
    return static_cast<float>(std::sqrt(sum / v.size()));
}

static float renderSine(ChannelStrip& dsp, Parameters p, double frequency, float amplitude)
{
    constexpr double fs = 48000.0;
    dsp.reset();
    dsp.update(p);
    std::vector<float> result(24000);
    for (size_t i = 0; i < result.size(); ++i)
    {
        const auto x = amplitude * std::sin(static_cast<float>(2.0 * heritage::pi * frequency * i / fs));
        result[i] = dsp.process(x, x, p).first;
    }
    return rms(std::vector<float>(result.begin() + 8000, result.end()));
}

int main()
{
    constexpr double fs = 48000.0;
    ChannelStrip dsp;
    Parameters p;
    dsp.prepare(fs);
    dsp.update(p);

    std::vector<float> output(48000);
    for (size_t i = 0; i < output.size(); ++i)
    {
        const auto x = 0.2f * std::sin(static_cast<float>(2.0 * heritage::pi * 997.0 * i / fs));
        output[i] = dsp.process(x, x, p).first;
        assert(std::isfinite(output[i]));
    }
    assert(rms(output) > 0.01f);
    assert(rms(output) < 1.5f);

    p.compThresholdDb = -28.0f;
    p.compAmount = 90.0f;
    dsp.reset();
    for (int i = 0; i < 24000; ++i) dsp.process(0.8f, 0.8f, p);
    assert(dsp.gainReductionDb() > 1.0f);

    Parameters makeup;
    makeup.inputDriveDb = 0.0f;
    makeup.eqEnabled = false;
    makeup.tapeEnabled = false;
    makeup.compThresholdDb = 0.0f;
    makeup.compAmount = 0.0f;
    makeup.compMakeupDb = 6.0f;
    const auto withoutMakeup = renderSine(dsp, Parameters{}, 997.0, 0.03f);
    const auto withMakeup = renderSine(dsp, makeup, 997.0, 0.03f);
    assert(withMakeup > withoutMakeup * 1.25f);

    Parameters clean;
    clean.inputDriveDb = 0.0f;
    clean.compressorEnabled = false;
    clean.tapeEnabled = false;
    clean.hpfIndex = 0;
    const auto bassWithoutFilter = renderSine(dsp, clean, 30.0, 0.1f);
    clean.hpfIndex = 2;
    const auto bassWith80HzFilter = renderSine(dsp, clean, 30.0, 0.1f);
    assert(bassWith80HzFilter < bassWithoutFilter * 0.4f);

    Parameters midWidth;
    midWidth.inputDriveDb = 0.0f;
    midWidth.compressorEnabled = false;
    midWidth.tapeEnabled = false;
    midWidth.midFreqIndex = 2;
    midWidth.midGainDb = 12.0f;
    midWidth.midQ = 0;
    const auto broadAt900 = renderSine(dsp, midWidth, 900.0, 0.03f);
    midWidth.midQ = 1;
    const auto tightAt900 = renderSine(dsp, midWidth, 900.0, 0.03f);
    assert(broadAt900 > tightAt900);

    Parameters tapeComparison;
    tapeComparison.inputDriveDb = 0.0f;
    tapeComparison.eqEnabled = false;
    tapeComparison.compressorEnabled = false;
    tapeComparison.tapeSaturation = 90.0f;
    tapeComparison.tapeType = 0;
    const auto stock250 = renderSine(dsp, tapeComparison, 997.0, 0.7f);
    tapeComparison.tapeType = 2;
    const auto stock900 = renderSine(dsp, tapeComparison, 997.0, 0.7f);
    assert(std::abs(stock250 - stock900) > 0.005f);

    for (int stock = 0; stock < 3; ++stock)
    {
        p.tapeType = stock;
        p.tapeSaturation = 100.0f;
        dsp.reset();
        dsp.update(p);
        for (int i = 0; i < 4096; ++i)
        {
            const auto y = dsp.process(i == 0 ? 0.9f : 0.0f, 0.0f, p).first;
            assert(std::isfinite(y));
            assert(std::abs(y) < 4.0f);
        }
    }

    std::cout << "Heritage DSP tests passed\n";
    return 0;
}
