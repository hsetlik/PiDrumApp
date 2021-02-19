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

const int LABEL_WIDTH = 80;
const int HEADER_HEIGHT = 45;

class StepComponent : public juce::ShapeButton
{
public:
    StepComponent(int trackIndex, int sequenceIndex, SequenceProcessor* p);
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
    SequenceProcessor* proc;
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


class TrackComponent : public juce::Component, juce::Button::Listener
{
public:
    TrackComponent(analogVoice v, SequenceProcessor* p);
    ~TrackComponent() {}
    void resized() override;
    void paint(juce::Graphics& g) override
    {
        for(auto* s : stepButtons)
            s->repaint();
    }
    StepComponent* stepAtXPos(int xPos);
    void buttonClicked(juce::Button* b) override;
    void mouseDown(const juce::MouseEvent& m) override;
    void mouseDrag(const juce::MouseEvent& m) override;
    void selectStep(StepComponent* toSelect);
    void clearSelection();
    static std::vector<StepComponent*> selectedSteps;
private:
    double subDivWidth;
    TrackLabelComponent label;
    analogVoice voiceType;
    juce::OwnedArray<StepComponent> stepButtons;
    SequenceProcessor* proc;
};

class SequenceHeader : public juce::Component
{
public:
    SequenceHeader(juce::String seqName, SequenceProcessor* p) : name(seqName), proc(p)
    {
        background = color.RGBColor(42, 58, 112);
    }
    ~SequenceHeader() {}
    void paint(juce::Graphics& g) override
    {
        g.fillAll(background);
        auto n = getHeight() / 6;
        g.setColour(text);
        g.setFont(juce::Font("Press Start 2P", 15.0f, 0));
        g.drawSingleLineText(name, n, 5 * n);
        auto tempoString = "Tempo: " + juce::String(proc->TEMPO);
        auto x = getWidth() / 3;
        g.drawSingleLineText(tempoString, 2 * x, 5 * n);
    }
private:
    juce::Colour background = juce::Colours::black;
    juce::Colour text = juce::Colours::white;
    Color color;
    juce::String name;
    SequenceProcessor* proc;
};

class SequenceComponent : public juce::Component, juce::Timer
{
public:
    SequenceComponent(SequenceProcessor* p);
    ~SequenceComponent() {}
    void timerCallback() override;
    void paint(juce::Graphics& g) override
    {
        for(auto* t : trackComponents)
            t->repaint();
    }
    void resized() override;
    bool keyPressed(const juce::KeyPress &p) override;
private:
    juce::OwnedArray<TrackData>* trackData;
    SequenceHeader header;
    juce::OwnedArray<TrackComponent> trackComponents;
    SequenceProcessor* proc;
};
