/*
  ==============================================================================

    PatternLoader.cpp
    Created: 21 Feb 2021 2:50:46pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#include "PatternLoader.h"

PatternManagerComponent::PatternManagerComponent(SequenceComponent* c, SequenceProcessor* p) : saveButtonImg(juce::ImageCache::getFromMemory(BinaryData::saveIcon_png, 647)), seqComponent(c), seqProcessor(p)
{
    setWantsKeyboardFocus(false);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(patternBox);
    saveButton.addListener(this);
    patternBox.addListener(this);
    saveButton.setImages(true, true, true, saveButtonImg, 1.0f,
                         juce::Colours::transparentBlack, saveButtonImg, 1.0f,
                         juce::Colours::transparentBlack, saveButtonImg, 1.0f,
                         juce::Colours::transparentBlack);
}

void PatternManagerComponent::buttonClicked(juce::Button* b)
{
    
}
void PatternManagerComponent::comboBoxChanged(juce::ComboBox* b)
{
    
}
void PatternManagerComponent::paint(juce::Graphics& g)
{
    auto n = getHeight() / 6;
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Press Start 2P", 15.0f, 0));
    auto tempoString = "Tempo: " + juce::String(seqProcessor->TEMPO);
    auto x = getWidth() / 16;
    g.drawSingleLineText(tempoString, 12 * x, 5 * n);
}
void PatternManagerComponent::resized()
{
    auto nHeight = getHeight() / 8;
    auto nWidth = getWidth() / 16;
    patternBox.setBounds(15, 2.5 * nHeight, nWidth * 7, nHeight * 4);
    saveButton.setBounds(nWidth * 8, nHeight, 40, 42);
}
void PatternManagerComponent::savePattern(juce::String patternName)
{
    
}
void PatternManagerComponent::loadPattern(juce::ValueTree t)
{
    
}
