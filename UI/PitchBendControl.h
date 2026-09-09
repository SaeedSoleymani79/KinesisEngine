#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "../PluginProcessor.h"

// --- 2. NEUMORPHIC PITCH BEND ---
class PitchBendControl : public juce::Component {
public:
    PitchBendControl(VirtualMidiKeyAudioProcessor& p) : processor(p) {
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour(NeumorphicTheme::recessDk());
        g.fillRoundedRectangle(bounds.translated(1, 1), 6.0f);
        g.setColour(NeumorphicTheme::recessLt());
        g.fillRoundedRectangle(bounds.translated(-1, -1), 6.0f);
        g.setColour(NeumorphicTheme::bgBase().darker(0.02f));
        g.fillRoundedRectangle(bounds, 6.0f);
        
        float thumbHeight = 14.0f; 
        float thumbY = (bounds.getHeight() - thumbHeight) * (1.0f - pbValue);
        
        g.setColour(NeumorphicTheme::recessLt());
        g.drawHorizontalLine((int)(bounds.getHeight() / 2.0f), 4.0f, bounds.getWidth() - 4.0f);

        auto thumbRect = juce::Rectangle<float>(6.0f, thumbY, bounds.getWidth() - 12.0f, thumbHeight); 
        g.setColour(NeumorphicTheme::recessDk());
        g.fillRoundedRectangle(thumbRect.translated(2, 2), 4.0f);
        
        g.setColour(isHovered ? NeumorphicTheme::ledAmber() : NeumorphicTheme::ledAmber().withAlpha(0.85f));
        g.fillRoundedRectangle(thumbRect, 4.0f);
    }
    
    void setPitchBend(float normalizedValue) {
        pbValue = juce::jlimit(0.0f, 1.0f, normalizedValue);
        int pbMidi = (int)(pbValue * 16383.0f);
        
        // NOTE: This assumes MidiQueueEvent is defined in your PluginProcessor.h
        processor.midiQueue.enqueue(MidiQueueEvent{false, false, false, true, 0, pbMidi});
        repaint();
    }
    
    void mouseEnter(const juce::MouseEvent&) override { isHovered = true; repaint(); }
    void mouseExit(const juce::MouseEvent&) override { isHovered = false; repaint(); }
    void mouseDown(const juce::MouseEvent& e) override { handleDrag(e); }
    void mouseDrag(const juce::MouseEvent& e) override { handleDrag(e); }
    void mouseUp(const juce::MouseEvent&) override { setPitchBend(0.5f); }
    
private:
    void handleDrag(const juce::MouseEvent& e) { setPitchBend(1.0f - ((float)e.y / (float)getHeight())); }
    VirtualMidiKeyAudioProcessor& processor;
    float pbValue{0.5f};
    bool isHovered{false};
};