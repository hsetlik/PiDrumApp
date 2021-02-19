/*
  ==============================================================================

    SequenceProcessor.cpp
    Created: 18 Feb 2021 1:13:54pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#include "SequenceProcessor.h"

void TrackData::tupletUp(int firstIndex, int lastIndex)
{
    if(lastIndex - firstIndex > 0)
    {
        auto startNum = lastIndex - firstIndex + 1;
        auto numEnd = startNum + 1;
        auto lengthInSubDivs = 0;
        for(int i = 0; i < startNum; ++i)
        {
            lengthInSubDivs += steps[firstIndex + i]->numSubDivs;
        }
        printf("length: %d\n", lengthInSubDivs);
        steps.removeRange(firstIndex, startNum);
        auto newLength = lengthInSubDivs / numEnd;
        for(int i = 0; i < numEnd; ++i)
        {
            steps.insert(firstIndex + i, new StepData(newLength, firstIndex + i));
        }
        int ind = 0;
        for(auto* s : steps)
        {
            s->indexInTrack = ind;
            ++ind;
        }
        setStartSubDivs();
        printf("Track Number %d\n", (int)voice);
        int i = 0;
        for(auto* s : steps)
        {
            printf("Step %d starts at %d with length %d\n", i, s->startSubDiv, s->numSubDivs);
            ++i;
        }
    }
}

void TrackData::tupletDown(int firstIndex, int lastIndex)
{
    if(lastIndex - firstIndex > 1)
    {
        auto startNum = lastIndex - firstIndex + 1;
        auto numEnd = startNum - 1;
        auto lengthInSubDivs = 0;
        for(int i = 0; i < startNum; ++i)
        {
            lengthInSubDivs += steps[firstIndex + i]->numSubDivs;
        }
        printf("length: %d\n", lengthInSubDivs);
        steps.removeRange(firstIndex, startNum);
        auto newLength = lengthInSubDivs / numEnd;
        for(int i = 0; i < numEnd; ++i)
        {
            steps.insert(firstIndex + i, new StepData(newLength, firstIndex + i));
        }
        int ind = 0;
        for(auto* s : steps)
        {
            s->indexInTrack = ind;
            ++ind;
        }
        setStartSubDivs();
        printf("Track Number %d\n", (int)voice);
        int i = 0;
        for(auto* s : steps)
        {
            printf("Step %d starts at %d with length %d\n", i, s->startSubDiv, s->numSubDivs);
            ++i;
        }
    }
}

SequenceProcessor::SequenceProcessor()
{
    TEMPO = 120;
    totalSubDivs = MIN_SUBDIV * NUM_NOTES;
    tracks.clear();
    for(int i = 0; i < NUM_TRACKS; ++i)
    {
        tracks.add(new TrackData(NUM_NOTES, i, (analogVoice)i));
    }
    samplesIntoSubDiv = 0;
    currentSubDiv = 0;
}

void SequenceProcessor::setSampleRate(double rate)
{
    sampleRate = rate;
    //beats per second = TEMPO / 60
    //subDivsPerBeat = MIN_SUBDIV
    secsPerSubDiv = 1.0f / (4.0f * MIN_SUBDIV * (TEMPO / 60.0f));

    samplesPerSubDiv = secsPerSubDiv * sampleRate;
    
    printf("Samples Per SubDiv: %f\n", samplesPerSubDiv);
}
void SequenceProcessor::advanceBySamples(int numSamples)
{
    samplesIntoSubDiv += numSamples;
    
    if(samplesIntoSubDiv >= samplesPerSubDiv)
    {
        samplesIntoSubDiv -= (samplesPerSubDiv * floor(numSamples / samplesPerSubDiv));
        currentSubDiv += floor(numSamples / samplesPerSubDiv);
    }
    if(currentSubDiv > totalSubDivs)
        currentSubDiv = 0;
    for(auto* t : tracks)
    {
        t->setToSubDiv(currentSubDiv);
    }
}

void SequenceProcessor::updateToTempo()
{
    secsPerSubDiv = 1.0f / (4.0f * MIN_SUBDIV * (TEMPO / 60.0f));
    samplesPerSubDiv = secsPerSubDiv * sampleRate;
}
