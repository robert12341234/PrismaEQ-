#pragma once

#include <array>
#include <memory>
#include <vector>
#include "PluginProcessor.h"
#include "Presets.h"

class PrismaEQAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit PrismaEQAudioProcessorEditor (PrismaEQAudioProcessor&);
    ~PrismaEQAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    static constexpr int fftOrder = 12;
    static constexpr int fftSize = 1 << fftOrder;

    void timerCallback() override;
    void selectBand (int index);
    void updateAnalyzer();

    float freqToX (float f) const;
    float xToFreq (float x) const;
    float gainToY (float g) const;
    float yToGain (float y) const;
    juce::Point<float> getNodePos (int band) const;
    int findNodeAt (juce::Point<float> p, float maxDist) const;

    void beginGesture (const juce::String& id);
    void endGesture (const juce::String& id);
    void setParam (const juce::String& id, float value);

    void buildPresetMenu();
    void applyPreset (int presetIndex);
    void savePresetToFile();
    void loadPresetFromFile();
    void setActiveSide (bool sideB);

    PrismaEQAudioProcessor& proc;

    int selectedBand = 3;
    int draggingBand = -1;
    int row2Top = 0;
    juce::Rectangle<float> displayArea, plotArea;

    float inLevel[2] = { -100.0f, -100.0f };
    float outLevel[2] = { -100.0f, -100.0f };

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, (size_t) fftSize> timeBuf {};
    std::array<float, (size_t) fftSize * 2> fftData {};
    std::array<float, (size_t) fftSize / 2> smoothDb {};
    std::vector<float> scratch;

    std::array<juce::TextButton, 6> bandButtons;
    juce::ComboBox typeBox;
    juce::ToggleButton bandOn;
    juce::Slider freqKnob, gainKnob, qKnob;
    juce::Label freqName, gainName, qName;
    juce::Slider inSlider, outSlider;
    juce::Label inName, outName, inValue, outValue;
    juce::TextButton autoButton;

    std::unique_ptr<SliderAttachment> freqAtt, gainAtt, qAtt, inAtt, outAtt;
    std::unique_ptr<ComboBoxAttachment> typeAtt;
    std::unique_ptr<ButtonAttachment> onAtt, autoAtt;

    juce::ComboBox presetBox;
    juce::TextButton saveButton, loadButton, aButton, bButton;
    juce::MemoryBlock stateA, stateB;
    bool onSideB = false;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrismaEQAudioProcessorEditor)
};
