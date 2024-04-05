/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
// #include <math.h>

//==============================================================================
IOSONOSourceControlAudioProcessorEditor::IOSONOSourceControlAudioProcessorEditor (IOSONOSourceControlAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
    setSize(300, 320);
    setWantsKeyboardFocus(true);


    azimSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    azimSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 24);
    azimSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);
    azimSlider.setRotaryParameters(0, juce::MathConstants<float>::twoPi, 0);
    //azimuth.setPopupDisplayEnabled(true, false, this);
    azimSlider.setTextValueSuffix("deg");
    azimSlider.setValue(0.0);

    distSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    distSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 24);
    distSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);
    //distSlider.setPopupDisplayEnabled(true, false, this);
    distSlider.setTextValueSuffix("m");
    distSlider.setValue(1.0);

    elevSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    elevSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 24);
    elevSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);
    //elevSlider.setPopupDisplayEnabled(true, false, this);
    elevSlider.setRotaryParameters(juce::MathConstants<float>::pi, 0, 1);
    elevSlider.setTextValueSuffix("deg");
    elevSlider.setValue(0.0);

    azimLabel.setText("Azimuth", juce::dontSendNotification);
    azimLabel.attachToComponent(&azimSlider, false);
    azimLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    azimLabel.setJustificationType(juce::Justification::centredBottom);

    distLabel.setText("Distance", juce::dontSendNotification);
    distLabel.attachToComponent(&distSlider, false);
    distLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    distLabel.setJustificationType(juce::Justification::centredBottom);

    elevLabel.setText("Elevation", juce::dontSendNotification);
    elevLabel.attachToComponent(&elevSlider, false);
    elevLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    elevLabel.setJustificationType(juce::Justification::centredBottom);


    radiusSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    radiusSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    radiusSlider.setVelocityBasedMode(true);
    radiusSlider.setVelocityModeParameters(0.4, 1, 0.09, false);

    radiusLabel.setText("Radius", juce::dontSendNotification);
    radiusLabel.attachToComponent(&radiusSlider, true);
    radiusLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    radiusLabel.setJustificationType(juce::Justification::right);

    volFactorSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    volFactorSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    volFactorSlider.setVelocityBasedMode(true);
    volFactorSlider.setVelocityModeParameters(0.4, 1, 0.09, false);

    volFactorLabel.setText("Factor", juce::dontSendNotification);
    volFactorLabel.attachToComponent(&volFactorSlider, true);
    volFactorLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    volFactorLabel.setJustificationType(juce::Justification::right);


    typeBox.addItem("point source", 1);
    typeBox.addItem("plane wave", 2);
    typeBox.setSelectedId(1);

    for (int i = 1; i < 65; i++)
    {
        indexBox.addItem(juce::String(i), i);
    }
    indexBox.setSelectedId(1);
    //indexBox.setEditableText(true);

    portText.setJustification(juce::Justification::centred);
    portText.setIndents(portText.getLeftIndent(), 0);
    portText.setText(juce::String(oscPort));
    portText.setInputRestrictions(5, "0123456789");

    portLabel.setText("Port:", juce::dontSendNotification);
    portLabel.attachToComponent(&portText, true);
    portLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    portLabel.setJustificationType(juce::Justification::right);

    ipText.setJustification(juce::Justification::centred);
    ipText.setIndents(ipText.getLeftIndent(), 0);
    ipText.setText(ipAddress);
    ipText.setInputRestrictions(0, "0123456789.");

    ipLabel.setText("IP address:", juce::dontSendNotification);
    ipLabel.attachToComponent(&ipText, true);
    ipLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    ipLabel.setJustificationType(juce::Justification::right);

    sourceLabel.setText("Source", juce::dontSendNotification);
    sourceLabel.attachToComponent(&indexBox, true);
    sourceLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    sourceLabel.setJustificationType(juce::Justification::right);

    airBtn.setButtonText("Air absorption");
    airBtn.setToggleable(true);
    airBtn.setClickingTogglesState(true);

    dopplerBtn.setButtonText("Doppler");
    dopplerBtn.setToggleable(true);
    dopplerBtn.setClickingTogglesState(true);
    
    // airBtn.onClick = [this] { airBtnClicked(); };
    

    // add UI elements to the editor
    addAndMakeVisible(&azimSlider);
    addAndMakeVisible(&distSlider);
    addAndMakeVisible(&elevSlider);
    addAndMakeVisible(&typeBox);
    addAndMakeVisible(&indexBox);
    addAndMakeVisible(&portText);
    addAndMakeVisible(&ipText);
    addAndMakeVisible(&sourceLabel);
    addAndMakeVisible(&portLabel);
    addAndMakeVisible(&ipLabel);
    addAndMakeVisible(&radiusSlider);
    addAndMakeVisible(&volFactorSlider);
    addAndMakeVisible(&airBtn);
    addAndMakeVisible(&dopplerBtn);

    // create attachments
    azimSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "AZIM", azimSlider);
    elevSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "ELEV", elevSlider);
    distSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DIST", distSlider);
    typeBoxAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "TYPE", typeBox);
    indexBoxAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "INDEX", indexBox);
    radiusAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "RADIUS", radiusSlider);
    volFactorAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "FACTOR", volFactorSlider);
    airAttachment        = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "AIR", airBtn);
    dopplerAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "DOPPLER", dopplerBtn);

    // add listeners
    azimSlider.addListener(this);
    distSlider.addListener(this);
    elevSlider.addListener(this);
    radiusSlider.addListener(this);
    volFactorSlider.addListener(this);
    typeBox.addListener(this);
    indexBox.addListener(this);
    portText.addListener(this);
    ipText.addListener(this);
    airBtn.addListener(this);
    dopplerBtn.addListener(this);

            
}

