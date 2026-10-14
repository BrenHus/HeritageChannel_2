#include "PluginEditor.h"

namespace
{
const auto background = juce::Colour(0xff0e1112);
const auto face = juce::Colour(0xff1b2021);
const auto inset = juce::Colour(0xff111516);
const auto brass = juce::Colour(0xffc79a52);
const auto brassDark = juce::Colour(0xff765a32);
const auto cream = juce::Colour(0xffeee5cf);
const auto muted = juce::Colour(0xff989b92);
const auto red = juce::Colour(0xff9f5144);
}

HeritageLookAndFeel::HeritageLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, cream);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::textColourId, cream);
    setColour(juce::ComboBox::outlineColourId, brassDark);
    setColour(juce::PopupMenu::backgroundColourId, face);
    setColour(juce::PopupMenu::textColourId, cream);
}

void HeritageLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float position, float start, float end, juce::Slider&)
{
    auto a = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                    static_cast<float>(width), static_cast<float>(height)).reduced(9.0f);
    const auto radius = juce::jmin(a.getWidth(), a.getHeight()) * 0.5f;
    const auto centre = a.getCentre();
    const auto angle = start + position * (end - start);
    g.setColour(juce::Colour(0xff080a0b));
    g.fillEllipse(a.withSizeKeepingCentre(radius * 2.0f, radius * 2.0f));
    g.setColour(juce::Colour(0xff313737));
    g.drawEllipse(a.withSizeKeepingCentre(radius * 2.0f, radius * 2.0f), 2.0f);

    juce::Path scale;
    scale.addCentredArc(centre.x, centre.y, radius + 3.0f, radius + 3.0f, 0.0f, start, end, true);
    g.setColour(brassDark.withAlpha(0.8f));
    g.strokePath(scale, juce::PathStrokeType(1.2f));
    juce::Path active;
    active.addCentredArc(centre.x, centre.y, radius + 3.0f, radius + 3.0f, 0.0f, start, angle, true);
    g.setColour(brass);
    g.strokePath(active, juce::PathStrokeType(2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path pointer;
    pointer.addRoundedRectangle(-1.4f, -radius + 7.0f, 2.8f, radius * 0.52f, 1.4f);
    g.setColour(cream);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

void HeritageLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(b.getToggleState() ? brass : juce::Colour(0xff282d2e));
    g.fillRoundedRectangle(r, 3.0f);
    g.setColour(b.getToggleState() ? background : muted);
    g.setFont(juce::Font(10.5f, juce::Font::bold));
    g.drawText(b.getButtonText(), r, juce::Justification::centred);
}

void HeritageLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                        int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    g.setColour(inset); g.fillRoundedRectangle(r, 3.0f);
    g.setColour(brassDark); g.drawRoundedRectangle(r, 3.0f, 1.0f);
    juce::Path arrow;
    arrow.addTriangle(static_cast<float>(width - 15), height * 0.42f,
                      static_cast<float>(width - 7), height * 0.42f,
                      static_cast<float>(width - 11), height * 0.62f);
    g.setColour(brass); g.fillPath(arrow);
}

