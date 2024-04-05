/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class IOSONOSourceControlAudioProcessorEditor  : public juce::AudioProcessorEditor,
    private juce::Slider::Listener,
    private juce::ComboBox::Listener,
    private juce::TextEditor::Listener,
    private juce::Button::Listener
    // private juce::Timer
{
public:
    IOSONOSourceControlAudioProcessorEditor (IOSONOSourceControlAudioProcessor&);
    ~IOSONOSourceControlAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    

private:

    void showConnectionErrorMessage(const juce::String& messageText)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Connection error",
            messageText,
            "OK");
    }

    void sliderValueChanged(juce::Slider* slider) override; 
    void comboBoxChanged(juce::ComboBox* ComboBox) override;
    void textEditorTextChanged(juce::TextEditor& textEditor) override;
    void buttonClicked(juce::Button* button) override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    IOSONOSourceControlAudioProcessor& audioProcessor;

    juce::Slider azimSlider;
    juce::Slider distSlider;
    juce::Slider elevSlider;
    juce::Slider radiusSlider;
    juce::Slider volFactorSlider;

    juce::Label azimLabel;
    juce::Label distLabel;
    juce::Label elevLabel;
    juce::Label radiusLabel;
    juce::Label volFactorLabel;


    juce::ComboBox typeBox;
    juce::ComboBox indexBox;
    juce::Label sourceLabel;
    
    juce::TextEditor portText;
    juce::Label portLabel;

    juce::TextEditor ipText;
    juce::Label ipLabel;

    juce::TextButton airBtn;
    juce::TextButton dopplerBtn;


    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> azimSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> elevSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeBoxAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> indexBoxAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> radiusAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volFactorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> airAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dopplerAttachment;

    //juce::OSCSender oscMessageSender;

    juce::String ipAddress = "127.0.0.1";
    int oscPort = 9001;

    int sourceIndex = 1;
    int sourceType = 0;
    float sourceVolume = 0.0f;
    float radius = 0.0f;
    float volFactor = 1.0f;
    float azimuth = 0.0f;
    float elevation = 0.0f;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IOSONOSourceControlAudioProcessorEditor)
};
