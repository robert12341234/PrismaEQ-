#include "PluginEditor.h"
#include <cmath>

namespace
{
    const juce::Colour bandColours[prisma::numBands] =
    {
        juce::Colour (0xff7F77DD), juce::Colour (0xff1D9E75), juce::Colour (0xffD85A30),
        juce::Colour (0xffD4537E), juce::Colour (0xff378ADD), juce::Colour (0xffBA7517)
    };

    const juce::Colour colBg      (0xff232322);
    const juce::Colour colDisplay (0xff161615);
    const juce::Colour colText    (0xffF1EFE8);
    const juce::Colour colMuted   (0xffB4B2A9);
    const juce::Colour colDim     (0xff888780);
    const juce::Colour colGrid    (0xff3a3a37);
    const juce::Colour colPanel   (0xff2C2C2A);
    const juce::Colour colLine    (0xff444441);
    const juce::Colour colPink    (0xffD4537E);
    const juce::Colour colGreen   (0xff639922);
    const juce::Colour colAmber   (0xffEF9F27);
    const juce::Colour colRed     (0xffE24B4A);

    juce::String formatFreq (float f)
    {
        return f >= 1000.0f ? juce::String (f / 1000.0f, 2) + " kHz"
                            : juce::String (juce::roundToInt (f)) + " Hz";
    }
}

PrismaEQAudioProcessorEditor::PrismaEQAudioProcessorEditor (PrismaEQAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p)
{
    smoothDb.fill (-100.0f);
    scratch.resize ((size_t) PrismaEQAudioProcessor::fifoSize);

    for (int i = 0; i < prisma::numBands; ++i)
    {
        auto& b = bandButtons[(size_t) i];
        b.setButtonText (juce::String (i + 1));
        b.setColour (juce::TextButton::buttonColourId, colPanel);
        b.setColour (juce::TextButton::buttonOnColourId, bandColours[i]);
        b.setColour (juce::TextButton::textColourOffId, colText);
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        b.onClick = [this, i] { selectBand (i); };
        addAndMakeVisible (b);
    }

    typeBox.addItemList ({ "Campana", "Shelf grave", "Shelf agudo", "Paso alto", "Paso bajo" }, 1);
    typeBox.setColour (juce::ComboBox::backgroundColourId, colPanel);
    typeBox.setColour (juce::ComboBox::textColourId, colText);
    typeBox.setColour (juce::ComboBox::outlineColourId, colLine);
    typeBox.setColour (juce::ComboBox::arrowColourId, colMuted);
    addAndMakeVisible (typeBox);

    bandOn.setButtonText ("Activa");
    bandOn.setColour (juce::ToggleButton::textColourId, colText);
    bandOn.setColour (juce::ToggleButton::tickColourId, colPink);
    addAndMakeVisible (bandOn);

    auto setupKnob = [this] (juce::Slider& s, juce::Label& l, const juce::String& name)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 18);
        s.setColour (juce::Slider::rotarySliderFillColourId, colPink);
        s.setColour (juce::Slider::rotarySliderOutlineColourId, colLine);
        s.setColour (juce::Slider::thumbColourId, colText);
        s.setColour (juce::Slider::textBoxTextColourId, colText);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (s);

        l.setText (name, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, colMuted);
        addAndMakeVisible (l);
    };

    setupKnob (freqKnob, freqName, "Frecuencia");
    setupKnob (gainKnob, gainName, "Ganancia");
    setupKnob (qKnob, qName, "Q");

    auto setupGain = [this] (juce::Slider& s, juce::Label& name, juce::Label& value,
                             const juce::String& text, juce::Colour track)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setColour (juce::Slider::trackColourId, track);
        s.setColour (juce::Slider::backgroundColourId, colLine);
        s.setColour (juce::Slider::thumbColourId, colText);
        addAndMakeVisible (s);

        name.setText (text, juce::dontSendNotification);
        name.setColour (juce::Label::textColourId, colMuted);
        addAndMakeVisible (name);

        value.setJustificationType (juce::Justification::centredRight);
        value.setColour (juce::Label::textColourId, colText);
        addAndMakeVisible (value);
    };

    setupGain (inSlider, inName, inValue, "Entrada", colDim);
    setupGain (outSlider, outName, outValue, "Salida", colPink);

    autoButton.setButtonText ("Auto gain");
    autoButton.setClickingTogglesState (true);
    autoButton.setColour (juce::TextButton::buttonColourId, colPanel);
    autoButton.setColour (juce::TextButton::buttonOnColourId, colPink);
    autoButton.setColour (juce::TextButton::textColourOffId, colText);
    autoButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    addAndMakeVisible (autoButton);

    inAtt   = std::make_unique<SliderAttachment> (proc.apvts, "in_gain", inSlider);
    outAtt  = std::make_unique<SliderAttachment> (proc.apvts, "out_gain", outSlider);
    autoAtt = std::make_unique<ButtonAttachment> (proc.apvts, "auto_gain", autoButton);

    selectBand (3);

    setResizable (true, true);
    setResizeLimits (640, 420, 1800, 1200);
    setSize (900, 560);

    startTimerHz (30);
}

