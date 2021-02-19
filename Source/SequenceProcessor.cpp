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
