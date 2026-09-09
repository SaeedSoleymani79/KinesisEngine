#include "PluginProcessor.h"
#include "PluginEditor.h"

VirtualMidiKeyAudioProcessor::VirtualMidiKeyAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{}

VirtualMidiKeyAudioProcessor::~VirtualMidiKeyAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout VirtualMidiKeyAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Fader Settings
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FADER1_VAL", "Fader 1 Value", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FADER2_VAL", "Fader 2 Value", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FADER1_CC", "Fader 1 CC", 1.0f, 127.0f, 11.0f)); // Expression
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FADER2_CC", "Fader 2 CC", 1.0f, 127.0f, 1.0f));  // Mod Wheel

    // Musical Engine Settings
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GLOBAL_SCALE", "Musical Scale", 0.0f, 4.0f, 0.0f)); 
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GLOBAL_ROOT", "Root Note", 0.0f, 11.0f, 0.0f)); 
    params.push_back(std::make_unique<juce::AudioParameterBool>("STRICT_QUANTIZE", "Strict Quantize", false)); 
    params.push_back(std::make_unique<juce::AudioParameterBool>("CHORD_MODE_ON", "Chord Mode On", true)); 
    
    // Pad Settings
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GLOBAL_CHORD", "Active Pad", 0.0f, 5.0f, 0.0f));
    int defaultRoots[6] = {0, 2, 4, 5, 7, 9};
    int defaultTypes[6] = {1, 2, 2, 1, 1, 2}; 

    for (int i = 0; i < 6; ++i) {
        params.push_back(std::make_unique<juce::AudioParameterFloat>("PAD_" + std::to_string(i) + "_ROOT", "Pad Root", 0.0f, 11.0f, (float)defaultRoots[i]));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("PAD_" + std::to_string(i) + "_TYPE", "Pad Type", 0.0f, 7.0f, (float)defaultTypes[i]));
    }

    return { params.begin(), params.end() };
}

void VirtualMidiKeyAudioProcessor::prepareToPlay (double, int) {}
void VirtualMidiKeyAudioProcessor::releaseResources() {}

void VirtualMidiKeyAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    buffer.clear();
    midiMessages.clear();

    MidiQueueEvent event;
    while (midiQueue.try_dequeue(event)) {
        if (event.isPitchBend) {
            midiMessages.addEvent(juce::MidiMessage::pitchWheel(1, event.value), 0);
        } else if (event.isCC) {
            midiMessages.addEvent(juce::MidiMessage::controllerEvent(1, event.number, event.value), 0);
        } else if (event.isNoteOn) {
            midiMessages.addEvent(juce::MidiMessage::noteOn(1, event.number, (uint8_t)event.value), 0);
        } else {
            midiMessages.addEvent(juce::MidiMessage::noteOff(1, event.number, (uint8_t)0), 0);
        }
    }
    float maxLevel = 0.0f;

    // Scan all incoming MIDI events in this block to find the highest value
    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            maxLevel = juce::jmax(maxLevel, msg.getFloatVelocity()); // 0.0 to 1.0
        } 
        else if (msg.isController() || msg.isPitchWheel()) {
            // Map 14-bit pitch wheel or 7-bit CC to a 0.0 - 1.0 range
            float val = msg.isPitchWheel() ? (msg.getPitchWheelValue() / 16383.0f) 
                                           : (msg.getControllerValue() / 127.0f);
            maxLevel = juce::jmax(maxLevel, val);
        }
    }

    // Send the highest detected level to the UI
    if (maxLevel > 0.0f) {
        currentMidiLevel.store(maxLevel, std::memory_order_relaxed);
    }

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
}

void VirtualMidiKeyAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void VirtualMidiKeyAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (parameters.state.getType())) {
        parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
    }
}

juce::AudioProcessorEditor* VirtualMidiKeyAudioProcessor::createEditor() {
    return new VirtualMidiKeyAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new VirtualMidiKeyAudioProcessor();
}