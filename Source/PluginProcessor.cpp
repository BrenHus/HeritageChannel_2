#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs
{
constexpr auto inputDrive = "inputDrive";
constexpr auto inputTrim = "inputTrim";
constexpr auto preampOn = "preampOn";
constexpr auto eqOn = "eqOn";
constexpr auto hpf = "hpf";
constexpr auto lowFreq = "lowFreq";
constexpr auto lowGain = "lowGain";
constexpr auto midFreq = "midFreq";
constexpr auto midQ = "midQ";
constexpr auto eqCharacter = "eqCharacter";
constexpr auto midGain = "midGain";
constexpr auto highGain = "highGain";
constexpr auto compOn = "compOn";
constexpr auto compThreshold = "compThreshold";
constexpr auto compAmount = "compAmount";
constexpr auto timeConstant = "timeConstant";
constexpr auto compMix = "compMix";
constexpr auto compMakeup = "compMakeup";
constexpr auto tapeOn = "tapeOn";
constexpr auto tapeType = "tapeType";
constexpr auto tapeSpeed = "tapeSpeed";
constexpr auto tapeSaturation = "tapeSaturation";
constexpr auto tapeBias = "tapeBias";
constexpr auto tapeNoise = "tapeNoise";
constexpr auto masterMix = "masterMix";
constexpr auto channelOn = "channelOn";
constexpr auto output = "output";
}

HeritageChannelAudioProcessor::HeritageChannelAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "PARAMETERS", createParameterLayout()),
      oversampling(nullptr)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout HeritageChannelAudioProcessor::createParameterLayout()
{
    using F = juce::AudioParameterFloat;
    using B = juce::AudioParameterBool;
    using C = juce::AudioParameterChoice;
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add = [&p](auto parameter) { p.add(std::move(parameter)); };
    add(std::make_unique<F>(IDs::inputDrive, "Input Drive", juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 6.0f, "dB"));
    add(std::make_unique<F>(IDs::inputTrim, "Input Trim", juce::NormalisableRange<float>(-18.0f, 6.0f, 0.1f), 0.0f, "dB"));
    add(std::make_unique<B>(IDs::preampOn, "Preamp", true));
    add(std::make_unique<B>(IDs::eqOn, "EQ", true));
    add(std::make_unique<C>(IDs::hpf, "High Pass", juce::StringArray { "Off", "50 Hz", "80 Hz", "160 Hz", "300 Hz" }, 0));
    add(std::make_unique<C>(IDs::lowFreq, "Low Frequency", juce::StringArray { "35 Hz", "60 Hz", "110 Hz", "220 Hz" }, 1));
    add(std::make_unique<F>(IDs::lowGain, "Low Gain", juce::NormalisableRange<float>(-16.0f, 16.0f, 0.5f), 0.0f, "dB"));
    add(std::make_unique<C>(IDs::midFreq, "Mid Frequency", juce::StringArray { "360 Hz", "700 Hz", "1.6 kHz", "3.2 kHz", "4.8 kHz", "7.2 kHz" }, 2));
    add(std::make_unique<C>(IDs::midQ, "Mid Q", juce::StringArray { "Broad", "Tight" }, 0));
    add(std::make_unique<C>(IDs::eqCharacter, "EQ Character", juce::StringArray { "Classic", "Clean" }, 0));
    add(std::make_unique<F>(IDs::midGain, "Mid Gain", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.5f), 0.0f, "dB"));
    add(std::make_unique<F>(IDs::highGain, "12 kHz", juce::NormalisableRange<float>(-16.0f, 16.0f, 0.5f), 0.0f, "dB"));
    add(std::make_unique<B>(IDs::compOn, "Compressor", true));
    add(std::make_unique<F>(IDs::compThreshold, "Threshold", juce::NormalisableRange<float>(-30.0f, 0.0f, 0.1f), -10.0f, "dB"));
    add(std::make_unique<F>(IDs::compAmount, "Compression", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 55.0f, "%"));
    add(std::make_unique<C>(IDs::timeConstant, "Time Constant", juce::StringArray { "1", "2", "3", "4", "5 Auto", "6 Auto" }, 4));
    add(std::make_unique<F>(IDs::compMix, "Compressor Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f, "%"));
    add(std::make_unique<F>(IDs::compMakeup, "Make-up", juce::NormalisableRange<float>(-6.0f, 18.0f, 0.1f), 0.0f, "dB"));
    add(std::make_unique<B>(IDs::tapeOn, "Tape", true));
    add(std::make_unique<C>(IDs::tapeType, "Tape Stock", juce::StringArray { "3M 250", "Ampex 456", "BASF SM900" }, 1));
    add(std::make_unique<C>(IDs::tapeSpeed, "Tape Speed", juce::StringArray { "7.5 ips", "15 ips", "30 ips" }, 1));
    add(std::make_unique<F>(IDs::tapeSaturation, "Tape Saturation", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 35.0f, "%"));
    add(std::make_unique<F>(IDs::tapeBias, "Tape Bias", juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f), 0.0f, "%"));
    add(std::make_unique<B>(IDs::tapeNoise, "Tape Noise", false));
    add(std::make_unique<F>(IDs::masterMix, "Master Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f, "%"));
    add(std::make_unique<B>(IDs::channelOn, "Channel", true));
    add(std::make_unique<F>(IDs::output, "Output", juce::NormalisableRange<float>(-18.0f, 12.0f, 0.1f), 0.0f, "dB"));
    return p;
}

void HeritageChannelAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        static_cast<size_t>(getTotalNumOutputChannels()), 1,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
    oversampling->initProcessing(static_cast<size_t>(samplesPerBlock));
    setLatencySamples(static_cast<int>(std::round(oversampling->getLatencyInSamples())));
    signalFlow.prepare(sampleRate * oversampling->getOversamplingFactor());
    signalFlow.update(readParameters());
    dryBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock, false, false, true);
}

