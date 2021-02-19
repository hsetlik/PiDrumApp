/*
  ==============================================================================

    SequenceProcessor.cpp
    Created: 18 Feb 2021 1:13:54pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#include "SequenceProcessor.h"

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
    auto subDivSecs = (double)(TEMPO / 60.0f) / MIN_SUBDIV;
    samplesPerSubDiv = subDivSecs * sampleRate;
}

void SequenceProcessor::advanceBySamples(int numSamples)
{
    samplesIntoSubDiv += numSamples;
    if(samplesIntoSubDiv > samplesPerSubDiv)
    {
        samplesIntoSubDiv -= samplesPerSubDiv;
        currentSubDiv += 1;
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
    auto subDivSecs = (double)(TEMPO / 60.0f) / MIN_SUBDIV;
    samplesPerSubDiv = subDivSecs * sampleRate;
}
