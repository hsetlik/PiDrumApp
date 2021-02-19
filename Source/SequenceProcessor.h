/*
  ==============================================================================

    SequenceProcessor.h
    Created: 18 Feb 2021 1:13:54pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

const int MIN_SUBDIV = 60;
const int NUM_NOTES = 16;
const int NUM_TRACKS = 7;

enum analogVoice
{
    kick1,
    kick2,
    openHat,
    closedHat,
    snare,
    clap,
    clave
};

enum stepState
{
    restOff,
    restOn,
    noteOff,
    noteOn
};

struct StepData
{
    StepData(int subDivLength, int indexInTrk) : numSubDivs(subDivLength), indexInTrack(indexInTrk)
    {
        hasNote = false;
        isCurrent = false;
        state = restOff;
    }
    void toggleNote()
    {
        hasNote = !hasNote;
    }
    stepState getState()
    {
        if(isCurrent)
        {
            if(hasNote)
                state = noteOn;
            else
                state = restOn;
        }
        else
        {
            if(hasNote)
                state = noteOff;
            else
                state = restOff;
        }
        return state;
    }
    int numSubDivs;
    int indexInTrack;
    int startSubDiv;
    bool hasNote;
    bool isCurrent;
    stepState state;
};

class TrackData
{
public:
    TrackData(int numSteps, int index, analogVoice v) : totalSubDivs(numSteps * MIN_SUBDIV), trackIndex(index), voice(v)
    {
        for(int i = 0; i < numSteps; ++i)
        {
            steps.add(new StepData(MIN_SUBDIV, i));
        }
        currentStep = steps[0];
        steps[0]->isCurrent = true;
    }
    ~TrackData() {}
    void setStartSubDivs()
    {
        int total = 0;
        for(auto* s : steps)
        {
            s->startSubDiv = total;
            total += s->numSubDivs;
        }
        currentStep = steps[0];
        currentSubDivIndex = 0;
    }
    juce::OwnedArray<StepData> steps;
    void setToSubDiv(int index)
    {
        currentSubDivIndex = index;
        if(index < totalSubDivs)
        {
            auto currentStepIndex = currentStep->indexInTrack;
            auto limit = currentStep->startSubDiv + currentStep->numSubDivs;
            if(index > limit)
            {
                currentStep->isCurrent = false;
                if(currentStepIndex == steps.size() - 1)
                    currentStep = steps[0];
                else
                    currentStep = steps[currentStepIndex + 1];
                currentStep->isCurrent = true;
            }
        }
        else
        {
            currentSubDivIndex -= totalSubDivs;
            currentStep->isCurrent = false;
            currentStep = steps[0];
            currentStep->isCurrent = true;
        }
    }
    //sort these shits out later...
    void tupletUp(int firstIndex, int lastIndex)
    {
        auto startCount = lastIndex - firstIndex;
        if(startCount >= 1)
        {
            
        }
    }
    void tupletDown(int firstIndex, int lastIndex)
    {
        
    }
    int activeStepIndex()
    {
        return currentStep->indexInTrack;
    }
private:
    int totalSubDivs;
    StepData* currentStep;
    int trackIndex;
    int currentSubDivIndex;
    analogVoice voice;
};

class SequenceProcessor
{
public:
    SequenceProcessor();
    ~SequenceProcessor() {}
    float TEMPO;
    void updateToTempo();
    void setSampleRate(double rate);
    void advanceBySamples(int numSamples);
    juce::OwnedArray<TrackData> tracks;
    bool isPlaying;
    int samplesPerSubDiv;
    double sampleRate;
private:
    int totalSubDivs;
    int currentSubDiv;
    int samplesIntoSubDiv;
};
