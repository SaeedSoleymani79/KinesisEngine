#pragma once
#include <JuceHeader.h>
#include <unordered_map>
#include <vector>

#include "PluginProcessor.h"
#include "UI/Theme.h"
#include "UI/MiniAutomationGraph.h"
#include "UI/PhysicsSlider.h"
#include "UI/CinematicFaderPad.h"
#include "UI/CinematicMidiKeyboard.h"
#include "UI/PitchBendControl.h"
#include "UI/ScaleAndChordPanel.h"

class VirtualMidiKeyAudioProcessorEditor : public juce::AudioProcessorEditor, 
                                           public juce::KeyListener, 
                                           private juce::Timer 
{
public:
    explicit VirtualMidiKeyAudioProcessorEditor(VirtualMidiKeyAudioProcessor&);
    ~VirtualMidiKeyAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override;
    bool keyStateChanged(bool isKeyDown, juce::Component*) override;
    void timerCallback() override;

private:
    const std::unordered_map<int, int> qwertyToSemitone = {
        {'z', 0}, {'s', 1}, {'x', 2}, {'d', 3}, {'c', 4}, {'v', 5}, 
        {'g', 6}, {'b', 7}, {'h', 8}, {'n', 9}, {'j', 10}, {'m', 11},
        {'q', 12}, {'2', 13}, {'w', 14}, {'3', 15}, {'e', 16}, {'r', 17}, 
        {'5', 18}, {'t', 19}, {'6', 20}, {'y', 21}, {'7', 22}, {'u', 23}
    };
    std::unordered_map<int, int> activeChordKeyToRoot; // Tracks held number keys
    
    // --- RESTORED HELPERS FOR UI SYNC ---
    float getHumanizedVelocity(float baseVelocity); 
    int quantizeNote(int semitoneOffset);
    void triggerNoteOn(int midiNote, float velocity);
    void triggerNoteOff(int midiNote);

    VirtualMidiKeyAudioProcessor& audioProcessor;
    NeumorphicLookAndFeel customLnF;

    ScaleAndChordPanel scaleChordPanel{audioProcessor};
    CinematicFaderPad faderPad{audioProcessor};
    PitchBendControl pitchBend{audioProcessor};
    
    // Bound directly to the processor's state so it perfectly syncs!
    CinematicMidiKeyboard keyboardComponent{audioProcessor.keyboardState, audioProcessor}; 

    juce::ToggleButton themeToggle;
    juce::Rectangle<float> lcdArea;
    juce::Rectangle<float> octDownArea;
    juce::Rectangle<float> octUpArea;

    int currentOctaveOffset{60}; 
    float midiActivityLevel { 0.0f };
    std::unordered_map<int, int> activeKeyToNote;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualMidiKeyAudioProcessorEditor)
};