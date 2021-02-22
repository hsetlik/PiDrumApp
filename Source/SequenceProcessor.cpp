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
    //each step tree is created as a child of a track tree
    //each has a name and three properties:
    //(int) length in minimum subdividions, (int) index among the other steps in the track, and (bool) whether the step should play a note
    return juce::ValueTree(juce::Identifier("StepTree" + juce::String(indexInTrack)),
                           {{subDivLengthId, juce::var(numSubDivs)}, {stepIndexId, juce::var(indexInTrack)}, {hasNoteId, juce::var(hasNote)}
    });
}

void StepData::loadStepTree(juce::ValueTree t)
{
    //check if the tree is both valid and was properly created by the getStepTree() function;
    if(t.isValid() && t.hasProperty(subDivLengthId))
    {
        //set relevant variables from the tree's properties
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
    //returns the appropriate MIDI note on message based on the track's analogVoice
    //note that this always goes out on midi channel 1 and always has full velocity
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
        default:
            break;
    }
    return juce::MidiMessage::noteOn(channel, noteNumber, velocity);
}

juce::ValueTree TrackData::getTrackTree()
{
    //each track tree has a name and one property (total length), plus a number of child trees for each step
    auto trkTree = juce::ValueTree(juce::Identifier("TrackTree" + juce::String(trackIndex)), {{totalLengthId, juce::var(totalSubDivs)}});
    for(auto* s : steps)
    {
        //create a sub-tree for each step and add it to the track tree
        trkTree.appendChild(s->getStepTree(), nullptr);
    }
    return trkTree;
}

void TrackData::loadTrackTree(juce::ValueTree t)
{
    totalSubDivs = (int) t.getProperty(totalLengthId);
    auto distinctSteps = t.getNumChildren();
    //if the new track has fewer steps, get rid of the extras
    if(steps.size() > distinctSteps)
    {
        for(int i = steps.size() - 1; i > distinctSteps; --i)
            steps.remove(i, true);
    }
    for(int i = 0; i < distinctSteps; ++i)
    {
        //match the values of each existing step to the corresponding one in the new track
        if(i < steps.size())
        {
            steps[i]->loadStepTree(t.getChild(i));
        }
        //if the new track has more steps than currently exist, create each one as necessary
        else
        {
            auto length = t.getChild(i).getProperty(subDivLengthId);
            steps.add(new StepData(length, i));
        }
    }
}

void TrackData::tupletUp(int firstIndex, int lastIndex)
{
    //make sure there is actually a note to split up
    if(lastIndex - firstIndex > 0)
    {
        //get the selection's total length in subDivs so we know how long each new note needs to be
        auto startNum = lastIndex - firstIndex + 1;
        auto numEnd = startNum + 1;
        auto lengthInSubDivs = 0;
        for(int i = 0; i < startNum; ++i)
        {
            lengthInSubDivs += steps[firstIndex + i]->numSubDivs;
        }
        //delete all the selected steps
        steps.removeRange(firstIndex, startNum);
        //get the length of the new tuplet steps, create each one in place inside the OwnedArray
        auto newLength = lengthInSubDivs / numEnd;
        for(int i = 0; i < numEnd; ++i)
        {
            steps.insert(firstIndex + i, new StepData(newLength, firstIndex + i));
        }
        //renumber all the steps in order so we don't end up with every step after the tuplet having an index 1 lower than it should be
        int ind = 0;
        for(auto* s : steps)
        {
            s->indexInTrack = ind;
            ++ind;
        }
        //recalculate the starting subdivision index for each step
        setStartSubDivs();
    }
}

void TrackData::tupletDown(int firstIndex, int lastIndex)
{
    //we need at least 2 steps to combine
    if(lastIndex - firstIndex > 1)
    {
        auto startNum = lastIndex - firstIndex + 1;
        //this function is just the same as tupletUp(), except endNum is one less than the starting number rather than one more
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
    //update variables based on sample rate and tempo such that the sequence can advance based on counting samples correctly
    sampleRate = rate;
    secsPerSubDiv = 1.0f / (4.0f * MIN_SUBDIV * (TEMPO / 60.0f));
    samplesPerSubDiv = secsPerSubDiv * sampleRate;
}
void SequenceProcessor::advanceBySamples(int numSamples)
{
    //this variable only keeps track of the offset of the current sample from the beginning of a given subdivision, not which subdivision we should be at
    samplesIntoSubDiv += numSamples;
    //one buffer length is usually several times the minimum subdivision
    if(samplesIntoSubDiv >= samplesPerSubDiv)
    {
        //figure out how many subdivisions this buffer should advance the playhead
        samplesIntoSubDiv -= (samplesPerSubDiv * floor(numSamples / samplesPerSubDiv));
        currentSubDiv += floor(numSamples / samplesPerSubDiv);
    }
    //go back to the first step if we've passed the last
    if(currentSubDiv > totalSubDivs)
        currentSubDiv = 0;
    //tell each track where it needs to be and send any MIDI messages
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
    //sets variables in the same way as setSampleRate
    //denominator is multiplied by 4 bc each step represents a 16th note where the tempo is in quarter notes
    secsPerSubDiv = 1.0f / (4.0f * MIN_SUBDIV * (TEMPO / 60.0f));
    samplesPerSubDiv = secsPerSubDiv * sampleRate;
}

juce::ValueTree SequenceProcessor::getSequenceTree()
{
    //name and two properties: length in subdivs and tempo
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
    //TODO: digital audio track support will need to verify and adjust to the number of tracks here. This works fine with just the 7 analog voices
    for(auto* trk : tracks)
    {
        if(t.getChild(i).isValid())
        {
            trk->loadTrackTree(t.getChild(i));
            ++i;
        }
    }
}
