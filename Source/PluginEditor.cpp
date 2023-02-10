/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//===============================================================================//

//class Visualiser : public juce::AudioVisualiserComponent
//{
//
//};


//===============================================================================//
//===============================================================================//


//draws the circular sliders
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

    //create bounds for the slider area
    auto bounds = Rectangle<float>(x, y, width, height);

    //fill circle with the bounds
    g.setColour(Colour(27u, 19u, 37u));
    g.fillEllipse(bounds);

    //outline the circle
    g.setColour(Colour(209u, 224u, 248u));
    g.drawEllipse(bounds, 1.f);

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto centre = bounds.getCentre();

        Path p;

        //sets bounds for the line tat indicates position of slider
        Rectangle<float> r;
        r.setLeft(centre.getX() - 2);
        r.setRight(centre.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(centre.getY() - rswl->getTextHeight() * 1.5);

        //sets a rounded rectangle with those bounds as a path
        p.addRoundedRectangle(r, 2.f);

        //raises an expection if the start angle is greater than the end angle
        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        //transforms the rounded rectangle with to the value of the parameter
        p.applyTransform(AffineTransform().rotated(sliderAngRad, centre.getX(), centre.getY()));

        //draws the path
        g.fillPath(p);

        //gets parameter value as text
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        //sets bounds for the text box
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        //draws text box
        g.setColour(Colours::black);
        g.fillRect(r);

        //draws the text
        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    auto range = getRange();
    auto sliderBoudns = getSliderBounds();

    //g.setColour(Colours::red);
    //g.drawRect(getLocalBounds());
    //g.setColour(Colours::yellow);
    //g.drawRect(sliderBoudns);

    //only call the custom skew function for the frequency sliders
    if (suffix == "Hz" || suffix == "kHz")
    {
        getLookAndFeel().drawRotarySlider(g,
            sliderBoudns.getX(),
            sliderBoudns.getY(),
            sliderBoudns.getWidth(),
            sliderBoudns.getHeight(),
            customSkew(jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0)),
            startAng,
            endAng,
            *this);
    }
    else
    {
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
}

double customSkew(double value)
{
    //index determines the amount of skew that occurs
    //i found 1000 to be a nice number
    int index = 1000;
    double normalisedValue = (log((index - 1) * value + 1) / log(index));
    return normalisedValue;
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextBoxHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);

    //sets centres for the sliders
    if (suffix == "" || suffix =="dB")
    {
        //centre stays at centre for distortion sliders
        r.setCentre(bounds.getCentreX(), bounds.getCentreY());
    }
    else
    {
        //centre is lower for the frequency sliders
        r.setCentre(bounds.getCentreX(), 0);
        r.setY(20);
    }

    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();

    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        //gets value of parameter
        float val = getValue();

        //if the value is bigger than 1000
        //  add k to the suffix
        if (val > 999.f)
        {
            val /= 1000.f;
            addK = true;
        }

        //convert value to string
        //if addK is true
        //  have two decimal places
        //if not
        //  no decimals
        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse;
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        
        //add k to the string if addK is true
        if (addK)
            str << "k";

        //add the suffix to the string
        str << suffix;
    }

    return str;
}

//===============================================================================//

