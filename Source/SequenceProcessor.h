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

struct StepData
{
    StepData(int subDivLength, int indexInTrk) : numSubDivs(subDivLength), indexInTrack(indexInTrk)
    {
        hasNote = false;
        isCurrent = false;
    }
    int numSubDivs;
    int indexInTrack;
    int startSubDiv;
    bool hasNote;
    bool isCurrent;
};

class TrackData
{
public:
    TrackData(int numSteps, int index) : totalSubDivs(numSteps * MIN_SUBDIV), trackIndex(index)
    {
        for(int i = 0; i < numSteps; ++i)
        {
            steps.add(new StepData(MIN_SUBDIV, i));
        }
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
                currentStep = steps[currentStepIndex + 1];
        }
        else
        {
            currentSubDivIndex -= totalSubDivs;
            currentStep = steps[0];
        }
    }
    //sort these shits out later...
    void tupletUp(int firstIndex, int lastIndex)
    {
        auto startCount = lastIndex - firstIndex;
        if(startCount >= 1)
        {
            auto endCount = startCount + 1;
            
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
};

class SequenceProcessor
{
public:
    SequenceProcessor();
    ~SequenceProcessor() {}
    int getSamplesPerSubDiv(float tempo);
    int getTotalSubDivs();
    static float TEMPO;
    void setSampleRate(double rate);
    void advanceBySamples(int numSamples);
    static juce::OwnedArray<TrackData> tracks;
private:
    int totalSubDivs;
    int currentSubDiv;
    int samplesPerSubDiv;
    int samplesIntoSubDiv;
    double sampleRate;
};
