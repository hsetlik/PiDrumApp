/*
  ==============================================================================

    PatternLoader.h
    Created: 21 Feb 2021 2:50:46pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SequenceComponent.h"



class PatternManagerComponent : public juce::Component, juce::Button::Listener, juce::ComboBox::Listener
{
public:
    PatternManagerComponent(SequenceComponent* c, SequenceProcessor* p);
    ~PatternManagerComponent() {}
    void buttonClicked(juce::Button* b) override;
    void comboBoxChanged(juce::ComboBox* b) override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void savePattern(juce::String patternName);
    void loadPattern(juce::ValueTree t);
    juce::Image saveButtonImg;
    juce::ImageButton saveButton;
    juce::ComboBox patternBox;
    SequenceComponent* seqComponent;
    SequenceProcessor* seqProcessor;
};