PrismaEQAudioProcessorEditor::~PrismaEQAudioProcessorEditor()
{
    stopTimer();
}

void PrismaEQAudioProcessorEditor::selectBand (int index)
{
    selectedBand = index;

    freqAtt.reset();
    gainAtt.reset();
    qAtt.reset();
    typeAtt.reset();
    onAtt.reset();

    freqAtt = std::make_unique<SliderAttachment> (proc.apvts, bandParamId (index, "freq"), freqKnob);
    gainAtt = std::make_unique<SliderAttachment> (proc.apvts, bandParamId (index, "gain"), gainKnob);
    qAtt    = std::make_unique<SliderAttachment> (proc.apvts, bandParamId (index, "q"), qKnob);
    typeAtt = std::make_unique<ComboBoxAttachment> (proc.apvts, bandParamId (index, "type"), typeBox);
    onAtt   = std::make_unique<ButtonAttachment> (proc.apvts, bandParamId (index, "on"), bandOn);

    for (int i = 0; i < prisma::numBands; ++i)
        bandButtons[(size_t) i].setToggleState (i == index, juce::dontSendNotification);

    const auto c = bandColours[index];
    for (auto* s : { &freqKnob, &gainKnob, &qKnob })
        s->setColour (juce::Slider::rotarySliderFillColourId, c);

    repaint();
}

float PrismaEQAudioProcessorEditor::freqToX (float f) const
{
    return plotArea.getX() + plotArea.getWidth() * std::log10 (f / 20.0f) / 3.0f;
}

float PrismaEQAudioProcessorEditor::xToFreq (float x) const
{
    const float t = juce::jlimit (0.0f, 1.0f, (x - plotArea.getX()) / plotArea.getWidth());
    return 20.0f * std::pow (1000.0f, t);
}

float PrismaEQAudioProcessorEditor::gainToY (float g) const
{
    return plotArea.getCentreY() - g * (plotArea.getHeight() / 24.0f);
}

float PrismaEQAudioProcessorEditor::yToGain (float y) const
{
    return juce::jlimit (-12.0f, 12.0f, (plotArea.getCentreY() - y) * 24.0f / plotArea.getHeight());
}

juce::Point<float> PrismaEQAudioProcessorEditor::getNodePos (int band) const
{
    const auto s = proc.getBand (band);
    const float gain = s.type >= prisma::HighPass ? 0.0f : s.gain;
    return { freqToX (s.freq), gainToY (gain) };
}

