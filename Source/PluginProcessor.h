#pragma once

#include <JuceHeader.h>
#include "dsp/HeritageDSP.h"

class HeritageChannelAudioProcessor final : public juce::AudioProcessor
{
public:
    HeritageChannelAudioProcessor();
    ~HeritageChannelAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState state;
    float getGainReductionDb() const { return gainReductionDb.load(); }
    float getOutputPeak() const { return outputPeak.load(); }

private:
    heritage::Parameters readParameters() const;
    heritage::ChannelStrip signalFlow;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    juce::AudioBuffer<float> dryBuffer;
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> outputPeak { 0.0f };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeritageChannelAudioProcessor)
};
