/*
  ==============================================================================

    SequenceProcessor.cpp
    Created: 18 Feb 2021 1:13:54pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#include "SequenceProcessor.h"

juce::MidiMessage TrackData::getNoteOn()
{
    auto channel = 1;
    auto velocity = 1.0f;
    int noteNumber;
    switch(voice)
    {
        case kick1:
        {
            noteNumber = 36;
            break;
        }
        case kick2: {
            noteNumber = 35;
            break;
        }
        case openHat: {
            noteNumber = 46;
            break;
        }
        case closedHat: {
            noteNumber = 42;
            break;
        }
        case snare: {
            noteNumber = 40;
            break;
        }
        case clap: {
            noteNumber = 39;
            break;
        }
        case clave: {
            noteNumber = 37;
            break;
        }
    }
    //printf("played note: %d\n", noteNumber);
    return juce::MidiMessage::noteOn(channel, noteNumber, velocity);
}

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
    
    auto allDevices = juce::MidiOutput::getAvailableDevices();
    midiOut = juce::MidiOutput::openDevice(allDevices[0].identifier);
}

void SequenceProcessor::setSampleRate(double rate)
{
    sampleRate = rate;
    //beats per second = TEMPO / 60
    //subDivsPerBeat = MIN_SUBDIV
    secsPerSubDiv = 1.0f / (4.0f * MIN_SUBDIV * (TEMPO / 60.0f));

    samplesPerSubDiv = secsPerSubDiv * sampleRate;
    
    //printf("Samples Per SubDiv: %f\n", samplesPerSubDiv);
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
        if(t->noteOutput)
        {
            auto m = t->getNoteOn();
            if(midiOut != NULL)
                midiOut->sendMessageNow(m);
        }
    }
}

void SequenceProcessor::updateToTempo()
{
    secsPerSubDiv = 1.0f / (4.0f * MIN_SUBDIV * (TEMPO / 60.0f));
    samplesPerSubDiv = secsPerSubDiv * sampleRate;
}