bool HeritageChannelAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

heritage::Parameters HeritageChannelAudioProcessor::readParameters() const
{
    heritage::Parameters p;
    auto f = [this](const char* id) { return state.getRawParameterValue(id)->load(); };
    p.inputDriveDb = f(IDs::inputDrive); p.inputTrimDb = f(IDs::inputTrim);
    p.preampEnabled = f(IDs::preampOn) > 0.5f; p.eqEnabled = f(IDs::eqOn) > 0.5f;
    p.hpfIndex = static_cast<int>(f(IDs::hpf)); p.lowFreqIndex = static_cast<int>(f(IDs::lowFreq));
    p.lowGainDb = f(IDs::lowGain); p.midFreqIndex = static_cast<int>(f(IDs::midFreq));
    p.midQ = static_cast<int>(f(IDs::midQ)); p.eqCharacter = static_cast<int>(f(IDs::eqCharacter));
    p.midGainDb = f(IDs::midGain); p.highGainDb = f(IDs::highGain);
    p.compressorEnabled = f(IDs::compOn) > 0.5f; p.compThresholdDb = f(IDs::compThreshold);
    p.compAmount = f(IDs::compAmount); p.timeConstant = static_cast<int>(f(IDs::timeConstant));
    p.compMix = f(IDs::compMix); p.compMakeupDb = f(IDs::compMakeup);
    p.tapeEnabled = f(IDs::tapeOn) > 0.5f;
    p.tapeType = static_cast<int>(f(IDs::tapeType)); p.tapeSpeed = static_cast<int>(f(IDs::tapeSpeed));
    p.tapeSaturation = f(IDs::tapeSaturation); p.tapeBias = f(IDs::tapeBias);
    p.tapeNoise = f(IDs::tapeNoise) > 0.5f;
    p.masterMix = f(IDs::masterMix); p.channelEnabled = f(IDs::channelOn) > 0.5f;
    p.outputDb = f(IDs::output);
    return p;
}

void HeritageChannelAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear(c, 0, buffer.getNumSamples());

    const auto p = readParameters();
    if (!p.channelEnabled)
    {
        gainReductionDb.store(0.0f);
        outputPeak.store(buffer.getMagnitude(0, buffer.getNumSamples()));
        return;
    }
    dryBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);
    for (int c = 0; c < buffer.getNumChannels(); ++c)
        dryBuffer.copyFrom(c, 0, buffer, c, 0, buffer.getNumSamples());
    signalFlow.update(p);
    juce::dsp::AudioBlock<float> baseBlock(buffer);
    if (oversampling == nullptr) return;
    auto up = oversampling->processSamplesUp(baseBlock);
    auto peak = 0.0f;
    const auto channels = up.getNumChannels();
    auto* left = up.getChannelPointer(0);
    auto* right = channels > 1 ? up.getChannelPointer(1) : nullptr;
    for (size_t i = 0; i < up.getNumSamples(); ++i)
    {
        const auto rIn = right != nullptr ? right[i] : left[i];
        const auto y = signalFlow.process(left[i], rIn, p);
        left[i] = y.first;
        if (right != nullptr) right[i] = y.second;
        peak = std::max(peak, std::max(std::abs(y.first), std::abs(y.second)));
    }
    oversampling->processSamplesDown(baseBlock);
    const auto wet = juce::jlimit(0.0f, 1.0f, p.masterMix / 100.0f);
    if (wet < 1.0f)
        for (int c = 0; c < buffer.getNumChannels(); ++c)
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                buffer.setSample(c, i, dryBuffer.getSample(c, i)
                                      + (buffer.getSample(c, i) - dryBuffer.getSample(c, i)) * wet);
    gainReductionDb.store(signalFlow.gainReductionDb());
    outputPeak.store(peak);
}

void HeritageChannelAudioProcessor::getStateInformation(juce::MemoryBlock& data)
{
    if (auto xml = state.copyState().createXml()) copyXmlToBinary(*xml, data);
}

void HeritageChannelAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* HeritageChannelAudioProcessor::createEditor()
{
    return new HeritageChannelAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HeritageChannelAudioProcessor();
}