//draws the horizontal linear sliders
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

    //draw box
    g.setColour(Colour(27u, 19u, 37u));
    g.fillRoundedRectangle(bounds, 2.f);

    //draw outline of box
    g.setColour(Colour(209u, 224u, 248u));
    g.drawRoundedRectangle(bounds, 10.f, 1.f);

    if (auto* rswl = dynamic_cast<LinearSliderWithLabels*>(&slider))
    {
        auto centre = bounds.getCentre();

        //line within the box for the path that the dot takes
        auto sliderLine = bounds;
        sliderLine.setY(sliderLine.getHeight() * 0.5);
        sliderLine.setX(sliderLine.getWidth() * 0.1);
        sliderLine.setHeight(sliderLine.getHeight() * 0.05);
        sliderLine.setWidth(sliderLine.getWidth() * 0.8);

        //draw line
        g.setColour(Colour(106u, 116u, 133u));
        g.drawRoundedRectangle(sliderLine, 2.f, 1.f);

        //dot
        Path p;

        //bounds for the dot
        Rectangle<float> r;
        r.setSize(6.f, 6.f);
        r.setCentre(bounds.getWidth() * 0.1, centre.getY());

        //add circle with those bounds as a path
        p.addEllipse(r);

        jassert(minSliderPos < maxSliderPos);

        auto sliderPosition = jmap(sliderPos, 0.f, 1.f, minSliderPos, maxSliderPos);

        //move dot with the parameter value
        p.applyTransform(AffineTransform().translated(sliderPos * 3.2, 0.f));

        //set colour and draw dot
        g.setColour(Colour(209u, 224u, 248u));
        g.fillPath(p);

        //gets parameter value as text
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        //creates text box bounds and draws
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getWidth() * 0.5, bounds.getY() + 30);

        g.setColour(Colours::black);
        g.fillRect(r);

        //draw text
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

    //add suffix to string
    str << suffix;

    return str;
}

//===============================================================================//

//draws the toggle boxes
void LookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& toggleButton,
    bool shouldDrawBurronAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    Path powerButton;

    //set bounds
    auto bounds = toggleButton.getLocalBounds();
    auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 12;
    auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
    r.setCentre(r.getCentreX(), r.getCentreY() + 5);

    float ang = 30.f;
    size -= 10;

    //adds an arc to the path
    powerButton.addCentredArc(r.getCentreX(), 
        r.getCentreY(), 
        size * 0.5, 
        size * 0.5, 
        0.f, 
        degreesToRadians(ang), 
        degreesToRadians(360.f - ang), 
        true);

    //add a vertical line to the path
    powerButton.startNewSubPath(r.getCentreX(), r.getY());
    powerButton.lineTo(r.getCentre());
    
    PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
    
    //change colour for each state of the toggle button
    auto colour = toggleButton.getToggleState() ? Colours::dimgrey : Colours::aliceblue;

    //set colour, draw path and draw circle around it
    g.setColour(colour);
    g.strokePath(powerButton, pst);
    g.drawEllipse(r, 2);
}

//===============================================================================//

//draws vertical linear sliders
void VerticalLinearSlider::paint(juce::Graphics& g)
{
    using namespace juce;

    //get values to use
    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    int x = sliderBounds.getX();
    int y = sliderBounds.getY();
    int width = sliderBounds.getWidth();
    int height = sliderBounds.getHeight();
    float sliderPos = jmap(getValue(), range.getStart(), range.getEnd(), 0.0, height * 0.1);
    float minSliderPos = 0.0;
    float maxSliderPos = 1.0;

    auto bounds = Rectangle<float>(x, y, width, height);

    //draw surrounding box
    g.setColour(Colour(27u, 19u, 37u));
    g.fillRoundedRectangle(bounds, 2.f);

    //draw outline
    g.setColour(Colour(209u, 224u, 248u));
    g.drawRoundedRectangle(bounds, 10.f, 1.f);

    if (auto* rswl = dynamic_cast<VerticalLinearSlider*>(&*this))
    {
        auto centre = bounds.getCentre();

        //line within the box
        auto sliderLine = bounds;
        sliderLine = sliderLine.withSizeKeepingCentre(sliderLine.getWidth() * 0.05, sliderLine.getHeight() * 0.8);

        //set colour and draw line
        g.setColour(Colour(106u, 116u, 133u));
        g.drawRoundedRectangle(sliderLine, 2.f, 1.f);

        //dash showing position of value
        Path p;

        //set bounds for dash
        Rectangle<float> r;
        r.setSize(20.f, 4.f);
        r.setCentre(bounds.getCentreX(), sliderLine.getCentreY() + sliderLine.getHeight() * 0.4);

        //draw dash
        p.addRoundedRectangle(r, 1.f);

        jassert(minSliderPos < maxSliderPos);

        //let the dash follow value of the parameter 
        p.applyTransform(AffineTransform().translated(0.f, 10.f + sliderPos * -7.8));

        //set colour and draw
        g.setColour(Colour(209u, 224u, 248u));
        g.fillPath(p);
    }
}

