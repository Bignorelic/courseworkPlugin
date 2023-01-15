/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
    }
};

struct ResponseCurveComponent : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    ResponseCurveComponent(CourseworkPluginAudioProcessor&);
    ~ResponseCurveComponent();

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

    void timerCallback() override;

    void paint(juce::Graphics& g) override;

private:
    CourseworkPluginAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };

    MonoChain monoChain;
};


//==============================================================================
/**
*/
class CourseworkPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CourseworkPluginAudioProcessorEditor (CourseworkPluginAudioProcessor&);
    ~CourseworkPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CourseworkPluginAudioProcessor& audioProcessor;

    //making the sliders
    CustomRotarySlider lowCutFreqSlider,
        highCutFreqSlider,
        driveSlider,
        postGainSlider;

    juce::Slider lowCutSlopeSelect = juce::Slider(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::NoTextBox);
    juce::Slider highCutSlopeSelect = juce::Slider(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::NoTextBox);

    ResponseCurveComponent responseCurveComponent;

    //connecting the sliders to the parameters
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment lowCutFreqSliderAttachment,
                highCutFreqSliderAttachment,
                driveSliderAttachment,
                postGainSliderAttachment,
                lowCutSlopeSelectAttachment,
                highCutSlopeSelectAttachment;

    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CourseworkPluginAudioProcessorEditor)
};