int PrismaEQAudioProcessorEditor::findNodeAt (juce::Point<float> p, float maxDist) const
{
    int best = -1;
    float bestDist = maxDist;

    for (int b = 0; b < prisma::numBands; ++b)
    {
        const float d = getNodePos (b).getDistanceFrom (p);

        if (d <= bestDist)
        {
            bestDist = d;
            best = b;
        }
    }

    return best;
}

void PrismaEQAudioProcessorEditor::beginGesture (const juce::String& id)
{
    if (auto* prm = proc.apvts.getParameter (id))
        prm->beginChangeGesture();
}

void PrismaEQAudioProcessorEditor::endGesture (const juce::String& id)
{
    if (auto* prm = proc.apvts.getParameter (id))
        prm->endChangeGesture();
}

void PrismaEQAudioProcessorEditor::setParam (const juce::String& id, float value)
{
    if (auto* prm = proc.apvts.getParameter (id))
        prm->setValueNotifyingHost (prm->convertTo0to1 (value));
}

void PrismaEQAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    draggingBand = findNodeAt (e.position, 16.0f);

    if (draggingBand >= 0)
    {
        selectBand (draggingBand);
        beginGesture (bandParamId (draggingBand, "freq"));
        beginGesture (bandParamId (draggingBand, "gain"));
    }
}

void PrismaEQAudioProcessorEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (draggingBand < 0)
        return;

    setParam (bandParamId (draggingBand, "freq"), juce::jlimit (20.0f, 20000.0f, xToFreq (e.position.x)));

    if (proc.getBand (draggingBand).type < prisma::HighPass)
        setParam (bandParamId (draggingBand, "gain"), yToGain (e.position.y));
}

void PrismaEQAudioProcessorEditor::mouseUp (const juce::MouseEvent&)
{
    if (draggingBand >= 0)
    {
        endGesture (bandParamId (draggingBand, "freq"));
        endGesture (bandParamId (draggingBand, "gain"));
        draggingBand = -1;
    }
}

void PrismaEQAudioProcessorEditor::mouseDoubleClick (const juce::MouseEvent& e)
{
    const int idx = findNodeAt (e.position, 16.0f);

    if (idx >= 0)
    {
        const auto id = bandParamId (idx, "gain");
        beginGesture (id);
        setParam (id, 0.0f);
        endGesture (id);
    }
}

void PrismaEQAudioProcessorEditor::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    const int idx = findNodeAt (e.position, 24.0f);

    if (idx < 0)
        return;

    const auto id = bandParamId (idx, "q");
    const float q = juce::jlimit (0.1f, 10.0f, proc.getBand (idx).q * std::exp (wheel.deltaY * 0.6f));

    beginGesture (id);
    setParam (id, q);
    endGesture (id);
}

void PrismaEQAudioProcessorEditor::updateAnalyzer()
{
    const int ready = proc.fifo.getNumReady();

    if (ready > 0)
    {
        const int toRead = juce::jmin (ready, (int) scratch.size());
        int s1 = 0, z1 = 0, s2 = 0, z2 = 0;
        proc.fifo.prepareToRead (toRead, s1, z1, s2, z2);

        std::copy (proc.fifoBuffer.begin() + s1, proc.fifoBuffer.begin() + s1 + z1, scratch.begin());
        std::copy (proc.fifoBuffer.begin() + s2, proc.fifoBuffer.begin() + s2 + z2, scratch.begin() + z1);
        proc.fifo.finishedRead (z1 + z2);

        const int got = z1 + z2;

        if (got >= fftSize)
        {
            std::copy (scratch.begin() + (got - fftSize), scratch.begin() + got, timeBuf.begin());
        }
        else if (got > 0)
        {
            std::copy (timeBuf.begin() + got, timeBuf.end(), timeBuf.begin());
            std::copy (scratch.begin(), scratch.begin() + got, timeBuf.end() - got);
        }
    }

    std::copy (timeBuf.begin(), timeBuf.end(), fftData.begin());
    std::fill (fftData.begin() + fftSize, fftData.end(), 0.0f);

    window.multiplyWithWindowingTable (fftData.data(), (size_t) fftSize);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    for (int i = 0; i < fftSize / 2; ++i)
    {
        const float db = juce::Decibels::gainToDecibels (fftData[(size_t) i] * 4.0f / (float) fftSize, -120.0f);
        float& s = smoothDb[(size_t) i];
        s = db > s ? db : s * 0.85f + db * 0.15f;
    }
}