HeritageChannelAudioProcessorEditor::HeritageChannelAudioProcessorEditor(HeritageChannelAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&look);
    setSize(1180, 650);
    setResizable(false, false);

    configureKnob(drive, "INPUT DRIVE"); configureKnob(lowGain, "LOW GAIN");
    configureKnob(midGain, "MID GAIN"); configureKnob(highGain, "PRESENCE");
    configureKnob(threshold, "THRESHOLD"); configureKnob(amount, "COMPRESSION");
    configureKnob(compMakeup, "MAKE-UP"); configureKnob(tapeSat, "SATURATION");
    configureKnob(tapeBias, "BIAS"); configureKnob(masterMix, "MIX"); configureKnob(output, "OUTPUT");

    configureCombo(hpf, "HPF", { "Off", "50 Hz", "80 Hz", "160 Hz", "300 Hz" });
    configureCombo(lowFreq, "LOW FREQ", { "35 Hz", "60 Hz", "110 Hz", "220 Hz" });
    configureCombo(midFreq, "MID FREQUENCY", { "360 Hz", "700 Hz", "1.6 kHz", "3.2 kHz", "4.8 kHz", "7.2 kHz" });
    configureCombo(midQ, "MID Q", { "Broad", "Tight" });
    configureCombo(eqCharacter, "EQ CHARACTER", { "Classic", "Clean" });
    configureCombo(timeConstant, "RECOVERY", { "1 Fast", "2", "3", "4 Slow", "5 Auto", "6 Auto" });
    configureCombo(tapeType, "FORMULA", { "250", "456", "900" });
    configureCombo(tapeSpeed, "SPEED", { "7.5 ips", "15 ips", "30 ips" });
    configureToggle(preampOn, "INPUT"); configureToggle(eqOn, "EQ ENGAGED");
    configureToggle(compOn, "COMP IN"); configureToggle(tapeOn, "TAPE IN");
    configureToggle(tapeNoise, "HISS"); configureToggle(channelOn, "CHANNEL");

    auto& s = processor.state;
    auto sa = [this, &s](juce::Slider& c, const char* id)
    { sliderAttachments.push_back(std::make_unique<SliderAttachment>(s, id, c)); };
    sa(drive, "inputDrive"); sa(lowGain, "lowGain"); sa(midGain, "midGain"); sa(highGain, "highGain");
    sa(threshold, "compThreshold"); sa(amount, "compAmount"); sa(compMakeup, "compMakeup");
    sa(tapeSat, "tapeSaturation"); sa(tapeBias, "tapeBias"); sa(masterMix, "masterMix"); sa(output, "output");
    auto ca = [this, &s](juce::ComboBox& c, const char* id)
    { comboAttachments.push_back(std::make_unique<ComboAttachment>(s, id, c)); };
    ca(hpf, "hpf"); ca(lowFreq, "lowFreq"); ca(midFreq, "midFreq"); ca(midQ, "midQ");
    ca(eqCharacter, "eqCharacter"); ca(timeConstant, "timeConstant"); ca(tapeType, "tapeType"); ca(tapeSpeed, "tapeSpeed");
    auto ba = [this, &s](juce::ToggleButton& c, const char* id)
    { buttonAttachments.push_back(std::make_unique<ButtonAttachment>(s, id, c)); };
    ba(preampOn, "preampOn"); ba(eqOn, "eqOn"); ba(compOn, "compOn");
    ba(tapeOn, "tapeOn"); ba(tapeNoise, "tapeNoise"); ba(channelOn, "channelOn");
    startTimerHz(30);
}

HeritageChannelAudioProcessorEditor::~HeritageChannelAudioProcessorEditor() { setLookAndFeel(nullptr); }

void HeritageChannelAudioProcessorEditor::configureKnob(juce::Slider& s, const juce::String& name)
{
    s.setName(name); s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 17); addAndMakeVisible(s);
}

void HeritageChannelAudioProcessorEditor::configureCombo(juce::ComboBox& c, const juce::String& name,
                                                           const juce::StringArray& items)
{
    c.setName(name); c.addItemList(items, 1); c.setJustificationType(juce::Justification::centred); addAndMakeVisible(c);
}

void HeritageChannelAudioProcessorEditor::configureToggle(juce::ToggleButton& b, const juce::String& text)
{
    b.setButtonText(text); addAndMakeVisible(b);
}

void HeritageChannelAudioProcessorEditor::drawPanel(juce::Graphics& g, juce::Rectangle<int> r,
                                                      const juce::String& title, const juce::String& subtitle)
{
    g.setColour(face); g.fillRoundedRectangle(r.toFloat(), 5.0f);
    g.setColour(juce::Colour(0xff33393a)); g.drawRoundedRectangle(r.toFloat(), 5.0f, 1.0f);
    auto top = r.removeFromTop(44);
    g.setColour(brass); g.fillRect(top.removeFromBottom(2).reduced(12, 0));
    g.setColour(cream); g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText(title, top.reduced(13, 3), juce::Justification::centredLeft);
    g.setColour(muted); g.setFont(juce::Font(9.5f));
    g.drawText(subtitle, top.reduced(13, 3), juce::Justification::centredRight);
}

