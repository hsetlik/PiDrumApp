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
const juce::Colour deselected = Color::RGBColor(37, 49, 53);
const juce::Colour selected = Color::RGBColor(159, 207, 219);

StepComponent::StepComponent(int trkIndex, int stpIndex, SequenceProcessor* p) : juce::ShapeButton("stepButton", restColorOff, restColorOff, noteColorOff), trackIndex(trkIndex), stepIndex(stpIndex), proc(p)
{
    state = restOff;
    isSelected = false;
    setClickingTogglesState(true);
    shouldUseOnColours(true);
}

void StepComponent::paintButton(juce::Graphics &g, bool mouseIsOver, bool mouseIsDown)
{
    state = proc->tracks[trackIndex]->steps[stepIndex]->getState();
    if(isSelected)
    {
        g.setColour(selected);
    }
    else
    {
        g.setColour(deselected);
    }
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
        leftEdge += ceil(proc->tracks[voiceType]->steps[i]->numSubDivs * subDivWidth);
        ++i;
    }
}

void TrackComponent::buttonClicked(juce::Button *b)
{
    StepComponent* thisStep = dynamic_cast<StepComponent*>(b);
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
    for(int i = 0; i < stepButtons.size(); ++i)
    {
        auto thisX = stepButtons.getUnchecked(i)->getScreenX();
        auto nextX = thisX + stepButtons.getUnchecked(i)->getWidth();
        if( xPos > thisX && xPos <= nextX)
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
    printf("Selected step %d on track %d\n", toSelect->stepIndex, (int)voiceType);
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
        endNum = startNum + 1;
        stepButtons.removeRange(startIndex, startNum);
        for(int i = 0; i < endNum; ++i)
        {
            stepButtons.insert(startIndex + i, new StepComponent((int)voiceType, startIndex + i, proc));
            stepButtons[startIndex + i]->addListener(this);
            stepButtons[startIndex + i]->addMouseListener(this, true);
            addAndMakeVisible(stepButtons[startIndex + i]);
        }
        resized();
        for(int i = endNum + startIndex; i < stepButtons.size(); ++i)
            stepButtons[i]->stepIndex += 1;
    }
}
//============================================================================

SequenceComponent::SequenceComponent(SequenceProcessor* p) : header("untitled", p), proc(p)
{
    addMouseListener(this, true);
    addAndMakeVisible(header);
    for(int i = 0; i < 7; ++i)
    {
        trackComponents.add(new TrackComponent(analogVoice(i), proc));
        addAndMakeVisible(trackComponents.getLast());
        trackComponents.getLast()->clearSelection();
        printf("Track %d created\n", i);
    }
    startTimerHz(30);
    setWantsKeyboardFocus(true);
}

void SequenceComponent::resized()
{
    header.setBounds(0, 0, getWidth(), HEADER_HEIGHT);
    auto bottomEdge = HEADER_HEIGHT;
    auto trackHeight = (getHeight() - HEADER_HEIGHT) / trackComponents.size();
    for(auto* t : trackComponents)
    {
        t->setBounds(0, bottomEdge, getWidth(), trackHeight);
        bottomEdge += trackHeight;
    }
}

void SequenceComponent::timerCallback()
{
    repaint();
}

void SequenceComponent::mouseDown(const juce::MouseEvent &m)
{
    for(auto* i : trackComponents)
    {
        i->clearSelection();
    }
}

bool SequenceComponent::keyPressed(const juce::KeyPress &p)
{
    auto key = p.getTextCharacter();
    switch(key)
    {
        case 'p': //p for play
        {
            proc->isPlaying = !proc->isPlaying;
            if(proc->isPlaying)
                printf("Playback Started\n");
            else
                printf("Playback Stopped\n");
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
            break;
        }
        default:
        {
            break;
        }
    }
    return false;
}