IOSONOSourceControlAudioProcessorEditor::~IOSONOSourceControlAudioProcessorEditor()
{
}

//==============================================================================
void IOSONOSourceControlAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::azure);

    g.drawRoundedRectangle(10, 45, 280, 165, 4, 1);
    g.drawRoundedRectangle(10, 215, 280, 100, 4, 1);

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText("Distance dependent volume", 0, 220, getWidth(), 22, juce::Justification::centred, 1);
    
}

void IOSONOSourceControlAudioProcessorEditor::resized()
{
    ipText.setBounds(95, 10, 100, 22);
    portText.setBounds(240, 10, 50, 22);

    azimSlider.setBounds(10, 70, 100, 100);
    elevSlider.setBounds(90, 70, 100, 100);
    distSlider.setBounds(170, 70, 100, 100);

    typeBox.setBounds(150, 180, 120, 22); 
    indexBox.setBounds(80, 180, 60, 22);

    radiusSlider.setBounds(80, 250, 60, 22);
    volFactorSlider.setBounds(200, 250, 60, 22);

    airBtn.setBounds(40, 285, 100, 22);
    dopplerBtn.setBounds(160, 285, 100, 22);
    
}

void IOSONOSourceControlAudioProcessorEditor::buttonClicked(juce::Button* button)
{

}


void IOSONOSourceControlAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{

    //distSlider.onValueChange = [this]
    //    {
    //        //calculateVolume();
    //    };

    //radiusSlider.onValueChange = [this]
    //    {
    //        //calculateVolume();
    //    };

    //volFactorSlider.onValueChange = [this]
    //    {
    //       // calculateVolume();
    //    };

    //azimSlider.onValueChange = [this]
    //    {
    //      //  calculateAzimuth();
    //    };

}


void IOSONOSourceControlAudioProcessorEditor::comboBoxChanged(juce::ComboBox* ComboBox)
{
    /*typeBox.onChange = [this]
        {
            sourceType = typeBox.getSelectedId() - 1;
        };

    indexBox.onChange = [this]
        {
            sourceIndex = (juce::uint32)indexBox.getText().getIntValue();
        };*/
}


void IOSONOSourceControlAudioProcessorEditor::textEditorTextChanged(juce::TextEditor& textEditor)
{


    portText.onReturnKey = [this]
        {
            oscPort = portText.getText().getIntValue();
            //oscConnect();
            portText.unfocusAllComponents();
        };

    portText.onFocusLost = [this]
        {
            oscPort = portText.getText().getIntValue();
            //oscConnect();
            portText.unfocusAllComponents();
        };


    ipText.onReturnKey = [this]
        {
            ipAddress = ipText.getText();
            //oscConnect();
            ipText.unfocusAllComponents();
        };

    ipText.onFocusLost = [this]
        {
            ipAddress = ipText.getText();
            //oscConnect();
            ipText.unfocusAllComponents();
        };


}

