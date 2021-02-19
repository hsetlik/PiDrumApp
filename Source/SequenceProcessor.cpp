/*
  ==============================================================================

    SequenceProcessor.cpp
    Created: 18 Feb 2021 1:13:54pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#include "SequenceProcessor.h"

juce::OwnedArray<TrackData> SequenceProcessor::tracks = juce::OwnedArray<TrackData>();

SequenceProcessor::SequenceProcessor()
{
    TEMPO = 120;
    tracks.clear();
    for(int i = 0; i < NUM_TRACKS; ++i)
    {
        tracks.add(new TrackData(NUM_NOTES, i));
    }
}
