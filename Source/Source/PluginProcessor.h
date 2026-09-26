#pragma once

#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DSP.h"

inline juce::String bandParamId (int band, const juce::String& what)
{
    return "b" + juce::String (band + 1) + "_" + what;
}

class PrismaEQAudioProcessor : public juce::AudioProcessor
{
public:
    struct BandSettings
    {
        int type = 0;
        float freq = 1000.0f;
        float gain = 0.0f;
        float q = 1.0f;
        bool on = true;
    };

    PrismaEQAudioProcessor();
    ~PrismaEQAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Prisma EQ"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    BandSettings getBand (int index) const;

    juce::AudioProcessorValueTreeState apvts;

    // Datos que el editor lee: medidores, auto gain y analizador de espectro
    std::atomic<float> inPeakL { 0.0f }, inPeakR { 0.0f }, outPeakL { 0.0f }, outPeakR { 0.0f };
    std::atomic<float> autoGainDb { 0.0f };

    static constexpr int fifoSize = 16384;
    juce::AbstractFifo fifo { fifoSize };
    std::array<float, (size_t) fifoSize> fifoBuffer {};

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    struct BandParams
    {
        std::atomic<float>* freq = nullptr;
        std::atomic<float>* gain = nullptr;
        std::atomic<float>* q = nullptr;
        std::atomic<float>* on = nullptr;
        std::atomic<float>* type = nullptr;
    };

    BandParams bandParams[prisma::numBands];
    std::atomic<float>* pInGain = nullptr;
    std::atomic<float>* pOutGain = nullptr;
    std::atomic<float>* pAutoGain = nullptr;

    prisma::BandState states[2][prisma::numBands];

    double currentSampleRate = 44100.0;
    double avgIn = 0.0, avgEq = 0.0;
    double compGain = 1.0;
    double lastOutGain = 1.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrismaEQAudioProcessor)
};
