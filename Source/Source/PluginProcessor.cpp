#include "PluginProcessor.h"
#include "PluginEditor.h"

PrismaEQAudioProcessor::PrismaEQAudioProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETROS", createLayout())
{
    for (int i = 0; i < prisma::numBands; ++i)
    {
        auto& bp = bandParams[i];
        bp.freq = apvts.getRawParameterValue (bandParamId (i, "freq"));
        bp.gain = apvts.getRawParameterValue (bandParamId (i, "gain"));
        bp.q    = apvts.getRawParameterValue (bandParamId (i, "q"));
        bp.on   = apvts.getRawParameterValue (bandParamId (i, "on"));
        bp.type = apvts.getRawParameterValue (bandParamId (i, "type"));
    }

    pInGain   = apvts.getRawParameterValue ("in_gain");
    pOutGain  = apvts.getRawParameterValue ("out_gain");
    pAutoGain = apvts.getRawParameterValue ("auto_gain");
}

juce::AudioProcessorValueTreeState::ParameterLayout PrismaEQAudioProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const float defFreq[prisma::numBands] = { 60.0f, 200.0f, 800.0f, 2500.0f, 6000.0f, 12000.0f };
    const float defQ[prisma::numBands]    = { 0.7f, 1.0f, 1.0f, 1.0f, 1.0f, 0.7f };
    const int defType[prisma::numBands]   = { prisma::LowShelf, prisma::Bell, prisma::Bell,
                                              prisma::Bell, prisma::Bell, prisma::HighShelf };

    const juce::StringArray typeNames { "Campana", "Shelf grave", "Shelf agudo", "Paso alto", "Paso bajo" };

    for (int i = 0; i < prisma::numBands; ++i)
    {
        const auto n = juce::String (i + 1);

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { bandParamId (i, "freq"), 1 },
            "Banda " + n + " frecuencia",
            juce::NormalisableRange<float> (20.0f, 20000.0f, 0.0f, 0.3f),
            defFreq[i],
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction ([] (float v, int)
                {
                    return v >= 1000.0f ? juce::String (v / 1000.0f, 2) + " kHz"
                                        : juce::String (juce::roundToInt (v)) + " Hz";
                })
                .withValueFromStringFunction ([] (const juce::String& t)
                {
                    const float v = t.getFloatValue();
                    return t.containsIgnoreCase ("k") ? v * 1000.0f : v;
                })));

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { bandParamId (i, "gain"), 1 },
            "Banda " + n + " ganancia",
            juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f),
            0.0f,
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " dB"; })));

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { bandParamId (i, "q"), 1 },
            "Banda " + n + " Q",
            juce::NormalisableRange<float> (0.1f, 10.0f, 0.0f, 0.4f),
            defQ[i],
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 2); })));

        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { bandParamId (i, "type"), 1 },
            "Banda " + n + " tipo",
            typeNames,
            defType[i]));

        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { bandParamId (i, "on"), 1 },
            "Banda " + n + " activa",
            true));
    }

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "in_gain", 1 }, "Entrada",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " dB"; })));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "out_gain", 1 }, "Salida",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " dB"; })));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "auto_gain", 1 }, "Auto gain", false));

    return layout;
}

PrismaEQAudioProcessor::BandSettings PrismaEQAudioProcessor::getBand (int index) const
{
    const auto& bp = bandParams[index];

    BandSettings s;
    s.type = juce::roundToInt (bp.type->load());
    s.freq = bp.freq->load();
    s.gain = bp.gain->load();
    s.q    = bp.q->load();
    s.on   = bp.on->load() > 0.5f;
    return s;
}

void PrismaEQAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

    for (auto& ch : states)
        for (auto& s : ch)
            s = prisma::BandState();

    avgIn = 0.0;
    avgEq = 0.0;
    compGain = 1.0;
    lastOutGain = 1.0;
}

bool PrismaEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

void PrismaEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();

    for (int c = numCh; c < buffer.getNumChannels(); ++c)
        buffer.clear (c, 0, n);

    if (numCh == 0 || n == 0)
        return;

    auto hold = [] (std::atomic<float>& a, float v)
    {
        if (v > a.load())
            a.store (v);
    };

    // 1. Ganancia de entrada y medidor de entrada
    const float inGain = juce::Decibels::decibelsToGain (pInGain->load());
    double sumIn = 0.0;

    for (int c = 0; c < numCh; ++c)
    {
        buffer.applyGain (c, 0, n, inGain);

        const float* d = buffer.getReadPointer (c);
        float peak = 0.0f;

        for (int i = 0; i < n; ++i)
        {
            peak = juce::jmax (peak, std::abs (d[i]));
            sumIn += (double) d[i] * (double) d[i];
        }

        hold (c == 0 ? inPeakL : inPeakR, peak);
    }

    // 2. Coeficientes de las seis bandas
    prisma::Coeffs cf[prisma::numBands];
    bool active[prisma::numBands];

    for (int b = 0; b < prisma::numBands; ++b)
    {
        const auto s = getBand (b);
        cf[b] = prisma::makeCoeffs (s.type, currentSampleRate, s.freq, s.gain, s.q);

        const bool needsGain = s.type == prisma::Bell || s.type == prisma::LowShelf || s.type == prisma::HighShelf;
        active[b] = s.on && (! needsGain || std::abs (s.gain) > 0.01f);
    }

    // 3. Ecualizador
    double sumEq = 0.0;

    for (int c = 0; c < numCh; ++c)
    {
        float* d = buffer.getWritePointer (c);

        for (int i = 0; i < n; ++i)
        {
            double x = (double) d[i];

            for (int b = 0; b < prisma::numBands; ++b)
                if (active[b])
                    x = prisma::process (cf[b], states[c][b], x);

            d[i] = (float) x;
            sumEq += x * x;
        }
    }

    // 4. Auto gain: iguala el nivel (RMS suavizado) de salida al de entrada
    const bool autoOn = pAutoGain->load() > 0.5f;
    const double denom = (double) n * (double) numCh;
    const double k = 1.0 - std::exp (-(double) n / (0.4 * currentSampleRate));

    avgIn += k * (sumIn / denom - avgIn);
    avgEq += k * (sumEq / denom - avgEq);

    if (avgIn > 1.0e-10 && avgEq > 1.0e-10)
        compGain = juce::jlimit (0.25, 4.0, std::sqrt (avgIn / avgEq));

    autoGainDb.store ((float) juce::Decibels::gainToDecibels (compGain));

    const double targetOut = autoOn ? compGain
                                    : (double) juce::Decibels::decibelsToGain (pOutGain->load());

    const float g0 = (float) lastOutGain;
    const float g1 = (float) targetOut;
    lastOutGain = targetOut;

    // 5. Ganancia de salida (con rampa) y medidor de salida
    for (int c = 0; c < numCh; ++c)
    {
        float* d = buffer.getWritePointer (c);
        float peak = 0.0f;

        for (int i = 0; i < n; ++i)
        {
            d[i] *= g0 + (g1 - g0) * (float) i / (float) n;
            peak = juce::jmax (peak, std::abs (d[i]));
        }

        hold (c == 0 ? outPeakL : outPeakR, peak);
    }

    // 6. Muestras para el analizador de espectro
    int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
    fifo.prepareToWrite (n, start1, size1, start2, size2);

    const float* l = buffer.getReadPointer (0);
    const float* r = buffer.getReadPointer (numCh > 1 ? 1 : 0);

    for (int i = 0; i < size1; ++i)
        fifoBuffer[(size_t) (start1 + i)] = 0.5f * (l[i] + r[i]);

    for (int i = 0; i < size2; ++i)
        fifoBuffer[(size_t) (start2 + i)] = 0.5f * (l[size1 + i] + r[size1 + i]);

    fifo.finishedWrite (size1 + size2);
}

juce::AudioProcessorEditor* PrismaEQAudioProcessor::createEditor()
{
    return new PrismaEQAudioProcessorEditor (*this);
}

void PrismaEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PrismaEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PrismaEQAudioProcessor();
}