juce::Rectangle<int> VerticalLinearSlider::getSliderBounds() const
{
    return getLocalBounds();
}

//===============================================================================//
//===============================================================================//

//response curve
ResponseCurveComponent::ResponseCurveComponent(CourseworkPluginAudioProcessor& p) : 
    audioProcessor(p),
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    //update curve
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

//produces path for FFT
void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;

    while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();

            juce::FloatVectorOperations::copy(
                monoBuffer.getWritePointer(0, 0),
                monoBuffer.getReadPointer(0, size),
                monoBuffer.getNumSamples() - size);

            juce::FloatVectorOperations::copy(
                monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                tempIncomingBuffer.getReadPointer(0, 0),
                size);

            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }

    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();

    const auto binWidth = sampleRate / (double)fftSize;

    /*
    * if there are FFT data buffers to pull
    *   if we can pull a buffer
    *       generate a path
    */
    while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if (leftChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }

    /*
    * while there are paths can be pulled
    *   pull as many as we can
    *       display most recent path
    */

    while (pathProducer.getNumPathsAvailable())
    {
        pathProducer.getPath(leftChannelFFTPath);
    }
}

void ResponseCurveComponent::timerCallback()
{
    //get values
    auto fftBounds = getAnalysisArea().toFloat();
    auto sampleRate = audioProcessor.getSampleRate();

    //produce path for eachh stereo channel
    leftPathProducer.process(fftBounds, sampleRate);
    rightPathProducer.process(fftBounds, sampleRate);

    //if parameters change
    //  update the curve
    if (parametersChanged.compareAndSetBool(false, true))
    {
        //update the monochain
        updateChain();
    }

    //constantly repaint
    repaint();
}

void ResponseCurveComponent::updateChain()
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);

    //update the filters based on parameters
    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);

    auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

    updateFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
    updateFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

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

    //each pixel along the width of the spectrum area
    for (int i = 0; i < spectrumW; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(spectrumW), 10.0, 20000.0);

        //change gain of frquqncy at that point depending on if the filters are on
        if (!monoChain.isBypassed<ChainPositions::LowCut>())
        {
            if (!lowcut.isBypassed<0>())
                mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<1>())
                mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<2>())
                mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<3>())
                mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!monoChain.isBypassed<ChainPositions::HighCut>())
        {
            if (!highcut.isBypassed<0>())
                mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<1>())
                mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<2>())
                mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<3>())
                mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        mags[i] = Decibels::gainToDecibels(mag);
    }

    //set the curve as a path
    Path responseCurve;

    //map the gain of frequency to height of path
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

    //generate and paint left channel FFT path
    auto leftChannelFFTPath = leftPathProducer.getPath();
    leftChannelFFTPath.applyTransform(AffineTransform().translation(spectrumArea.getX(), spectrumArea.getY()));

    g.setColour(Colours::slateblue);
    g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));

    //generate and paint right channel FFT path
    auto rightChannelFFTPath = rightPathProducer.getPath();
    rightChannelFFTPath.applyTransform(AffineTransform().translation(spectrumArea.getX(), spectrumArea.getY()));

    //g.setColour(Colours::red);
    g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));

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

    //getting values for the bounding area so its quick to use
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

        g.drawVerticalLine(left + width * normX, top - 4, bottom + 4);
    }

    //horizontal lines for gain
    Array<float> gain
    {
        -24, -12, 0, 12, 24
    };

    for (auto gdB : gain)
    {
        auto y = jmap(gdB, -24.f, 24.f, float(bottom), float(top));
        g.setColour(gdB == 0.f ? Colour(Colours::ghostwhite) : Colour(Colours::grey));
        g.drawHorizontalLine(y, left, right);
    }

    //vertical lines for main frequencies
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

        //same addK code as the rotary sliders
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

        //set bounds
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);

        //draw text
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

