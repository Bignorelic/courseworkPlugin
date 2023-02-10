/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

//class Visualiser : public juce::AudioVisualiserComponent
//{
//public:
//    Visualiser() : AudioVisualiserComponent(1)
//    {
//        //settings for waveform visualiser
//        setBufferSize(64);
//        setSamplesPerBlock(256);
//    }
//};

//==============================================================================
//==============================================================================

CourseworkPluginAudioProcessor::CourseworkPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), waveformViewer(2)
#endif
{
    //settings for the waveform visualiser
    waveformViewer.setRepaintRate(60);
    waveformViewer.setBufferSize(512);
    waveformViewer.setSamplesPerBlock(8);
}

CourseworkPluginAudioProcessor::~CourseworkPluginAudioProcessor()
{
}

//==============================================================================
const juce::String CourseworkPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CourseworkPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CourseworkPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CourseworkPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CourseworkPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CourseworkPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CourseworkPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CourseworkPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CourseworkPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void CourseworkPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void CourseworkPluginAudioProcessor::updateLowCutFilters(const ChainSettings& chainSettings)
{
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(), 2 * (chainSettings.lowCutSlope + 1));

    //low cut filter in both channels
    auto& leftLowCut = leftChain.get <ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get <ChainPositions::LowCut>();

    leftChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    rightChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);

    updateFilter(leftLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
    updateFilter(rightLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
}

void CourseworkPluginAudioProcessor::updateHighCutFilters(const ChainSettings& chainSettings)
{
    //same with the high cut
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, getSampleRate(), 2 * (chainSettings.highCutSlope + 1));

    auto& leftHighCut = leftChain.get <ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get <ChainPositions::HighCut>();

    leftChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    rightChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);

    updateFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
    updateFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
}

void CourseworkPluginAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings(apvts);

    //update both filters
    updateLowCutFilters(chainSettings);
    updateHighCutFilters(chainSettings);
}

//==============================================================================
void CourseworkPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    updateFilters();

    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);

    //sine oscillator tester
    osc.initialise([](float x) { return std::sin(x); });

    spec.numChannels = getTotalNumOutputChannels();
    osc.prepare(spec);
    osc.setFrequency(50);
}

void CourseworkPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CourseworkPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CourseworkPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //update filters
    updateFilters();

    juce::dsp::AudioBlock<float> block(buffer);

    //sine oscillator
    //buffer.clear();
    //
    //juce::dsp::ProcessContextReplacing<float> stereoContext(block);
    //osc.process(stereoContext);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);

    //get distortion parameters
    float drive = *apvts.getRawParameterValue("Drive");
    float postGain = *apvts.getRawParameterValue("Post Gain");
    float mix = *apvts.getRawParameterValue("Mix");

    //distortion logic
    for (int channel = 0; channel < totalNumInputChannels; channel++)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            //save original signal
            float drySignal = *channelData;

            //raise volume
            *channelData *= drive;

            //clip audio with tanh function
            //mix with original signal
            //multiply by gain
            *channelData = ((tanh(*channelData) * mix + drySignal * (1 - mix))) * gainToAmplifier(postGain);

            //other distortion algorithms
            //*channelData = ((sin(*channelData) * mix + drySignal * (1 - mix))) * gainToAmplifier(postGain);
            //*channelData = ((pow(sin(*channelData), 3) * mix + drySignal * (1 - mix))) * gainToAmplifier(postGain);
            //*channelData = ( ( 0.625 * tan(sin(*channelData)) * mix + drySignal * (1 - mix) ) ) * gainToAmplifier(postGain);

            channelData++;
        }
    }

    //waveform viewer
    waveformViewer.pushBuffer(buffer);

    //update FFT spectrum analyser
    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);

    //level meter
    rmsLevelLeft = juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
    rmsLevelRight = juce::Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
}

float gainToAmplifier(float gain)
{
    //converts gain in dB to multiplier
    return pow(10, gain / 20);
}

//==============================================================================
bool CourseworkPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CourseworkPluginAudioProcessor::createEditor()
{
    return new CourseworkPluginAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void CourseworkPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.


    ////saving the parameter states
    //juce::MemoryOutputStream mos(destData, true);
    //apvts.state.writeToStream(mos);
}

void CourseworkPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    ////loading parameter states
    //auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    //if (tree.isValid())
    //{
    //    apvts.replaceState(tree);
    //    updateFilter();
    //}
}

float CourseworkPluginAudioProcessor::getRmsValue(const int channel) const
{
    jassert(channel == 0 || channel == 1);
    if (channel == 0)
        return rmsLevelLeft;
    if (channel == 1)
        return rmsLevelRight;
    return 0.f;
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    //get parameter values
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());
    
    settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
    settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;

    return settings;
}

void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}

juce::AudioProcessorValueTreeState::ParameterLayout CourseworkPluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //making variables for the filter frequencies
    //normalisableRange -> (lower frequency, upper frequency, frequency step, skew)
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", "LowCut Freq", juce::NormalisableRange<float>(10.f, 20000.f, 1.f, 0.25f), 10.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq", "HighCut Freq", juce::NormalisableRange<float>(10.f, 20000.f, 1.f, 0.25f), 20000.f));

    //making different slopes: 12db/Oct, 24db/Oct, 36db/Oct and 48db/Oct
    juce::StringArray stringArray;
    for (int i = 0; i < 4; ++i)
    {
        juce::String str;
        str << (12 + i * 12);
        str << " db/Oct";
        stringArray.add(str);
    }

    //slopes for the filters
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));

    //parameters for distortion
    layout.add(std::make_unique<juce::AudioParameterFloat>("Drive", "Drive", juce::NormalisableRange<float>(1.f, 10.f, 0.01f, 1.f), 1.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Post Gain", "Post Gain", juce::NormalisableRange<float>(-12.f, 0.f, 0.01f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Mix", "Mix", juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f), 1.f));

    //toggle box for bypass buttons
    layout.add(std::make_unique<juce::AudioParameterBool>("LowCut Bypassed", "LowCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Bypassed", "HighCut Bypassed", false));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CourseworkPluginAudioProcessor();
}
