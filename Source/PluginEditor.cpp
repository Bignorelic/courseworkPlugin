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

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto centre = bounds.getCentre();

        Path p;

        Rectangle<float> r;
        r.setLeft(centre.getX() - 2);
        r.setRight(centre.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(centre.getY() - rswl->getTextHeight() * 1.5);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, centre.getX(), centre.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(Colours::black);
        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

//===============================================================================//

void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBoudns = getSliderBounds();

    /*g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow);
    g.drawRect(sliderBoudns);*/

    getLookAndFeel().drawRotarySlider(g,
        sliderBoudns.getX(),
        sliderBoudns.getY(),
        sliderBoudns.getWidth(),
        sliderBoudns.getHeight(),
        //mapFromLog10((float)jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0), 2.f, 10.f),
        //mapToLog10((float)getValue(), 10.f, 20000.f),
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        startAng,
        endAng,
        *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    //return getLocalBounds();

    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextBoxHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);

    if (suffix == "" || suffix =="dB")
    {
        r.setCentre(bounds.getCentreX(), bounds.getCentreY());
    }
    else
    {
        r.setCentre(bounds.getCentreX(), 0);
        r.setY(20);
    }

    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    //return juce::String(getValue());

    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();

    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();

        if (val > 999.f)
        {
            val /= 1000.f;
            addK = true;
        }

        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse;
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        if (addK)
            str << "k";

        str << suffix;
    }

    return str;
}
//===============================================================================//
//===============================================================================//

void LookAndFeel::drawLinearSlider(juce::Graphics& g,
    int x, 
    int y, 
    int width, 
    int height,
    float sliderPos,
    float minSliderPos,
    float maxSliderPos,
    const juce::Slider::SliderStyle,
    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    //surroundng box
    g.setColour(Colour(27u, 19u, 37u));
    g.fillRoundedRectangle(bounds, 2.f);

    g.setColour(Colour(209u, 224u, 248u));
    g.drawRoundedRectangle(bounds, 10.f, 1.f);

    if (auto* rswl = dynamic_cast<LinearSliderWithLabels*>(&slider))
    {
        auto centre = bounds.getCentre();

        //line within the box
        auto sliderLine = bounds;
        sliderLine.setY(sliderLine.getHeight() * 0.5);
        sliderLine.setX(sliderLine.getWidth() * 0.1);
        sliderLine.setHeight(sliderLine.getHeight() * 0.05);
        sliderLine.setWidth(sliderLine.getWidth() * 0.8);

        g.setColour(Colour(106u, 116u, 133u));
        g.drawRoundedRectangle(sliderLine, 2.f, 1.f);

        //dot
        Path p;

        Rectangle<float> r;
        //r.setLeft(bounds.getX());
        //r.setRight(centre.getY());
        r.setSize(6.f, 6.f);
        r.setCentre(bounds.getWidth() * 0.1, centre.getY());


        p.addEllipse(r);

        jassert(minSliderPos < maxSliderPos);

        auto sliderPosition = jmap(sliderPos, 0.f, 1.f, minSliderPos, maxSliderPos);

        p.applyTransform(AffineTransform().translated(sliderPos * 3.2, 0.f));

        g.setColour(Colour(209u, 224u, 248u));
        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getWidth() * 0.5, bounds.getY() + 30);

        g.setColour(Colours::black);
        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}



void LinearSliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    getLookAndFeel().drawLinearSlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight()*.5,
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, sliderBounds.getWidth() * 0.25),
        0.0,
        sliderBounds.getWidth(),
        LinearHorizontal,
        *this);
}

juce::Rectangle<int> LinearSliderWithLabels::getSliderBounds() const
{
    return getLocalBounds();
}

juce::String LinearSliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();

    juce::String str;

    str << suffix;

    return str;
}


//===============================================================================//
//===============================================================================//

ResponseCurveComponent::ResponseCurveComponent(CourseworkPluginAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    updateChain();

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
        updateChain();
        //repaint
        repaint();
    }
}

void ResponseCurveComponent::updateChain()
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);

    auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

    updateFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
    updateFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);

    g.drawImage(background, getLocalBounds().toFloat());

    //making the response curve that shows what the filters are doing
    auto bounds = getAnalysisArea();

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
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);

    //draws the curve
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

void ResponseCurveComponent::resized()
{
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);  

    Graphics g(background);

    Array<float> mainFreqs
    {
        10, 100, 1000, 10000
    };
    
    Array<float> subFreqs
    {
        20, 30, 40, 50, 60, 70, 80, 90,
        200, 300, 400, 500, 600, 700, 800, 900,
        2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000
    };

    //getting values for the bouneding area so its quick to use
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    //vertical lines for the sub frequencies
    g.setColour(Colours::darkgrey);
    for (auto f : subFreqs)
    {
        auto normX = mapFromLog10(f, 10.f, 20000.f);

        g.drawVerticalLine(getWidth() * normX, top - 4, bottom + 4);
    }

    //horizontal lines for gain
    Array<float> gain
    {
        -24,-12,0,12,24
    };

    for (auto gdB : gain)
    {
        auto y = jmap(gdB, -24.f, 24.f, float(bottom), float(top));
        g.setColour(gdB == 0.f ? Colour(Colours::ghostwhite) : Colour(Colours::grey));
        g.drawHorizontalLine(y, left, right);
    }

    //vertical lines for frequency
    Array<float> xs;
    for (auto f : mainFreqs)
    {
        auto normX = mapFromLog10(f, 10.f, 20000.f);
        xs.add(left + width * normX);
    }

    g.setColour(Colours::white);
    for (auto x : xs)
    {
        if (x != left + width * mapFromLog10(10.f, 10.f, 20000.f))
            g.drawVerticalLine(x, top - 4, bottom + 4);
    }  

    //g.setColour(Colours::orange);
    //g.drawRect(getRenderArea());
    //g.setColour(Colours::green);
    //g.drawRect(getAnalysisArea());

    //adding labels for main frequencies
    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);

    for (int i = 0; i < mainFreqs.size(); ++i)
    {
        auto f = mainFreqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;
        if (f > 999.f)
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
            str << "k";
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }

    for (auto gdB : gain)
    {
        auto y = jmap(gdB, -24.f, 24.f, float(bottom), float(top));
        String str;
        if (gdB > 0)
            str << "+";
        str << gdB;

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(10 , y);

        g.setColour(gdB == 0.f ? Colour(Colours::ghostwhite) : Colour(Colours::lightgrey));
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();

    bounds.removeFromTop(12);
    bounds.removeFromLeft(20);
    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
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

    responseCurveComponent      (audioProcessor),
    lowCutFreqSliderAttachment  (audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment (audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    driveSliderAttachment       (audioProcessor.apvts,"Drive", driveSlider),
    postGainSliderAttachment    (audioProcessor.apvts,"PostGain", driveSlider),
    lowCutSlopeSelectAttachment (audioProcessor.apvts, "LowCut Slope", lowCutSlopeSelect),
    highCutSlopeSelectAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSelect)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    addAndMakeVisible(audioProcessor.waveformViewer);
    audioProcessor.waveformViewer.setColours(juce::Colours::black, juce::Colours::white);

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
    audioProcessor.waveformViewer.setBounds(waveformArea);

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


    //waveform size specifics
    //juce::Graphics g(background);
    //g.setColour(juce::Colours::white);
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