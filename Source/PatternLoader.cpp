/*
  ==============================================================================

    PatternLoader.cpp
    Created: 21 Feb 2021 2:50:46pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#include "PatternLoader.h"

PatternManagerComponent::PatternManagerComponent(SequenceComponent* c, SequenceProcessor* p) : seqComponent(c), seqProcessor(p)
{
    saveButton.setButtonText("Save Pattern");
    addAndMakeVisible(saveButton);
    addAndMakeVisible(patternBox);
    saveButton.addListener(this);
    patternBox.addListener(this);
}
