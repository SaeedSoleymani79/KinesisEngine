#pragma once
#include <JuceHeader.h>
#include <vector>

class MidiEngine {
public:
    static void processQueueEvent(const MidiQueueEvent& event, 
                                  juce::MidiBuffer& midiMessages, 
                                  juce::AudioProcessorValueTreeState& apvts, 
                                  juce::MidiKeyboardState& kbState) 
    {
        if (event.isPitchBend) {
            midiMessages.addEvent(juce::MidiMessage::pitchWheel(1, event.value), 0);
        } 
        else if (event.isCC) {
            midiMessages.addEvent(juce::MidiMessage::controllerEvent(1, event.number, event.value), 0);
        } 
        else if (event.isNoteOn || event.isNoteOff) {
            // Apply scale quantization using APVTS
            int quantizedNote = event.octaveOffset + quantizeNote(event.number, apvts);
            
            auto* toggleP = apvts.getRawParameterValue("CHORD_MODE_ON");
            bool chordModeActive = toggleP ? (*toggleP > 0.5f) : false;

            if (event.isNoteOn) {
                float finalVel = getHumanizedVelocity(event.velocity);
                kbState.noteOn(1, quantizedNote, finalVel);
                
                if (chordModeActive) generateChord(quantizedNote, finalVel, true, apvts, kbState);
            } 
            else {
                kbState.noteOff(1, quantizedNote, 0.0f);
                
                if (chordModeActive) generateChord(quantizedNote, 0.0f, false, apvts, kbState);
            }
        }
    }

private:
    static float getHumanizedVelocity(float baseVelocity) {
        auto& random = juce::Random::getSystemRandom();
        float variance = (((random.nextFloat() + random.nextFloat()) / 2.0f) * 0.30f) - 0.15f; 
        return juce::jlimit(0.1f, 1.0f, baseVelocity + variance);
    }

    static int quantizeNote(int semitoneOffset, juce::AudioProcessorValueTreeState& apvts) {
        int scale = (int)*apvts.getRawParameterValue("GLOBAL_SCALE");
        float strict = *apvts.getRawParameterValue("STRICT_QUANTIZE");
        if (scale == 0 || strict < 0.5f) return semitoneOffset; 

        int root = (int)*apvts.getRawParameterValue("GLOBAL_ROOT");
        static const std::vector<int> maj = {0, 2, 4, 5, 7, 9, 11};
        static const std::vector<int> min = {0, 2, 3, 5, 7, 8, 10};
        static const std::vector<int> dor = {0, 2, 3, 5, 7, 9, 10};
        static const std::vector<int> phr = {0, 1, 3, 5, 7, 8, 10};

        const auto& activeScale = (scale == 1) ? maj : (scale == 2) ? min : (scale == 3) ? dor : phr;

        int inputDegree = (semitoneOffset - root) % 12;
        if (inputDegree < 0) inputDegree += 12;
        int octave = (semitoneOffset - root) / 12;
        if (semitoneOffset - root < 0) octave--;

        int closest = activeScale[0]; int minDiff = 12;
        for (int interval : activeScale) {
            if (std::abs(inputDegree - interval) < minDiff) { 
                minDiff = std::abs(inputDegree - interval); closest = interval; 
            }
        }
        return root + (octave * 12) + closest;
    }

    static void generateChord(int rootNote, float velocity, bool isNoteOn, juce::AudioProcessorValueTreeState& apvts, juce::MidiKeyboardState& kbState) {
        auto* chordP = apvts.getRawParameterValue("GLOBAL_CHORD");
        int activePad = chordP ? (int)*chordP : 0;
        auto* typeP = apvts.getRawParameterValue("PAD_" + juce::String(activePad) + "_TYPE");
        int mode = typeP ? (int)*typeP : 0;

        float chordVel = velocity * 0.85f; 
        auto play = [&](int offset) {
            if (isNoteOn) kbState.noteOn(1, rootNote + offset, chordVel);
            else kbState.noteOff(1, rootNote + offset, 0.0f);
        };

        if (mode == 1) { play(4); play(7); } 
        else if (mode == 2) { play(3); play(7); } 
        else if (mode == 3) { play(3); play(6); } 
        else if (mode == 4) { play(5); play(7); } 
        else if (mode == 5) { play(3); play(7); play(10); } 
        else if (mode == 6) { play(4); play(7); play(11); } 
        else if (mode == 7) { play(7); play(12); } 
    }
};