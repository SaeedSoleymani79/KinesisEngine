#pragma once
#include <JuceHeader.h>
#include <vector>
#include <algorithm>
#include "Theme.h"
#include "../PluginProcessor.h"

// --- 1. NEUMORPHIC KEYBOARD ---
class CinematicMidiKeyboard : public juce::MidiKeyboardComponent {
public:
    CinematicMidiKeyboard(juce::MidiKeyboardState& state, VirtualMidiKeyAudioProcessor& p) 
        : juce::MidiKeyboardComponent(state, juce::MidiKeyboardComponent::horizontalKeyboard), processor(p) {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    bool isNoteInScale(int midiNote) {
        auto* scaleP = processor.parameters.getRawParameterValue("GLOBAL_SCALE");
        auto* rootP = processor.parameters.getRawParameterValue("GLOBAL_ROOT");
        int scale = scaleP ? (int)*scaleP : 0;
        if (scale == 0) return true; 

        int root = rootP ? (int)*rootP : 0;
        int degree = (midiNote - root) % 12;
        if (degree < 0) degree += 12;

        static const std::vector<int> maj = {0, 2, 4, 5, 7, 9, 11};
        static const std::vector<int> min = {0, 2, 3, 5, 7, 8, 10};
        static const std::vector<int> dor = {0, 2, 3, 5, 7, 9, 10};
        static const std::vector<int> phr = {0, 1, 3, 5, 7, 8, 10};

        const auto& activeScale = (scale == 1) ? maj : (scale == 2) ? min : (scale == 3) ? dor : phr;
        return std::find(activeScale.begin(), activeScale.end(), degree) != activeScale.end();
    }

    void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver, juce::Colour, juce::Colour) override {
        bool inScale = isNoteInScale(midiNoteNumber);
        
        // Note: Assuming NeumorphicTheme::isLight is defined in your Theme.h or you can just default this to false/true
        bool isLight = false; 
        
        auto baseColor = isLight ? (inScale ? juce::Colour(0xffffffff) : juce::Colour(0xffe5e7eb)) 
                                 : (inScale ? juce::Colour(0xffe6e8eb) : juce::Colour(0xff9a9ea3)); 
        
        g.setColour(isDown ? NeumorphicTheme::ledAmber().withAlpha(0.6f) : (isOver ? baseColor.brighter(0.1f) : baseColor));
        g.fillRect(area);
        g.setColour(NeumorphicTheme::recessLt()); // Replaced missing outline()
        g.drawRect(area, 1.0f);

        if (midiNoteNumber % 12 == 0) {
            int octave = (midiNoteNumber / 12) - 2;
            g.setColour(NeumorphicTheme::ledAmber());
            g.setFont(juce::FontOptions("Helvetica Neue", 13.0f, juce::Font::bold));
            g.drawText("C" + juce::String(octave), area.withTrimmedBottom(4.0f).toNearestInt(), juce::Justification::centredBottom, false);
        }
    }

    void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver, juce::Colour) override {
        bool inScale = isNoteInScale(midiNoteNumber);
        
        bool isLight = false;
        
        auto baseColor = isLight ? (inScale ? juce::Colour(0xff757d8a) : juce::Colour(0xffa1a8b3)) 
                                 : (inScale ? juce::Colour(0xff2b2d31) : juce::Colour(0xff1e1f22)); 
        
        auto blackKeyArea = area.reduced(2.0f, 0.0f).withTrimmedBottom(6.0f);
        g.setColour(isDown ? NeumorphicTheme::ledAmber().withAlpha(0.8f) : (isOver ? baseColor.brighter(0.2f) : baseColor));
        g.fillRoundedRectangle(blackKeyArea, 3.0f);
        
        if (!isDown && inScale) {
            g.setColour(NeumorphicTheme::recessLt().withAlpha(0.2f)); // Replaced missing shadowLt()
            g.drawRoundedRectangle(blackKeyArea.translated(0, -1), 3.0f, 1.0f);
        }
    }
private:
    VirtualMidiKeyAudioProcessor& processor;
};