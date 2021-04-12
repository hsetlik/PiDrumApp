/*
  ==============================================================================

    SequenceComponent.cpp
    Created: 18 Feb 2021 9:29:25pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#include "SequenceComponent.h"


StepComponent::StepComponent(int trkIndex, int stpIndex, SequenceProcessor* p) : juce::ShapeButton("stepButton", restColorOff, restColorOff, noteColorOff), trackIndex(trkIndex), stepIndex(stpIndex), proc(p)
{
    state = restOff;
    isSelected = false;
    setClickingTogglesState(true);
    shouldUseOnColours(true);
}

void StepComponent::paintButton(juce::Graphics &g, bool mouseIsOver, bool mouseIsDown)
{
    if(proc->tracks[trackIndex]->steps[stepIndex]->getState() >= 0)
        state = proc->tracks[trackIndex]->steps[stepIndex]->getState();
    if(isSelected)
    {
        g.setColour(selected);
    }
    else
    {
        g.setColour(deselected);
    }
    //g.setColour(Color::complement())
    g.fillRect(getLocalBounds());
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
    g.fillRect(getLocalBounds().reduced(3));
}

//==============================================

 TrackComponent::TrackComponent(analogVoice v, SequenceProcessor* p) : label(v), voiceType(v), proc(p)
{
    setInterceptsMouseClicks(true, true);
    addAndMakeVisible(label);
    auto trkIndex = (int)voiceType;
    for(int i = 0; i < NUM_NOTES; ++i)
    {
        stepButtons.add(new StepComponent(trkIndex, i, proc));
        stepButtons.getLast()->addListener(this);
        stepButtons.getLast()->addMouseListener(this, true);
        addAndMakeVisible(stepButtons.getLast());
    }
}

void TrackComponent::resized()
{
    label.setBounds(0, 0, LABEL_WIDTH, getHeight());
    subDivWidth = (double)(getWidth() - LABEL_WIDTH) / (NUM_NOTES * MIN_SUBDIV);
    float leftEdge = LABEL_WIDTH;
    int i = 0;
    for(auto* s : stepButtons)
    {
        s->setBounds(leftEdge, 0, ceil(proc->tracks[voiceType]->steps[i]->numSubDivs * subDivWidth), getHeight());
        leftEdge += proc->tracks[voiceType]->steps[i]->numSubDivs * subDivWidth;
        ++i;
    }
}

void TrackComponent::buttonClicked(juce::Button *b)
{
    if(StepComponent* thisStep = dynamic_cast<StepComponent*>(b))
        proc->tracks[thisStep->trackIndex]->steps[thisStep->stepIndex]->toggleNote();
}

void TrackComponent::clearSelection()
{
    if(selectedSteps.size() > 0)
    {
        for(auto* s : selectedSteps)
            s->isSelected = false;
    }
    selectedSteps.clear();
}

StepComponent* TrackComponent::stepAtXPos(int xPos)
{
    //finds the step on this track which, if it exists, corresponds to a given x pixel value
    for(int i = 0; i < stepButtons.size(); ++i)
    {
        auto thisX = stepButtons.getUnchecked(i)->getScreenX();
        auto nextX = thisX + stepButtons.getUnchecked(i)->getWidth();
        if(xPos > thisX && xPos <= nextX)
        {
            return stepButtons.getUnchecked(i);
            break;
        }
    }
    return NULL;
}

void TrackComponent::selectStep(StepComponent *toSelect)
{
    toSelect->isSelected = true;
    selectedSteps.push_back(toSelect);
}

void TrackComponent::mouseDown(const juce::MouseEvent &m)
{
    clearSelection();
}

void TrackComponent::mouseDrag(const juce::MouseEvent &m)
{
    if(m.mouseWasDraggedSinceMouseDown())
    {
        StepComponent* selectedStep = stepAtXPos(m.getScreenX());
        if(selectedStep != NULL)
        {
            if(selectedStep->isSelected == false && selectedSteps.size() < 5)
            {
                selectStep(selectedStep);
            }
        }
    }
}

bool TrackComponent::hasSelection()
{
    if(selectedSteps.size() > 1)
        return true;
    else
        return false;
}

void TrackComponent::tupletUp()
{
    if(selectedSteps.size() > 0)
    {
        startIndex = selectedSteps[0]->stepIndex;
        endIndex = selectedSteps[selectedSteps.size() - 1]->stepIndex;
        proc->tracks[(int)voiceType]->tupletUp(startIndex, endIndex);
        startNum = endIndex - startIndex + 1;
        if(startNum == 1)
            endNum = 3;
        else
            endNum = startNum + 1;
        stepButtons.removeRange(startIndex, startNum);
        clearSelection();
        for(int i = 0; i < endNum; ++i)
        {
            stepButtons.insert(startIndex + i, new StepComponent((int)voiceType, startIndex + i, proc));
            stepButtons[startIndex + i]->addListener(this);
            stepButtons[startIndex + i]->addMouseListener(this, true);
            selectedSteps.push_back(stepButtons[startIndex + i]);
            stepButtons[startIndex + i]->isSelected = true;
            addAndMakeVisible(stepButtons[startIndex + i]);
        }
        resized();
        for(int i = endNum + startIndex; i < stepButtons.size(); ++i)
            stepButtons[i]->stepIndex += 1;
    }
}

void TrackComponent::tupletDown()
{
    if(selectedSteps.size() > 1)
    {
        startIndex = selectedSteps[0]->stepIndex;
        endIndex = selectedSteps[selectedSteps.size() - 1]->stepIndex;
        proc->tracks[(int)voiceType]->tupletDown(startIndex, endIndex);
        startNum = endIndex - startIndex + 1;
        endNum = startNum - 1;
        stepButtons.removeRange(startIndex, startNum);
        clearSelection();
        for(int i = 0; i < endNum; ++i)
        {
            stepButtons.insert(startIndex + i, new StepComponent((int)voiceType, startIndex + i, proc));
            stepButtons[startIndex + i]->addListener(this);
            stepButtons[startIndex + i]->addMouseListener(this, true);
            selectedSteps.push_back(stepButtons[startIndex + i]);
            stepButtons[startIndex + i]->isSelected = true;
            addAndMakeVisible(stepButtons[startIndex + i]);
        }
        resized();
        for(int i = endNum + startIndex; i < stepButtons.size(); ++i)
            stepButtons[i]->stepIndex -= 1;
    }
}

void TrackComponent::loadTrackTree(juce::ValueTree t)
{
    //because the step component constructor only takes an
    //index and a track number, all we need to do here is ensure that the OwnedArray has the right NUMBER of steps
    //sizing and on/off states are handled in the paint/resize functions via pointer to the sequence processor
    auto currentSteps = stepButtons.size();
    auto newSteps = t.getNumChildren();
    if(currentSteps < newSteps)
    {
        for(int i = currentSteps; i < newSteps; ++i)
        {
            stepButtons.add(new StepComponent((int)voiceType, i, proc));
            stepButtons.getLast()->addListener(this);
            stepButtons.getLast()->addMouseListener(this, true);
            addAndMakeVisible(stepButtons.getLast());
        }
    }
    else if(newSteps < currentSteps)
    {
        for(int i = currentSteps; i > newSteps; --i)
            stepButtons.remove(i, true);
    }
}
//============================================================================
//processor side handles all the sequence-specific data, we just need to resize each track component
void SequenceComponent::loadPatternTree(juce::ValueTree t)
{
    int i = 0;
    for(auto* trk : trackComponents)
    {
        trk->loadTrackTree(t.getChild(i));
        ++i;
    }
}

SequenceComponent::SequenceComponent(SequenceProcessor* p, juce::Component* sib) : proc(p), headerLabel(sib)
{
    addMouseListener(this, true);
    for(int i = 0; i < 7; ++i)
    {
        trackComponents.add(new TrackComponent(analogVoice(i), proc));
        addAndMakeVisible(trackComponents.getLast());
        trackComponents.getLast()->clearSelection();
    }
    startTimerHz(24);
    setFocusContainer(true);
    setWantsKeyboardFocus(true);
}

void SequenceComponent::resized()
{
    auto bottomEdge = 0;
    auto trackHeight = getHeight() /  trackComponents.size();
    for(auto* t : trackComponents)
    {
        t->setBounds(0, bottomEdge, getWidth(), trackHeight);
        bottomEdge += trackHeight;
    }
    if(hasKeyboardFocus(false) == false)
    {
        juce::Timer::callAfterDelay (100, [&] { grabKeyboardFocus(); });
    }
}

void SequenceComponent::timerCallback()
{
    //make sure that this component still recieves keystrokes if the user touches the patch manager
    if(!hasKeyboardFocus(false))
        grabKeyboardFocus();
    headerLabel->repaint();
    repaint();
}

void SequenceComponent::mouseDown(const juce::MouseEvent &m)
{
    for(auto* t : trackComponents)
        t->clearSelection();
}

bool SequenceComponent::keyPressed(const juce::KeyPress &p)
{
    auto key = p.getTextCharacter();
    switch(key)
    {
        case ' ': //
        {
            proc->isPlaying = !proc->isPlaying;
            break;
        }
        case 'f': //f for faster
        {
            proc->TEMPO += 1.0;
            proc->updateToTempo();
            break;
        }
        case 's': //s for slower
        {
            proc->TEMPO -= 1.0;
            proc->updateToTempo();
            break;
        }
        case 'm': //m for more
        {
            auto* track = selectedTrack();
            if(track != nullptr)
                track->tupletUp();
            break;
        }
        case 'l':
        {
            auto* track = selectedTrack();
            if(track != nullptr)
                track->tupletDown();
            break;
        }
        default:
        {
            break;
        }
    }
    return false;
}