void HeritageChannelAudioProcessorEditor::drawBee(juce::Graphics& g, juce::Rectangle<float> r)
{
    const auto c = r.getCentre();
    g.setColour(brass);
    g.drawEllipse(c.x - 25.0f, c.y - 18.0f, 23.0f, 20.0f, 2.0f);
    g.drawEllipse(c.x + 2.0f, c.y - 18.0f, 23.0f, 20.0f, 2.0f);
    juce::Path body;
    body.startNewSubPath(c.x, c.y - 11.0f); body.lineTo(c.x + 12.0f, c.y - 3.0f);
    body.lineTo(c.x + 9.0f, c.y + 14.0f); body.lineTo(c.x, c.y + 21.0f);
    body.lineTo(c.x - 9.0f, c.y + 14.0f); body.lineTo(c.x - 12.0f, c.y - 3.0f); body.closeSubPath();
    g.fillPath(body);
    g.setColour(background); g.fillRect(c.x - 9.0f, c.y - 1.0f, 18.0f, 3.0f); g.fillRect(c.x - 8.0f, c.y + 7.0f, 16.0f, 3.0f);
    g.setColour(brass);
    for (int side : { -1, 1 })
    {
        g.drawLine(c.x + side * 6.0f, c.y + 2.0f, c.x + side * 22.0f, c.y + 9.0f, 1.7f);
        g.drawLine(c.x + side * 6.0f, c.y + 9.0f, c.x + side * 20.0f, c.y + 20.0f, 1.7f);
        g.drawLine(c.x + side * 4.0f, c.y - 9.0f, c.x + side * 14.0f, c.y - 23.0f, 1.5f);
    }
}

void HeritageChannelAudioProcessorEditor::drawMeter(juce::Graphics& g, juce::Rectangle<float> r,
                                                      const juce::String& label, float value, float maximum)
{
    g.setColour(juce::Colour(0xffd9c89e)); g.fillRoundedRectangle(r, 4.0f);
    g.setColour(juce::Colour(0xff7d714e)); g.drawRoundedRectangle(r, 4.0f, 1.0f);
    auto inner = r.reduced(8.0f, 5.0f);
    g.setColour(juce::Colour(0xff574d34));
    for (int i = 0; i <= 8; ++i)
    {
        const auto x = inner.getX() + inner.getWidth() * static_cast<float>(i) / 8.0f;
        g.drawLine(x, inner.getY() + 2.0f, x, inner.getY() + 7.0f, 1.0f);
    }
    const auto norm = juce::jlimit(0.0f, 1.0f, value / maximum);
    const auto pivot = juce::Point<float>(r.getCentreX(), r.getBottom() + 18.0f);
    const auto angle = juce::MathConstants<float>::pi * (1.23f + 0.54f * norm);
    const auto tip = pivot.getPointOnCircumference(r.getWidth() * 0.54f, angle);
    g.setColour(red); g.drawLine(pivot.x, pivot.y, tip.x, tip.y, 1.7f);
    g.setColour(juce::Colour(0xff332d20)); g.setFont(juce::Font(9.5f, juce::Font::bold));
    g.drawText(label, r.removeFromBottom(17.0f), juce::Justification::centred);
}

void HeritageChannelAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);
    juce::ColourGradient glow(brass.withAlpha(0.08f), 590.0f, 0.0f, background, 590.0f, 280.0f, false);
    g.setGradientFill(glow); g.fillRect(getLocalBounds());
    drawBee(g, { 28.0f, 11.0f, 76.0f, 62.0f });
    g.setColour(brass); g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.drawText("B.E.E", 105, 15, 110, 25, juce::Justification::centredLeft);
    g.setColour(muted); g.setFont(juce::Font(9.0f));
    g.drawText("BREN'S ELECTRONIC EFFECTS", 106, 39, 180, 15, juce::Justification::centredLeft);
    g.setColour(cream); g.setFont(juce::Font(27.0f, juce::Font::bold));
    g.drawText("HERITAGE", 420, 12, 340, 32, juce::Justification::centred);
    g.setColour(brass); g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("ANALOGUE CHANNEL  •  MODEL 01", 420, 43, 340, 16, juce::Justification::centred);
    g.setColour(muted); g.setFont(juce::Font(9.0f));
    g.drawText("DESIGNED IN SUSSEX", 940, 22, 210, 18, juce::Justification::centredRight);

    drawPanel(g, { 18, 78, 505, 482 }, "CONSOLE EQ", "DISCRETE INPUT / MUSICAL EQ");
    drawPanel(g, { 535, 78, 286, 482 }, "VALVE COMPRESSOR", "VARIABLE-MU");
    drawPanel(g, { 833, 78, 329, 482 }, "TAPE & MASTER", "RECORD / REPRO");
    auto label = [&g](juce::Component& c)
    {
        g.setColour(muted); g.setFont(juce::Font(9.2f, juce::Font::bold));
        g.drawText(c.getName(), c.getX() - 8, c.getY() - 15, c.getWidth() + 16, 13, juce::Justification::centred);
    };
    for (auto* c : { &drive, &lowGain, &midGain, &highGain, &threshold, &amount, &compMakeup,
                     &tapeSat, &tapeBias, &masterMix, &output }) label(*c);
    for (auto* c : { &hpf, &lowFreq, &midFreq, &midQ, &eqCharacter, &timeConstant, &tapeType, &tapeSpeed }) label(*c);
    drawMeter(g, { 560.0f, 130.0f, 236.0f, 82.0f }, "GAIN REDUCTION", displayedGR, 20.0f);
    drawMeter(g, { 858.0f, 130.0f, 144.0f, 72.0f }, "RECORD LEVEL", displayedPeak, 1.0f);

    g.setColour(juce::Colour(0xff2d3334)); g.fillRoundedRectangle(18.0f, 574.0f, 1144.0f, 52.0f, 4.0f);
    g.setColour(brassDark); g.drawLine(45.0f, 600.0f, 1135.0f, 600.0f, 1.0f);
    g.setColour(cream); g.setFont(juce::Font(9.5f, juce::Font::bold));
    g.drawText("INPUT TRANSFORMER", 30, 581, 150, 18, juce::Justification::centred);
    g.drawText("CONSOLE EQ", 245, 581, 110, 18, juce::Justification::centred);
    g.drawText("VALVE COMPRESSION", 500, 581, 150, 18, juce::Justification::centred);
    g.drawText("TAPE RECORD / REPRO", 780, 581, 165, 18, juce::Justification::centred);
    g.drawText("MASTER", 1040, 581, 90, 18, juce::Justification::centred);
    g.setColour(muted); g.setFont(juce::Font(9.0f));
    g.drawText("B.E.E  •  HERITAGE CHANNEL 0.2  •  2× OVERSAMPLING", 18, 628, 1144, 16, juce::Justification::centred);
}

void HeritageChannelAudioProcessorEditor::resized()
{
    drive.setBounds(43, 145, 100, 112); hpf.setBounds(160, 160, 106, 27); lowFreq.setBounds(284, 160, 110, 27);
    preampOn.setBounds(412, 151, 88, 25); eqOn.setBounds(412, 183, 88, 25);
    lowGain.setBounds(48, 294, 98, 112); midGain.setBounds(166, 294, 98, 112); highGain.setBounds(284, 294, 98, 112);
    midFreq.setBounds(52, 454, 132, 28); midQ.setBounds(200, 454, 96, 28); eqCharacter.setBounds(312, 454, 158, 28);
    threshold.setBounds(558, 262, 82, 108); amount.setBounds(651, 262, 82, 108);
    compMakeup.setBounds(716, 403, 82, 108); timeConstant.setBounds(558, 424, 132, 29); compOn.setBounds(560, 487, 112, 26);
    tapeType.setBounds(1022, 139, 112, 28); tapeSpeed.setBounds(1022, 181, 112, 28);
    tapeSat.setBounds(850, 245, 92, 108); tapeBias.setBounds(951, 245, 92, 108);
    tapeOn.setBounds(1054, 253, 82, 25); tapeNoise.setBounds(1054, 287, 82, 25);
    masterMix.setBounds(865, 407, 92, 108); output.setBounds(975, 407, 92, 108); channelOn.setBounds(1073, 449, 70, 43);
}

void HeritageChannelAudioProcessorEditor::timerCallback()
{
    displayedGR += 0.20f * (processor.getGainReductionDb() - displayedGR);
    const auto peak = juce::jlimit(0.0f, 1.2f, processor.getOutputPeak());
    displayedPeak = peak > displayedPeak ? peak : displayedPeak * 0.92f;
    repaint(548, 120, 470, 102);
}
