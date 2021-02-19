/*
  ==============================================================================

    SequenceComponent.h
    Created: 18 Feb 2021 9:29:25pm
    Author:  Hayden Setlik

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SequenceProcessor.h"
#include "RGBColor.h"

class StepComponent : public juce::ShapeButton
{
public:
    StepComponent(int start, int length);
    ~StepComponent() {}
    void paintButton(juce::Graphics& g, bool mouseIsOver, bool mouseIsDown) override;
    void toggleNote()
    {
        isNote = !isNote;
    }
    bool isNote;
    bool isSelected;
    stepState state;
    int trackIndex;
    int stepIndex;
};

class TrackLabelComponent : public juce::Component
{
public:
    TrackLabelComponent(analogVoice type) : voiceType(type)
    {
        setInterceptsMouseClicks(false, false);
        setWantsKeyboardFocus(false);
        bkgnd = color.RGBColor(70, 69, 75);
        switch(type)
        {
            case kick1:
            {
                image = juce::ImageCache::getFromMemory(BinaryData::kick10_5x_png, 1452);
                imgW = 51;
                imgH = 51;
                break;
            }
            case kick2:
            {
                image = juce::ImageCache::getFromMemory(BinaryData::kick20_5x_png, 1453);
                imgW = 51;
                imgH = 51;
                break;
            }
            case openHat:
            {
                image = juce::ImageCache::getFromMemory(BinaryData::openHat0_5x_png, 1195);
                imgW = 67;
                imgH = 38;
                break;
            }
            case closedHat:
            {
                image = juce::ImageCache::getFromMemory(BinaryData::closedHat0_5x_png, 1072);
                imgW = 67;
                imgH = 38;
                break;
            }
            case snare:
            {
                image = juce::ImageCache::getFromMemory(BinaryData::snare0_5x_png, 1717);
                imgW = 63;
                imgH = 53;
                break;
            }
            case clave:
            {
                image = juce::ImageCache::getFromMemory(BinaryData::clave0_5x_png, 1236);
                imgW = 55;
                imgH = 36;
                break;
            }
            case clap:
            {
                image = juce::ImageCache::getFromMemory(BinaryData::clap0_5x_png, 1629);
                imgW = 50;
                imgH = 58;
                break;
            }
        }
    }
    ~TrackLabelComponent() {}
    void paint(juce::Graphics& g) override
    {
        g.fillAll(bkgnd);
        g.setOpacity(1.0f);
        int destX = (getWidth() - imgW) / 2;
        int destY = (getHeight() - imgH) / 2;
        g.drawImage(image, destX, destY, imgW, imgH, 0, 0, imgW, imgH);
        juce::Rectangle<int> border = getLocalBounds();
        g.setColour(juce::Colours::black);
        g.drawRect(border);
    }
private:
    int imgW;
    int imgH;
    Color color;
    juce::Colour bkgnd;
    analogVoice voiceType;
    juce::Image image;
};


class TrackComponent : public juce::Component
{
public:
    TrackComponent(analogVoice v);
    ~TrackComponent() {}
    void resized() override;
    void paint(juce::Graphics& g) override;
    
private:
    TrackLabelComponent label;
    analogVoice voiceType;
    juce::OwnedArray<StepComponent> stepButtons;
};