//===============================================================================//
//===============================================================================//

CourseworkPluginAudioProcessorEditor::CourseworkPluginAudioProcessorEditor (CourseworkPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),

    highCutFreqSlider   (*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    lowCutFreqSlider    (*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    driveSlider         (*audioProcessor.apvts.getParameter("Drive"), ""),
    postGainSlider      (*audioProcessor.apvts.getParameter("Post Gain"), "dB"),
    distortionMix       (*audioProcessor.apvts.getParameter("Mix"), ""),
    lowCutSlopeSelect   (*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    highCutSlopeSelect  (*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Oct"),

    responseCurveComponent      (audioProcessor),
    lowCutFreqSliderAttachment  (audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment (audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    driveSliderAttachment       (audioProcessor.apvts, "Drive", driveSlider),
    postGainSliderAttachment    (audioProcessor.apvts, "Post Gain", postGainSlider),
    distortionMixAttachment     (audioProcessor.apvts, "Mix", distortionMix),
    lowCutSlopeSelectAttachment (audioProcessor.apvts, "LowCut Slope", lowCutSlopeSelect),
    highCutSlopeSelectAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSelect),

    lowCutBypassButtonAttachment    (audioProcessor.apvts, "LowCut Bypassed", lowCutBypassButton),
    highCutBypassButtonAttachment   (audioProcessor.apvts, "HighCut Bypassed", highCutBypassButton)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    //add waveform visualiser
    addAndMakeVisible(audioProcessor.waveformViewer);
    audioProcessor.waveformViewer.setColours(juce::Colours::black, juce::Colours::white);
    //audioProcessor.waveformViewer.setColours(juce::Colour(10.f, 10.f, 10.f, 0.f), juce::Colours::white);

    //add each parameter
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    lowCutBypassButton.setLookAndFeel(&lnf);
    highCutBypassButton.setLookAndFeel(&lnf);

    //plugin size
    setSize (800, 400);
}

CourseworkPluginAudioProcessorEditor::~CourseworkPluginAudioProcessorEditor()
{
    lowCutBypassButton.setLookAndFeel(nullptr);
    highCutBypassButton.setLookAndFeel(nullptr);
}

void CourseworkPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;

    g.fillAll(Colours::black);

    auto bounds = getLocalBounds();

    //set bounds for everything
    auto visualiserArea = bounds.removeFromTop(bounds.getHeight() * 0.375);
    auto spectrumArea = visualiserArea.removeFromLeft(visualiserArea.getWidth() * 0.625);
    auto waveformArea = visualiserArea;
    audioProcessor.waveformViewer.setBounds(waveformArea.withSizeKeepingCentre(298, 100));
    auto waveformBounds = waveformArea.withSizeKeepingCentre(300, 120);

    auto filterArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);
    auto lowCutArea = filterArea.removeFromLeft(filterArea.getWidth() * 0.5);
    lowCutArea = lowCutArea.removeFromLeft(lowCutArea.getWidth() * 0.9);
    lowCutArea = lowCutArea.removeFromRight(lowCutArea.getWidth() * 0.89);
    auto highCutArea = filterArea.removeFromLeft(filterArea.getWidth() * 0.9);
    highCutArea = highCutArea.removeFromRight(highCutArea.getWidth() * 0.89);

    bounds.removeFromRight(20);

    auto driveArea = bounds.removeFromLeft(165);
    auto distortionMixArea = bounds.removeFromLeft(50);
    distortionMixArea = distortionMixArea.withSizeKeepingCentre(20, 140);
    auto postGainArea = bounds;

    //g.setColour(Colours::red);
    //g.drawRect(lowCutArea);
    //g.drawRect(highCutArea);
    //g.drawRect(distortionMixArea);
    //g.drawRect(driveArea);
    //g.drawRect(postGainArea);

    //draw labels for all parameters
    g.setColour(Colours::white);

    labelWriter(g, lowCutArea, "Low Cut", 0);
    labelWriter(g, highCutArea, "High Cut", 0);
    labelWriter(g, driveArea, "Drive", 1);
    labelWriter(g, distortionMixArea, "Drive Mix", 2);
    labelWriter(g, postGainArea, "Post Gain", 1);

    //draw lines for waveform visualiser
    g.setColour(Colours::lavender);
    g.drawRoundedRectangle(waveformBounds.toFloat(), 4.f, 1.f);
    g.setColour(Colours::grey);
    g.drawHorizontalLine(24, waveformArea.getX(), waveformArea.getRight());
    g.drawHorizontalLine(waveformArea.getHeight() - 24, waveformArea.getX(), waveformArea.getRight());
}

void labelWriter(juce::Graphics&g, //juce graphics
    juce::Rectangle<int> area, //area
    juce::String text, //text
    int yPos) //determines height
{
    auto textArea = area.withSizeKeepingCentre(100, 50);
    int offset;

    if (yPos == 0)
        offset = 65;
    else if (yPos == 1)
        offset = 75;
    else
        offset = 85;

    //set area size
    textArea.setCentre(area.getCentreX(), area.getCentreY() + offset);

    //draw text
    g.drawFittedText(text, textArea, juce::Justification::centred, 1);
}

void CourseworkPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //making bounds for everything
    auto bounds = getLocalBounds();

    auto visualiserArea = bounds.removeFromTop(bounds.getHeight() * 0.375);
    auto spectrumArea = visualiserArea.removeFromLeft(visualiserArea.getWidth() * 0.625);
    auto waveformArea = visualiserArea;
    //audioProcessor.waveformViewer.setBounds(waveformArea);

    responseCurveComponent.setBounds(spectrumArea);

    auto filterArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);
    auto lowCutArea = filterArea.removeFromLeft(filterArea.getWidth() * 0.5);
    lowCutArea = lowCutArea.removeFromLeft(lowCutArea.getWidth() * 0.9);
    lowCutArea = lowCutArea.removeFromRight(lowCutArea.getWidth() * 0.89);
    auto highCutArea = filterArea.removeFromLeft(filterArea.getWidth() * 0.9);
    highCutArea = highCutArea.removeFromRight(highCutArea.getWidth() * 0.89);

    bounds.removeFromRight(20);

    auto driveArea = bounds.removeFromLeft(165);
    auto distortionMixArea = bounds.removeFromLeft(50);
    distortionMixArea = distortionMixArea.withSizeKeepingCentre(20, 140);
    auto postGainArea = bounds;

    //setting bounds for everything
    lowCutBypassButton.setBounds(lowCutArea.removeFromTop(35));
    highCutBypassButton.setBounds(highCutArea.removeFromTop(35));

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.8));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.8));

    lowCutSlopeSelect.setBounds(lowCutArea);
    highCutSlopeSelect.setBounds(highCutArea);

    driveSlider.setBounds(driveArea.withSizeKeepingCentre(150, 230));
    postGainSlider.setBounds(postGainArea.withSizeKeepingCentre(150, 230));
    distortionMix.setBounds(distortionMixArea);
}

std::vector<juce::Component*> CourseworkPluginAudioProcessorEditor::getComps()
{
    //returns each parameter to make visible
    return
    {
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &driveSlider,
        &postGainSlider,
        &distortionMix,
        &lowCutSlopeSelect,
        &highCutSlopeSelect,
        &responseCurveComponent,

        &lowCutBypassButton,
        &highCutBypassButton
    };
}