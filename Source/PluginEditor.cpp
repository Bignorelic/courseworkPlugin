/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CourseworkPluginAudioProcessorEditor::CourseworkPluginAudioProcessorEditor (CourseworkPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
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
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
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

    auto filterArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);
    auto lowCutArea = filterArea.removeFromLeft(filterArea.getWidth() * 0.5);
    lowCutArea = lowCutArea.removeFromLeft(lowCutArea.getWidth() * 0.9);
    auto highCutArea = filterArea.removeFromLeft(filterArea.getWidth() * 0.9);

    auto driveArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);
    driveArea = driveArea.removeFromLeft(driveArea.getWidth() * 0.9);
    auto postGainArea = bounds.removeFromLeft(bounds.getWidth() * 0.9);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromRight(lowCutArea.getWidth() * 0.89));
    highCutFreqSlider.setBounds(highCutArea.removeFromRight(highCutArea.getWidth() * 0.89));

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
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}