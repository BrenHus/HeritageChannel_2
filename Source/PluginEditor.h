#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class HeritageLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    HeritageLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
};

class HeritageChannelAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit HeritageChannelAudioProcessorEditor(HeritageChannelAudioProcessor&);
    ~HeritageChannelAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void timerCallback() override;
    void configureKnob(juce::Slider&, const juce::String&);
    void configureCombo(juce::ComboBox&, const juce::String&, const juce::StringArray&);
    void configureToggle(juce::ToggleButton&, const juce::String&);
    void drawPanel(juce::Graphics&, juce::Rectangle<int>, const juce::String&, const juce::String&);
    void drawBee(juce::Graphics&, juce::Rectangle<float>);
    void drawMeter(juce::Graphics&, juce::Rectangle<float>, const juce::String&, float, float);

    HeritageChannelAudioProcessor& processor;
    HeritageLookAndFeel look;
    juce::Slider drive, lowGain, midGain, highGain;
    juce::Slider threshold, amount, compMakeup;
    juce::Slider tapeSat, tapeBias, masterMix, output;
    juce::ComboBox hpf, lowFreq, midFreq, midQ, eqCharacter, timeConstant, tapeType, tapeSpeed;
    juce::ToggleButton preampOn, eqOn, compOn, tapeOn, tapeNoise, channelOn;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboAttachment>> comboAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;
    float displayedGR = 0.0f, displayedPeak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeritageChannelAudioProcessorEditor)
};
