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

    CustomRotarySlider lowCutFreqSlider,
        highCutFreqSlider,
        driveSlider,
        postGainSlider;

    juce::Slider lowCutSlopeSelect = juce::Slider(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::NoTextBox);
    juce::Slider highCutSlopeSelect = juce::Slider(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::NoTextBox);


    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CourseworkPluginAudioProcessorEditor)
};
