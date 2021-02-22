/*
  ==============================================================================

    SequenceProcessor.cpp
    Created: 18 Feb 2021 1:13:54pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#include "SequenceProcessor.h"

juce::ValueTree StepData::getStepTree()
{
    return juce::ValueTree(juce::Identifier("StepTree" + juce::String(indexInTrack)),
                           {{subDivLengthId, juce::var(numSubDivs)}, {stepIndexId, juce::var(indexInTrack)}, {hasNoteId, juce::var(hasNote)}
    });
}

void StepData::loadStepTree(juce::ValueTree t)
{
    if(t.hasProperty(subDivLengthId))
    {
        numSubDivs = (int)t.getProperty(subDivLengthId);
        indexInTrack = (int)t.getProperty(stepIndexId);
        hasNote = (bool)t.getProperty(hasNoteId);
        printf("Step %d loaded\n", indexInTrack);
    }
    else
        printf("Not a valid step tree\n");
}

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

juce::ValueTree TrackData::getTrackTree()
{
    auto trkTree = juce::ValueTree(juce::Identifier("TrackTree" + juce::String(trackIndex)), {{totalLengthId, juce::var(totalSubDivs)}});
    for(auto* s : steps)
    {
        trkTree.appendChild(s->getStepTree(), nullptr);
    }
    return trkTree;
}

void TrackData::loadTrackTree(juce::ValueTree t)
{
    totalSubDivs = (int) t.getProperty(totalLengthId);
    int numFullSteps = totalSubDivs / MIN_SUBDIV;
    for(int i = 0; i < numFullSteps; ++i)
    {
        if(i < steps.size())
        {
            steps[i]->loadStepTree(t.getChild(i));
        }
        else
        {
            auto length = t.getChild(i).getProperty(subDivLengthId);
            steps.add(new StepData(length, i));
        }
    }
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

juce::ValueTree SequenceProcessor::getSequenceTree()
{
    auto seqTree = juce::ValueTree(sequenceId,
        {{totalLengthId, juce::var(totalSubDivs)},
        {tempoId, juce::var(TEMPO)}
    });
    for(auto* t : tracks)
        seqTree.appendChild(t->getTrackTree(), nullptr);
    return seqTree;
}

void SequenceProcessor::loadSequenceTree(juce::ValueTree t)
{
    totalSubDivs = (int) t.getProperty(totalLengthId);
    TEMPO = (double) t.getProperty(tempoId);
    int i = 0;
    for(auto* trk : tracks)
    {
        if(t.getChild(i).isValid())
        {
            trk->loadTrackTree(t.getChild(i));
            ++i;
        }
    }
}