void PrismaEQAudioProcessorEditor::timerCallback()
{
    auto upd = [] (float& lvl, float peak)
    {
        lvl = juce::jmax (juce::Decibels::gainToDecibels (peak, -100.0f), lvl - 3.0f);
    };

    upd (inLevel[0], proc.inPeakL.exchange (0.0f));
    upd (inLevel[1], proc.inPeakR.exchange (0.0f));
    upd (outLevel[0], proc.outPeakL.exchange (0.0f));
    upd (outLevel[1], proc.outPeakR.exchange (0.0f));

    updateAnalyzer();

    const bool autoOn = autoButton.getToggleState();
    outSlider.setEnabled (! autoOn);

    inValue.setText (juce::String (inSlider.getValue(), 1) + " dB", juce::dontSendNotification);
    outValue.setText (autoOn ? juce::String (proc.autoGainDb.load(), 1) + " dB auto"
                             : juce::String (outSlider.getValue(), 1) + " dB",
                      juce::dontSendNotification);

    repaint();
}

void PrismaEQAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();

    const int dispH = juce::roundToInt ((float) b.getHeight() * 0.58f);
    displayArea = b.removeFromTop (dispH).toFloat();
    plotArea = displayArea.withTrimmedTop (40.0f).withTrimmedBottom (22.0f);

    auto controls = b.reduced (12, 8);
    const int row2H = juce::jmax (44, controls.getHeight() * 30 / 100);
    auto row2 = controls.removeFromBottom (row2H);
    row2Top = row2.getY() - 4;

    auto left = controls.removeFromLeft (controls.getWidth() * 45 / 100);
    auto right = controls;

    auto btnRow = left.removeFromTop (34);
    const int bw = btnRow.getWidth() / 6;

    for (int i = 0; i < prisma::numBands; ++i)
        bandButtons[(size_t) i].setBounds (btnRow.removeFromLeft (bw).reduced (2));

    left.removeFromTop (8);
    auto l2 = left.removeFromTop (28);
    typeBox.setBounds (l2.removeFromLeft (l2.getWidth() * 60 / 100).reduced (2, 0));
    bandOn.setBounds (l2.reduced (6, 0));

    const int kw = right.getWidth() / 3;

    auto place = [&] (juce::Slider& s, juce::Label& l)
    {
        auto col = right.removeFromLeft (kw);
        l.setBounds (col.removeFromTop (16));
        s.setBounds (col);
    };

    place (freqKnob, freqName);
    place (gainKnob, gainName);
    place (qKnob, qName);

    const int third = row2.getWidth() / 3;
    auto inArea = row2.removeFromLeft (third);
    auto autoArea = row2.removeFromLeft (third);
    auto outArea = row2;

    auto placeGain = [] (juce::Rectangle<int> area, juce::Label& name, juce::Slider& s, juce::Label& value)
    {
        name.setBounds (area.removeFromLeft (56));
        value.setBounds (area.removeFromRight (78));
        s.setBounds (area);
    };

    placeGain (inArea.reduced (4, 0), inName, inSlider, inValue);
    placeGain (outArea.reduced (4, 0), outName, outSlider, outValue);
    autoButton.setBounds (autoArea.reduced (10, 6));
}

void PrismaEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (colBg);
    g.setColour (colDisplay);
    g.fillRect (displayArea);

    const double sr = proc.getSampleRate() > 0.0 ? proc.getSampleRate() : 44100.0;
    const float left = plotArea.getX();
    const float right = plotArea.getRight();
    const float top = plotArea.getY();
    const float bottom = plotArea.getBottom();

    // Espectro
    {
        juce::Path a;
        a.startNewSubPath (left, bottom);

        for (float x = left; x <= right; x += 2.0f)
        {
            const int bin = juce::jlimit (1, fftSize / 2 - 1,
                                          (int) std::round (xToFreq (x) * (float) fftSize / (float) sr));
            const float db = juce::jlimit (-100.0f, -10.0f, smoothDb[(size_t) bin]);
            a.lineTo (x, juce::jmap (db, -100.0f, -10.0f, bottom, top));
        }

        a.lineTo (right, bottom);
        a.closeSubPath();
        g.setColour (colDim.withAlpha (0.28f));
        g.fillPath (a);
    }

    // Rejilla
    g.setColour (colGrid);

    for (float f : { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f })
        g.drawVerticalLine (juce::roundToInt (freqToX (f)), top, bottom);

    for (float d : { -12.0f, -6.0f, 6.0f, 12.0f })
        g.drawHorizontalLine (juce::roundToInt (gainToY (d)), left, right);

    g.setColour (juce::Colour (0xff5F5E5A));
    g.drawHorizontalLine (juce::roundToInt (gainToY (0.0f)), left, right);

    // Bandas
    prisma::Coeffs cf[prisma::numBands];
    PrismaEQAudioProcessor::BandSettings bs[prisma::numBands];

    for (int b = 0; b < prisma::numBands; ++b)
    {
        bs[b] = proc.getBand (b);
        cf[b] = prisma::makeCoeffs (bs[b].type, sr, bs[b].freq, bs[b].gain, bs[b].q);
    }

    const float yc = gainToY (0.0f);
    auto clampY = [top, bottom] (float y) { return juce::jlimit (top, bottom, y); };

    for (int b = 0; b < prisma::numBands; ++b)
    {
        if (! bs[b].on || bs[b].type >= prisma::HighPass)
            continue;

        juce::Path p;
        p.startNewSubPath (left, yc);

        for (float x = left; x <= right; x += 2.0f)
            p.lineTo (x, clampY (gainToY ((float) prisma::magnitudeDb (cf[b], xToFreq (x), sr))));

        p.lineTo (right, yc);
        p.closeSubPath();

        g.setColour (bandColours[b].withAlpha (b == selectedBand ? 0.40f : 0.25f));
        g.fillPath (p);
    }

    // Curva total
    {
        juce::Path master;
        bool first = true;

        for (float x = left; x <= right; x += 2.0f)
        {
            double db = 0.0;

            for (int b = 0; b < prisma::numBands; ++b)
                if (bs[b].on)
                    db += prisma::magnitudeDb (cf[b], xToFreq (x), sr);

            const float y = clampY (gainToY ((float) db));

            if (first)
            {
                master.startNewSubPath (x, y);
                first = false;
            }
            else
            {
                master.lineTo (x, y);
            }
        }

        g.setColour (colText);
        g.strokePath (master, juce::PathStrokeType (2.0f));
    }

    // Etiquetas
    g.setFont (11.0f);
    g.setColour (colDim);

    const struct { float db; const char* text; } dbLabels[] = { { 12.0f, "+12" }, { 6.0f, "+6" }, { 0.0f, "0" },
                                                                { -6.0f, "-6" }, { -12.0f, "-12" } };
    for (auto& l : dbLabels)
        g.drawText (l.text, juce::Rectangle<float> (left + 18.0f, gainToY (l.db) - 13.0f, 34.0f, 12.0f),
                    juce::Justification::centredLeft, false);

    const struct { float f; const char* text; } fLabels[] = { { 50.0f, "50" }, { 100.0f, "100" }, { 500.0f, "500" },
                                                               { 1000.0f, "1k" }, { 5000.0f, "5k" }, { 10000.0f, "10k" } };
    for (auto& l : fLabels)
        g.drawText (l.text, juce::Rectangle<float> (freqToX (l.f) - 20.0f, bottom + 4.0f, 40.0f, 14.0f),
                    juce::Justification::centred, false);

    // Medidores de entrada (izquierda) y salida (derecha)
    auto drawMeter = [&] (juce::Rectangle<float> bar, float db)
    {
        g.setColour (colPanel);
        g.fillRoundedRectangle (bar, 2.0f);

        const float norm = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
        g.setColour (db > -1.0f ? colRed : (db > -9.0f ? colAmber : colGreen));
        g.fillRoundedRectangle (bar.withTop (bar.getBottom() - bar.getHeight() * norm), 2.0f);
    };

    const float meterH = bottom - top;
    const float lx = displayArea.getX() + 6.0f;
    const float rx = displayArea.getRight() - 16.0f;

    drawMeter (juce::Rectangle<float> (lx, top, 4.0f, meterH), inLevel[0]);
    drawMeter (juce::Rectangle<float> (lx + 6.0f, top, 4.0f, meterH), inLevel[1]);
    drawMeter (juce::Rectangle<float> (rx, top, 4.0f, meterH), outLevel[0]);
    drawMeter (juce::Rectangle<float> (rx + 6.0f, top, 4.0f, meterH), outLevel[1]);

    g.setColour (colMuted);
    g.drawText ("IN", juce::Rectangle<float> (lx - 4.0f, bottom + 4.0f, 20.0f, 14.0f), juce::Justification::centred, false);
    g.drawText ("OUT", juce::Rectangle<float> (rx - 8.0f, bottom + 4.0f, 30.0f, 14.0f), juce::Justification::centred, false);

    // Cabecera
    g.setColour (colPink);
    g.fillEllipse (displayArea.getX() + 14.0f, displayArea.getY() + 15.0f, 10.0f, 10.0f);
    g.setColour (colText);
    g.setFont (16.0f);
    g.drawText ("Prisma EQ", juce::Rectangle<float> (displayArea.getX() + 30.0f, displayArea.getY() + 8.0f, 200.0f, 24.0f),
                juce::Justification::centredLeft, false);

    // Nodos
    g.setFont (11.0f);

    for (int b = 0; b < prisma::numBands; ++b)
    {
        const auto pos = getNodePos (b);
        const juce::Rectangle<float> dot (pos.x - 8.0f, pos.y - 8.0f, 16.0f, 16.0f);

        g.setColour (bandColours[b].withAlpha (bs[b].on ? 1.0f : 0.35f));
        g.fillEllipse (dot);
        g.setColour (juce::Colours::white);
        g.drawText (juce::String (b + 1), dot, juce::Justification::centred, false);

        if (b == selectedBand)
        {
            g.setColour (colText.withAlpha (0.6f));
            g.drawEllipse (pos.x - 13.0f, pos.y - 13.0f, 26.0f, 26.0f, 1.0f);

            juce::Rectangle<float> tip (pos.x - 62.0f, pos.y - 42.0f, 124.0f, 20.0f);
            tip.setPosition (juce::jlimit (displayArea.getX() + 2.0f, displayArea.getRight() - tip.getWidth() - 2.0f, tip.getX()),
                             juce::jmax (displayArea.getY() + 34.0f, tip.getY()));

            g.setColour (colText);
            g.fillRoundedRectangle (tip, 4.0f);
            g.setColour (colPanel);
            g.drawText (formatFreq (bs[b].freq) + "   " + juce::String (bs[b].gain, 1) + " dB", tip,
                        juce::Justification::centred, false);
        }
    }

    // Separadores de la zona de controles
    g.setColour (colLine);
    g.drawHorizontalLine (juce::roundToInt (displayArea.getBottom()), 0.0f, (float) getWidth());
    g.drawHorizontalLine (row2Top, 0.0f, (float) getWidth());
}
