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
    kick1 = 0,
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

class StepData
{
public:
    StepData(int subDivLength, int indexInTrk) : numSubDivs(subDivLength), indexInTrack(indexInTrk), startSubDiv((MIN_SUBDIV / 4) * indexInTrk)
    {
        hasNote = false;
        isCurrent = false;
        state = restOff;
    }
    ~StepData() {}
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
        steps[0]->isCurrent = true;
        setStartSubDivs();
        currentStep = steps[0];
        currentSubDivIndex = 0;
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
    }
    juce::OwnedArray<StepData> steps;
    void setToSubDiv(int index)
    {
        currentSubDivIndex = index;
        for(auto* s : steps)
        {
            if(s->startSubDiv <= currentSubDivIndex && currentSubDivIndex < (s->startSubDiv + s->numSubDivs))
            {
                currentStep = s;
                s->isCurrent = true;
                break;
            }
            else
                s->isCurrent = false;
        }
    }
    void tupletUp(int firstIndex, int lastIndex);
    void tupletDown(int firstIndex, int lastIndex);
    int activeStepIndex()
    {
        return currentStep->indexInTrack;
    }
private:
    int lastStepIndex = 0;
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
    double samplesPerSubDiv;
    double sampleRate;
    double secsPerSubDiv;
private:
    int totalSubDivs;
    int currentSubDiv;
    int samplesIntoSubDiv;
};
