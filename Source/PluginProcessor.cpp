/*
  ==============================================================================
    PluginProcessor.cpp
    Author:  regnier
    Does a mishmash of things, only for testing purposes.
    --> send OSC data to MMT for visualization and for control of IOSONO Core/Processor
    --> simulates distance: level attenuation, air absorption, doppler effect

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
IOSONOSourceControlAudioProcessor::IOSONOSourceControlAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameters())
#endif
{

    
    

    // start timer  t=20ms
    startTimer(20);

    // establish OSC connection
    oscMessageSender.connect("127.0.0.1", 9001);

    // add listeners
    apvts.addParameterListener("DIST", this); 
    apvts.addParameterListener("RADIUS", this);
    apvts.addParameterListener("FACTOR", this);

}


IOSONOSourceControlAudioProcessor::~IOSONOSourceControlAudioProcessor()
{
}

//==============================================================================
const juce::String IOSONOSourceControlAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IOSONOSourceControlAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool IOSONOSourceControlAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool IOSONOSourceControlAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double IOSONOSourceControlAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int IOSONOSourceControlAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int IOSONOSourceControlAudioProcessor::getCurrentProgram()
{
    return 0;
}

void IOSONOSourceControlAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String IOSONOSourceControlAudioProcessor::getProgramName (int index)
{
    return {};
}

void IOSONOSourceControlAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void IOSONOSourceControlAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };

    // filter init
    lowpass.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
    lowpass.prepare(spec);
    lowpass.setCutoffFrequency(300.0f);

    // delay init
    LagrangeDelay.prepare(spec);
    LagrangeDelay.reset();
    
    // smoothers init
    smoothDelay.reset(getSampleRate(), 0.15);   // ramp length of 150 ms.. arbitrary.. 
    smoothDelay.setCurrentAndTargetValue(1.0);

    smoothAmp.reset(getSampleRate(), 0.02);     // ramp length of 20 ms.. arbitrary.. 
    smoothAmp.setCurrentAndTargetValue(0.0);

    smoothCutoff.reset(getSampleRate(), 0.02);  // ramp length of 20 ms.. arbitrary.. 
    smoothCutoff.setCurrentAndTargetValue(0.0);

    // calculate inital values
    calculateVolume();
    calculateAzimuth();
    calculateDelay();
}

void IOSONOSourceControlAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool IOSONOSourceControlAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void IOSONOSourceControlAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    //for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    //    buffer.clear (i, 0, buffer.getNumSamples());


    // TODO: create callbacks for these 2 instead of polling at every block
    auto  absorb        = apvts.getRawParameterValue("AIR")->load();
    auto  dopplerEffect = apvts.getRawParameterValue("DOPPLER")->load();

    smoothAmp.setTargetValue(volume);
    smoothCutoff.setTargetValue(cutoff);
    smoothDelay.setTargetValue(delayValue);

    auto leftInSamples   = buffer.getReadPointer(0);
    auto rightInSamples  = buffer.getReadPointer(1);
    auto leftOutSamples  = buffer.getWritePointer(0);
    auto rightOutSamples = buffer.getWritePointer(1);
    float leftSample;
    float rightSample;

    
    // DBG("dist: " << dist << " freq: " << cutoff << " Delay: " << delayValue << " Volume: " << volume); // debug

    for (int sample = 0; sample < buffer.getNumSamples(); sample++)
    {

        // double monoIn = (*(leftInSamples + sample) + *(rightInSamples + sample)) * 0.5; //  conversion to mono (for IOSONO, 1 source = 1 channel only)
        leftSample = *(leftInSamples + sample);
        rightSample = *(rightInSamples + sample);
        

        // get smoothed values
        auto currentDelay = smoothDelay.getNextValue(); 
        auto currentVolume = smoothAmp.getNextValue();
        auto currentCutoff = smoothCutoff.getNextValue();

        // enter delay line
        LagrangeDelay.setDelay(currentDelay);           // set delay value
        LagrangeDelay.pushSample(0, leftSample);        // delay line input L
        LagrangeDelay.pushSample(1, rightSample);       // delay line input R

        // set absorption frequency
        lowpass.setCutoffFrequency(currentCutoff);

        /******************** Avoid branching ***********************/

        // if doppler, delay inputs
        leftSample  = dopplerEffect * LagrangeDelay.popSample(0) + (1 - dopplerEffect) * leftSample;
        rightSample = dopplerEffect * LagrangeDelay.popSample(1) + (1 - dopplerEffect) * rightSample;

        
        // if air absorption, filter and attenuate, else, only attenuate 
        *(leftOutSamples + sample)  = currentVolume * (absorb * lowpass.processSample(0, leftSample)  + (1 - absorb) * leftSample);
        *(rightOutSamples + sample) = currentVolume * (absorb * lowpass.processSample(1, rightSample) + (1 - absorb) * rightSample);

        /**************** IOSONO Mode ****************/
        /* when using the IOSONO renderer: gain attenuation using currentVolume should be removed, as it's already in the metadata  */      
        // *(leftOutSamples + sample)  = absorb  * lowpass.processSample(0, leftSample)  + (1 - absorb) * leftSample;
        // *(rightOutSamples + sample) = absorb  * lowpass.processSample(1, rightSample) + (1 - absorb) * rightSample;
    }

    

    //juce::dsp::AudioBlock<float> block(buffer);
    //juce::dsp::ProcessContextReplacing<float> context(block);
    //int absorb = apvts.getRawParameterValue("AIR")->load();
    //if (absorb) lowpass.process(context);
    
}

