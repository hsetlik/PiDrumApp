/*
  ==============================================================================

    SequenceComponent.cpp
    Created: 18 Feb 2021 9:29:25pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#include "SequenceComponent.h"

const juce::Colour restColorOff = Color::RGBColor(51, 81, 90);
const juce::Colour restColorOn = Color::blend(restColorOff, juce::Colours::white, 0.35);
const juce::Colour noteColorOn = Color::RGBColor(255, 84, 106);
const juce::Colour noteColorOff = Color::blend(noteColorOn, juce::Colours::black, 0.35f);

StepComponent::StepComponent(int trkIndex, int stpIndex) : juce::ShapeButton("stepButton", restColorOff, restColorOff, noteColorOff), trackIndex(trkIndex), stepIndex(stpIndex)
{
    state = restOff;
    
}

void StepComponent::paintButton(juce::Graphics &g, bool mouseIsOver, bool mouseIsDown)
{
    state = SequenceProcessor::tracks[trackIndex]->steps[stepIndex]->state;
    switch(state)
    {
        case restOff:
        {
            g.setColour(restColorOff);
            break;
        }
        case restOn:
        {
            g.setColour(restColorOn);
            break;
        }
        case noteOff:
        {
            g.setColour(noteColorOff);
            break;
        }
        case noteOn:
        {
            g.setColour(noteColorOn);
            break;
        }
    }
    g.fillRect(getLocalBounds());
    auto border = getLocalBounds().expanded(1);
    if(isSelected)
    {
        g.setColour(juce::Colours::white);
    }
    else
    {
        g.setColour(juce::Colours::black);
    }
    if(!(border.getX() < 0))
        g.drawRect(border, 2);
}
