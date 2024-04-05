/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AirAbsorption.h"


//==============================================================================
/**
*/
class IOSONOSourceControlAudioProcessor  : public juce::AudioProcessor,
    private juce::Timer,
    private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    IOSONOSourceControlAudioProcessor();
    ~IOSONOSourceControlAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    AirAbsorption air;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    void oscConnect();
    void sendMetadata();
    void timerCallback() final
    {
      sendMetadata();
    }

    void parameterChanged(const juce::String& parameterID, float newValue);

    void calculateVolume();
    void calculateCutoff();
    void calculateDelay();
    void calculateAzimuth();

    int sourceIndex = 1;
    int sourceType = 0;
    float radius = 0.0f;
    float volFactor = 1.0f;
    float azimuth = 0.0f;
    float elevation = 0.0f;
    float dist = 1.0f;

    float volume = 0.0f;
    float cutoff = 20000.0f;
    float delayValue = 0.0f;

    
    /* instantiate filter */
    juce::dsp::FirstOrderTPTFilter<float> lowpass;

    /* instantiate delay line */
    static constexpr auto maxDelaySamples = 192000; // 4 seconds @48 kHz => max distance of 1360 meters
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> LagrangeDelay { maxDelaySamples };

    /* instantiate smoothers */
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothAmp;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothCutoff;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothDelay;
    
    /* instantiate OSC message sender */
    juce::OSCSender oscMessageSender;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IOSONOSourceControlAudioProcessor)
};