//==============================================================================
bool IOSONOSourceControlAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* IOSONOSourceControlAudioProcessor::createEditor()
{
    return new IOSONOSourceControlAudioProcessorEditor (*this);
}

//==============================================================================
void IOSONOSourceControlAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void IOSONOSourceControlAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IOSONOSourceControlAudioProcessor();
}


void IOSONOSourceControlAudioProcessor::calculateVolume()
{
    auto  currentRadius = apvts.getRawParameterValue("RADIUS")->load();
    // auto  currentDist = apvts.getRawParameterValue("DIST")->load();
    auto  currentFactor = apvts.getRawParameterValue("FACTOR")->load();

    volume = juce::jlimit(currentRadius, 300.0f, dist);
    volume = pow(currentRadius / volume, currentFactor);
    volume = juce::jlimit(0.0f, 1.0f, volume);
}

void IOSONOSourceControlAudioProcessor::calculateCutoff()
{
    // Calculate cutoff frequency for specified atmospheric conditions (50% humidity, 20 degrees, at sea level)
    air.FilterCutoffSolver(50, 20, air.kPressureSeaLevelPascals);
    cutoff = juce::jlimit(20.0, 0.499 * getSampleRate(), air.cutoffSolve(dist, 3));
}

void IOSONOSourceControlAudioProcessor::calculateDelay()
{
    // auto  currentDist = apvts.getRawParameterValue("DIST")->load();
    delayValue = dist * 2.94117647 * 0.001 * getSampleRate();
    delayValue = juce::jlimit(1.0f, (float)maxDelaySamples, delayValue); // avoid a 0-sample delay
}

void IOSONOSourceControlAudioProcessor::calculateAzimuth()
{
    // convert from "usual" conventions - 0 deg in front, clockwise- to IOSONO - 0 deg to the right, anticlockwise -
    azimuth = apvts.getRawParameterValue("AZIM")->load();
    azimuth = 90.0f - 1.0f * azimuth;
    if (azimuth < 0) azimuth += 360.0f; // wrap around

    // TODO convert to radians .. 
    // azimuth *= 0.01745329251;

}



juce::AudioProcessorValueTreeState::ParameterLayout IOSONOSourceControlAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>    ("AZIM", "azim", 0.0f, 360.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>    ("ELEV", "elev", -90.0f, 90.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>    ("DIST", "dist", juce::NormalisableRange<float>(0.0f, 300.0f, 0.01f, 0.5f), 1.0f)); // NormalisableRange if skew factor needs to be specified
    params.push_back(std::make_unique<juce::AudioParameterInt>      ("TYPE", "type", 1, 2, 1));
    params.push_back(std::make_unique<juce::AudioParameterInt>      ("INDEX", "index", 1, 64, 1));
    params.push_back(std::make_unique<juce::AudioParameterFloat>    ("RADIUS", "radius", 1.0f, 10.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>    ("FACTOR", "factor", 0.0f, 10.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>      ("AIR", "air", 0, 1, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>      ("DOPPLER", "doppler", 0, 1, 0));

    return { params.begin(), params.end() };

}





void IOSONOSourceControlAudioProcessor::oscConnect()
{
    oscMessageSender.connect("127.0.0.1", 9001);
}

void IOSONOSourceControlAudioProcessor::sendMetadata()
{
    // juce::String address = juce::String::formatted("/source/%d/aed", sourceIndex); // old test, spat osc address format
    juce::OSCAddressPattern oscAddress("/iosono/renderer/version1/src");
    juce::OSCMessage oscMessage(oscAddress);


    auto elev = apvts.getRawParameterValue("ELEV")->load();
    auto dist = apvts.getRawParameterValue("DIST")->load();
    auto type = apvts.getRawParameterValue("TYPE")->load();
    auto idx = apvts.getRawParameterValue("INDEX")->load();
    
    // calculateVolume();
    calculateAzimuth();

    // IOSONO UDP Packet 
    // /iosono/renderer/version1/src #source_index #source_type #azim #elev #dist #volume 0. 0. 0 0 0. 0
    
    oscMessage.addInt32(idx);
    oscMessage.addInt32(type - 1);
    oscMessage.addFloat32(azimuth);
    oscMessage.addFloat32(elev);
    oscMessage.addFloat32(dist);
    oscMessage.addFloat32(volume);
    oscMessage.addFloat32(0.0f);
    oscMessage.addFloat32(0.0f);
    oscMessage.addInt32(0);
    oscMessage.addInt32(0);
    oscMessage.addFloat32(0.0f);
    oscMessage.addInt32(0);

    oscMessageSender.send(oscMessage);
}


void IOSONOSourceControlAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "DIST")
    {
        dist = juce::jlimit(0.1f, 300.0f, newValue);
        calculateVolume();
        calculateCutoff();
        calculateDelay();
    }
    
    if (parameterID == "RADIUS")
    {
        calculateVolume();
    }

    if (parameterID == "FACTOR")
    {
        calculateVolume();
    }

}



