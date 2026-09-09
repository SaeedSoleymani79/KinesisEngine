#pragma once
#include <JuceHeader.h>
#include <vector>
#include <memory>
#include "Theme.h"
#include "../PluginProcessor.h"

// --- 3. NEUMORPHIC SCALE & CHORD PAD MODULE ---
class ScaleAndChordPanel : public juce::Component {
public:
    ScaleAndChordPanel(VirtualMidiKeyAudioProcessor& p) : processor(p) {
        scaleBox.addItem("Chromatic", 1); scaleBox.addItem("Major", 2); 
        scaleBox.addItem("Natural Minor", 3); scaleBox.addItem("Dorian", 4); scaleBox.addItem("Phrygian", 5);
        scaleBox.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(scaleBox);
        
        // 1. Attach Scale Dropdown to APVTS
        scaleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, "GLOBAL_SCALE", scaleBox);

        for (int i=0; i<12; ++i) rootBox.addItem(noteNames[i], i+1);
        rootBox.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(rootBox);
        
        // 2. Attach Root Dropdown to APVTS
        rootAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, "GLOBAL_ROOT", rootBox);

        chordModeToggle.setButtonText("CHORD MODE");
        chordModeToggle.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(chordModeToggle);
        
        // 3. Attach Toggle Button to APVTS
        chordModeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "CHORD_MODE_ON", chordModeToggle);

        for (int i = 0; i < 6; ++i) {
            auto* rBox = padRootBoxes.add(new juce::ComboBox());
            for (int n = 0; n < 12; ++n) rBox->addItem(noteNames[n], n + 1);
            rBox->setMouseCursor(juce::MouseCursor::PointingHandCursor);
            addAndMakeVisible(rBox);
            
            // 4. Attach Pad Roots
            padRootAttaches.add(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(processor.parameters, "PAD_" + juce::String(i) + "_ROOT", *rBox));

            auto* tBox = padTypeBoxes.add(new juce::ComboBox());
            tBox->addItem("OFF", 1); tBox->addItem("Major", 2); tBox->addItem("Minor", 3); 
            tBox->addItem("Dim", 4); tBox->addItem("Sus4", 5); tBox->addItem("Min7", 6); 
            tBox->addItem("Maj7", 7); tBox->addItem("5th", 8);
            tBox->setMouseCursor(juce::MouseCursor::PointingHandCursor);
            addAndMakeVisible(tBox);
            
            // 5. Attach Pad Types
            padTypeAttaches.add(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(processor.parameters, "PAD_" + juce::String(i) + "_TYPE", *tBox));
        }

        // We KEEP the onChange lambdas ONLY for visual repaints and generating presets!
        // The APVTS handles updating the actual backend math automatically.
        scaleBox.onChange = [this] { populateDefaultChords(); getParentComponent()->repaint(); };
        rootBox.onChange = [this] { populateDefaultChords(); getParentComponent()->repaint(); };
        chordModeToggle.onClick = [this] { repaint(); };
    }

    void populateDefaultChords() {
        int scale = scaleBox.getSelectedId() - 1;
        int root = rootBox.getSelectedId() - 1;
        std::vector<int> degrees = {0, 2, 4, 5, 7, 9}; 
        std::vector<int> types = {1, 2, 2, 1, 1, 2}; 
        
        if (scale == 1) { degrees = {0, 2, 4, 5, 7, 9}; types = {1, 2, 2, 1, 1, 2}; } 
        else if (scale == 2) { degrees = {0, 2, 3, 5, 7, 8}; types = {2, 3, 1, 2, 2, 1}; } 
        else if (scale == 3) { degrees = {0, 2, 3, 5, 7, 9}; types = {2, 2, 1, 1, 2, 3}; } 
        else if (scale == 4) { degrees = {0, 1, 3, 5, 7, 8}; types = {2, 1, 1, 2, 3, 1}; } 

        for (int i = 0; i < 6; ++i) {
            int chordRoot = (root + degrees[i]) % 12;
            // Because these are attached to APVTS, setting the ID here safely updates the parameters too!
            padRootBoxes[i]->setSelectedId(chordRoot + 1, juce::sendNotificationAsync);
            padTypeBoxes[i]->setSelectedId(types[i] + 1, juce::sendNotificationAsync);
        }
    }

    void resized() override {
        auto bounds = getLocalBounds().reduced(15);
        auto topRow = bounds.removeFromTop(28); 
        rootBox.setBounds(topRow.removeFromLeft(60).reduced(0, 2));
        topRow.removeFromLeft(10); 
        scaleBox.setBounds(topRow.removeFromLeft(120).reduced(0, 2));
        chordModeToggle.setBounds(topRow.removeFromRight(140)); 
        
        bounds.removeFromTop(15); 
        float padW = (bounds.getWidth() - 20.0f) / 3.0f; 
        float padH = (bounds.getHeight() - 20.0f) / 2.0f;
        padFullRects.clear();

        for (int i = 0; i < 6; ++i) {
            float px = bounds.getX() + (i % 3) * (padW + 10.0f);
            float py = bounds.getY() + (i / 3) * (padH + 10.0f);
            auto padArea = juce::Rectangle<int>((int)px, (int)py, (int)padW, (int)padH);
            auto dropdownRow = padArea.removeFromTop(26).reduced(2, 2);
            padRootBoxes[i]->setBounds(dropdownRow.removeFromLeft(dropdownRow.getWidth() / 2).reduced(1));
            padTypeBoxes[i]->setBounds(dropdownRow.reduced(1));
            padFullRects.push_back(padArea.toFloat());
        }
    }

    void mouseMove(const juce::MouseEvent& e) override {
        int newHover = -1;
        for (int i = 0; i < 6; ++i) { if (padFullRects[i].contains(e.position)) { newHover = i; break; } }
        if (hoveredPad != newHover) { hoveredPad = newHover; repaint(); }
        if (hoveredPad != -1) setMouseCursor(juce::MouseCursor::PointingHandCursor);
        else setMouseCursor(juce::MouseCursor::NormalCursor);
    }
    
    void mouseExit(const juce::MouseEvent&) override { hoveredPad = -1; repaint(); }

    void paint(juce::Graphics& g) override {
        // NOTICE: We completely deleted the manual Box updates here! 
        // We only read parameters now for drawing the illuminated highlights.
        auto* toggleP = processor.parameters.getRawParameterValue("CHORD_MODE_ON");
        auto* activeChordP = processor.parameters.getRawParameterValue("GLOBAL_CHORD");
        
        int activeChord = activeChordP ? (int)*activeChordP : 0;
        bool isChordMode = toggleP ? (*toggleP > 0.5f) : false;

        for (int i = 0; i < 6; ++i) {
            bool isActive = (i == activeChord && isChordMode);
            bool isHovered = (i == hoveredPad);
            
            if (isActive) {
                g.setColour(NeumorphicTheme::bgBase().darker(0.04f));
                g.fillRoundedRectangle(padFullRects[i], 8.0f);
                g.setColour(NeumorphicTheme::recessDk());
                g.drawRoundedRectangle(padFullRects[i].translated(1, 1), 8.0f, 2.0f);
                g.setColour(NeumorphicTheme::ledAmber().withAlpha(isHovered ? 0.25f : 0.15f));
                g.fillRoundedRectangle(padFullRects[i].reduced(2.0f), 6.0f);
            } else {
                g.setColour(NeumorphicTheme::recessDk()); 
                g.fillRoundedRectangle(padFullRects[i].translated(4, 4), 8.0f);
                g.setColour(NeumorphicTheme::recessLt()); 
                g.fillRoundedRectangle(padFullRects[i].translated(-2, -2), 8.0f);
                g.setColour(isHovered ? NeumorphicTheme::recessLt().withAlpha(0.2f) : NeumorphicTheme::bgBase()); 
                g.fillRoundedRectangle(padFullRects[i], 8.0f);
            }

            g.setColour((isActive || isHovered) ? NeumorphicTheme::ledAmber() : NeumorphicTheme::textMain());
            g.setFont(juce::FontOptions("Helvetica Neue", 16.0f, juce::Font::bold));
            juce::String padLabel = padRootBoxes[i]->getText() + " " + padTypeBoxes[i]->getText();
            g.drawText(padLabel, (int)padFullRects[i].getX(), (int)padFullRects[i].getY() + 15, (int)padFullRects[i].getWidth(), (int)padFullRects[i].getHeight() - 15, juce::Justification::centred, false);
            
            g.setFont(juce::FontOptions("Helvetica Neue", 10.0f, juce::Font::bold));
            g.setColour(isActive ? NeumorphicTheme::ledAmber() : NeumorphicTheme::textMain().withAlpha(0.6f));
            g.drawText(juce::String(i+1), (int)padFullRects[i].getX() + 8, (int)padFullRects[i].getY() + 6, 20, 20, juce::Justification::topLeft, false);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        for (int i = 0; i < 6; ++i) {
            if (padFullRects[i].contains(e.position)) {
                if (auto* p = processor.parameters.getParameter("GLOBAL_CHORD")) p->setValueNotifyingHost(i / 5.0f);
                repaint(); getParentComponent()->repaint();
                break;
            }
        }
    }

private:
    VirtualMidiKeyAudioProcessor& processor;
    juce::ComboBox scaleBox, rootBox;
    juce::ToggleButton chordModeToggle;
    juce::OwnedArray<juce::ComboBox> padRootBoxes;
    juce::OwnedArray<juce::ComboBox> padTypeBoxes;
    std::vector<juce::Rectangle<float>> padFullRects;
    int hoveredPad{-1};
    juce::String noteNames[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

    // --- NEW: APVTS ATTACHMENTS ---
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rootAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> chordModeAttach;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> padRootAttaches;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> padTypeAttaches;
};