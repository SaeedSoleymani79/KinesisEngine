#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <vector>
#include "Core/readerwriterqueue.h"

// UPDATED: Now carries full Note data
struct MidiQueueEvent {
    bool isCC = false;
    bool isNoteOn = false;
    bool isNoteOff = false;
    bool isPitchBend = false;
    int number = 0;        // CC number or raw semitone (0-23)
    int value = 0;         // CC or PB Value
    float velocity = 0.0f; // Base spatial velocity
    int octaveOffset = 60; // Current editor octave
};

class VirtualMidiKeyAudioProcessor : public juce::AudioProcessor {
public:
    VirtualMidiKeyAudioProcessor();
    ~VirtualMidiKeyAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "VirtualMidiKey"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int /*index*/) override {}
    const juce::String getProgramName(int /*index*/) override { return {}; }
    void changeProgramName(int /*index*/, const juce::String& /*newName*/) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    moodycamel::ReaderWriterQueue<MidiQueueEvent> midiQueue{512};
    juce::MidiKeyboardState keyboardState;
    
    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float> currentMidiLevel { 0.0f };
    
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualMidiKeyAudioProcessor)
};