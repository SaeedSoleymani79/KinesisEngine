#pragma once
#include <JuceHeader.h>
#include <memory>
#include "Theme.h"
#include "PhysicsSlider.h"
#include "MiniAutomationGraph.h"
#include "../PluginProcessor.h" 

// --- 4. HARDWARE FADERS (WITH AUTOMATION GRAPHS) ---
class CinematicFaderPad : public juce::Component {
public:
    CinematicFaderPad(VirtualMidiKeyAudioProcessor& p) : processor(p) {
        for (int i = 1; i <= 127; ++i) {
            juce::String ccName = getCCName(i);
            juce::String label = "CC " + juce::String(i);
            if (ccName.isNotEmpty()) { label += " - " + ccName; }
            
            cc1Dropdown.addItem(label, i);
            cc2Dropdown.addItem(label, i);
        }
        
        cc1Dropdown.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        cc2Dropdown.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(cc1Dropdown); addAndMakeVisible(cc2Dropdown);

        // 1. Attach Dropdowns to APVTS
        cc1Attach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, "FADER1_CC", cc1Dropdown);
        cc2Attach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, "FADER2_CC", cc2Dropdown);

        addAndMakeVisible(graph1);
        addAndMakeVisible(graph2);

        // Pass the attachment pointers into the setup function
        setupHardwareSlider(fader1, "FADER1_VAL", cc1Dropdown, val1, graph1, fader1Attach);
        setupHardwareSlider(fader2, "FADER2_VAL", cc2Dropdown, val2, graph2, fader2Attach);
    }

    ~CinematicFaderPad() override {
        fader1.setLookAndFeel(nullptr);
        fader2.setLookAndFeel(nullptr);
    }

    void resized() override {
        auto bounds = getLocalBounds().reduced(15); 
        bounds.removeFromTop(25); 
        
        auto bottomRow = bounds.removeFromBottom(24);
        bounds.removeFromBottom(20); 
        
        float fW = (bounds.getWidth() - 40.0f) / 2.0f; 
        
        cc1Dropdown.setBounds(bottomRow.removeFromLeft((int)fW));
        bottomRow.removeFromLeft(40);
        cc2Dropdown.setBounds(bottomRow.removeFromLeft((int)fW));
        
        auto leftColumn = bounds.removeFromLeft((int)fW);
        graph1.setBounds(leftColumn.removeFromTop(40)); 
        leftColumn.removeFromTop(10); 
        fader1.setBounds(leftColumn); 
        
        bounds.removeFromLeft(40); 
        
        auto rightColumn = bounds.removeFromLeft((int)fW);
        graph2.setBounds(rightColumn.removeFromTop(40)); 
        rightColumn.removeFromTop(10); 
        fader2.setBounds(rightColumn);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // DELETED manual CC Parameter polling here!

        g.setColour(NeumorphicTheme::textMain());
        g.setFont(juce::FontOptions("Helvetica Neue", 12.0f, juce::Font::bold));
        g.drawText("Controllers", (int)bounds.getX(), (int)bounds.getY() + 10, (int)bounds.getWidth(), 20, juce::Justification::centred, false);
    }

private:
    static juce::String getCCName(int ccNumber) {
        switch (ccNumber) {
            case 1:  return "Modulation";
            case 2:  return "Breath Controller";
            case 4:  return "Foot Controller";
            case 5:  return "Portamento Time";
            case 7:  return "Volume";
            case 8:  return "Balance";
            case 10: return "Pan";
            case 11: return "Expression";
            case 64: return "Sustain Pedal";
            case 65: return "Portamento On/Off";
            case 67: return "Soft Pedal";
            case 68: return "Legato Pedal";
            case 71: return "Resonance / Timbre";
            case 74: return "Cutoff / Brightness";
            default: return ""; 
        }
    }

    void setupHardwareSlider(juce::Slider& slider, const juce::String& paramId, juce::ComboBox& dropdown, uint8_t& valStore, MiniAutomationGraph& graph, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attach) {
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        
        // APVTS now strictly handles the range (0.0 to 1.0)
        slider.setLookAndFeel(&hardwareTheme); 
        slider.setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        
        // 2. Attach Fader to APVTS
        attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, paramId, slider);
        
        // 3. Keep the lambda ONLY to draw the visual graph and push to our CC Queue
        slider.onValueChange = [this, &slider, &dropdown, &valStore, &graph] {
            float normVal = (float)slider.getValue();
            uint8_t val = (uint8_t)(normVal * 127.0f);
            
            graph.pushValue(normVal);
            
            if (valStore != val) { 
                valStore = val; 
                // Enqueue using the new MidiQueueEvent struct size
                processor.midiQueue.enqueue({true, false, false, false, dropdown.getSelectedId(), valStore, 0.0f, 60}); 
            }
        };
        addAndMakeVisible(slider);
    }

    VirtualMidiKeyAudioProcessor& processor;
    HardwareLookAndFeel hardwareTheme; 
    
    juce::ComboBox cc1Dropdown, cc2Dropdown;
    PhysicsSlider fader1, fader2;
    MiniAutomationGraph graph1, graph2; 
    uint8_t val1{64}, val2{0};

    // --- NEW: APVTS ATTACHMENTS ---
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cc1Attach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cc2Attach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fader1Attach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fader2Attach;
};