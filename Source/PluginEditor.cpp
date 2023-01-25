/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//drawing the circular sliders

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(Colour(27u, 19u, 37u));
    g.fillEllipse(bounds);

    g.setColour(Colour(209u, 224u, 248u));
    g.drawEllipse(bounds, 1.f);

    auto centre = bounds.getCentre();

    Path p;

    Rectangle<float> r;
    r.setLeft(centre.getX() - 2);
    r.setRight(centre.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(centre.getY() - 75);

    p.addRectangle(r);

    jassert(rotaryStartAngle < rotaryEndAngle);

    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

    p.applyTransform(AffineTransform().rotated(sliderAngRad, centre.getX(), centre.getY()));

    g.fillPath(p);
}

//===============================================================================//

void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBoudns = getSliderBounds();

    getLookAndFeel().drawRotarySlider(g, 
        sliderBoudns.getX(), 
        sliderBoudns.getY(), 
        sliderBoudns.getWidth(), 
        sliderBoudns.getHeight(), 
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0), 
        startAng, 
        endAng, 
        *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    return getLocalBounds();
}

//===============================================================================//

ResponseCurveComponent::ResponseCurveComponent(CourseworkPluginAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        //update the monochain
        auto chainSettings = getChainSettings(audioProcessor.apvts);

        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

        updateFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
        updateFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);

        //repaint
        repaint();
    }
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);

    //making the response curve that shows what the filters are doing
    auto bounds = getLocalBounds();

    //setting bounds
    auto spectrumArea = bounds;
    auto spectrumW = spectrumArea.getWidth();
    auto waveformArea = bounds;
    auto waveformW = waveformArea.getWidth();

    //getting the chains to read off
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags;

    mags.resize(spectrumW);

    for (int i = 0; i < spectrumW; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(spectrumW), 20.0, 20000.0);

        if (!lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!highcut.isBypassed<0>())
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;

    const double outputMin = spectrumArea.getBottom();
    const double outputMax = spectrumArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };

    responseCurve.startNewSubPath(spectrumArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(spectrumArea.getX() + i, map(mags[i]));
    };

    //draws a box for the area
    g.setColour(Colours::lavender);
    g.drawRoundedRectangle(spectrumArea.toFloat(), 4.f, 1.f);

    //draws the curve
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

//==============================================================================
CourseworkPluginAudioProcessorEditor::CourseworkPluginAudioProcessorEditor (CourseworkPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),

    highCutFreqSlider   (*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    lowCutFreqSlider    (*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    driveSlider         (*audioProcessor.apvts.getParameter("Drive"), ""),
    postGainSlider      (*audioProcessor.apvts.getParameter("PostGain"), "dB"),
    lowCutSlopeSelect   (*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    highCutSlopeSelect  (*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Oct"),

    responseCurveComponent(audioProcessor),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    driveSliderAttachment(audioProcessor.apvts,"Drive", driveSlider),
    postGainSliderAttachment(audioProcessor.apvts,"PostGain", driveSlider),
    lowCutSlopeSelectAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSelect),
    highCutSlopeSelectAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSelect)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    setSize (800, 400);
}

CourseworkPluginAudioProcessorEditor::~CourseworkPluginAudioProcessorEditor()
{
}

//==============================================================================
void CourseworkPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;
   
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
}

void CourseworkPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //setting bounds for where each parameter will be
    auto bounds = getLocalBounds();

    auto visualiserArea = bounds.removeFromTop(bounds.getHeight() * 0.375);
    auto spectrumArea = visualiserArea.removeFromLeft(visualiserArea.getWidth() * 0.625);
    auto waveformArea = visualiserArea;

    responseCurveComponent.setBounds(spectrumArea);

    auto filterArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);
    auto lowCutArea = filterArea.removeFromLeft(filterArea.getWidth() * 0.5);
    lowCutArea = lowCutArea.removeFromLeft(lowCutArea.getWidth() * 0.9);
    lowCutArea = lowCutArea.removeFromRight(lowCutArea.getWidth() * 0.89);
    auto highCutArea = filterArea.removeFromLeft(filterArea.getWidth() * 0.9);
    highCutArea = highCutArea.removeFromRight(highCutArea.getWidth() * 0.89);

    auto driveArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);
    driveArea = driveArea.removeFromLeft(driveArea.getWidth() * 0.9);
    auto postGainArea = bounds.removeFromLeft(bounds.getWidth() * 0.9);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.8));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.8));

    lowCutSlopeSelect.setBounds(lowCutArea);
    highCutSlopeSelect.setBounds(highCutArea);

    driveSlider.setBounds(driveArea.removeFromRight(driveArea.getWidth() * 0.89));
    postGainSlider.setBounds(postGainArea.removeFromRight(postGainArea.getWidth() * 0.89));
}
std::vector<juce::Component*> CourseworkPluginAudioProcessorEditor::getComps()
{
    return
    {
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &driveSlider,
        &postGainSlider,
        &lowCutSlopeSelect,
        &highCutSlopeSelect,
        &responseCurveComponent
    };
}