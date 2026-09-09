#include "PluginProcessor.h"
#include "PluginEditor.h"

VirtualMidiKeyAudioProcessorEditor::VirtualMidiKeyAudioProcessorEditor(VirtualMidiKeyAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), keyboardComponent(p.keyboardState, p)
{
    customLnF.updateColors();
    setLookAndFeel(&customLnF);

    addAndMakeVisible(scaleChordPanel);
    addAndMakeVisible(faderPad);
    addAndMakeVisible(pitchBend);
    addAndMakeVisible(keyboardComponent);
    
    themeToggle.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    themeToggle.setButtonText("TOGGLE THEME");
    addAndMakeVisible(themeToggle);
    
    themeToggle.onClick = [this] {
        NeumorphicTheme::isLightMode = !NeumorphicTheme::isLightMode;
        themeToggle.setToggleState(NeumorphicTheme::isLightMode, juce::dontSendNotification);
        themeToggle.setButtonText(NeumorphicTheme::isLightMode ? "DARK THEME" : "LIGHT THEME");
        customLnF.updateColors();
        sendLookAndFeelChange(); 
        repaint();
        scaleChordPanel.repaint();
        faderPad.repaint();
        pitchBend.repaint();
        keyboardComponent.repaint();
    };
    
    keyboardComponent.setKeyWidth(28.0f); 
    keyboardComponent.setAvailableRange(0, 127); 
    keyboardComponent.setWantsKeyboardFocus(false);
    keyboardComponent.setLowestVisibleKey(currentOctaveOffset); 
    keyboardComponent.setScrollButtonsVisible(false);
    keyboardComponent.setAvailableRange(currentOctaveOffset, 127);
    
    setSize(900, 600);
    setWantsKeyboardFocus(true);
    addKeyListener(this);
    startTimerHz(30);
}

VirtualMidiKeyAudioProcessorEditor::~VirtualMidiKeyAudioProcessorEditor() {
    setLookAndFeel(nullptr); 
    removeKeyListener(this);
    stopTimer();
}

void VirtualMidiKeyAudioProcessorEditor::timerCallback() {
    if (!hasKeyboardFocus(true)) grabKeyboardFocus();
    float newLevel = audioProcessor.currentMidiLevel.exchange(0.0f, std::memory_order_relaxed);
    if (newLevel > midiActivityLevel) midiActivityLevel = newLevel;
    else midiActivityLevel = juce::jmax(0.0f, midiActivityLevel - 0.04f); 
    repaint(lcdArea.toNearestInt()); 
}

void VirtualMidiKeyAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(NeumorphicTheme::bgBase()); 

    g.setColour(NeumorphicTheme::recessDk());
    g.fillRoundedRectangle(lcdArea.translated(1, 1), 6.0f);
    g.setColour(NeumorphicTheme::recessLt());
    g.fillRoundedRectangle(lcdArea.translated(-1, -1), 6.0f);
    g.setColour(juce::Colour(0xff121315)); 
    g.fillRoundedRectangle(lcdArea, 6.0f);
    
    juce::String activeNotesStr = "- IDLE -";
    if (!activeKeyToNote.empty()) {
        activeNotesStr = "";
        const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        for (auto const& [key, note] : activeKeyToNote) {
            int oct = (note / 12) - 2; int semi = note % 12;
            if (semi < 0) { semi += 12; oct--; }
            activeNotesStr += juce::String(noteNames[semi]) + juce::String(oct) + " ";
        }
    }
    
    g.setColour(NeumorphicTheme::ledAmber());
    g.setFont(juce::FontOptions("Courier New", 18.0f, juce::Font::bold));
    g.drawText("OUTPUT  [ " + activeNotesStr + " ]", (int)lcdArea.getX(), (int)lcdArea.getY(), (int)lcdArea.getWidth(), (int)lcdArea.getHeight(), juce::Justification::centred);

    float meterWidth = 80.0f;
    juce::Rectangle<float> meterArea(lcdArea.getRight() - meterWidth - 15.0f, lcdArea.getY() + (lcdArea.getHeight() / 2.0f) - 4.0f, meterWidth, 8.0f);
    
    g.setColour(juce::Colour(0xff0a0a0c)); 
    g.fillRoundedRectangle(meterArea, 3.0f);
    g.setColour(NeumorphicTheme::recessLt().withAlpha(0.1f)); 
    g.drawRoundedRectangle(meterArea, 3.0f, 1.0f);
    
    if (midiActivityLevel > 0.0f) {
        float fillWidth = meterArea.getWidth() * midiActivityLevel;
        juce::Rectangle<float> fillArea = meterArea.withWidth(fillWidth);
        g.setColour(NeumorphicTheme::ledAmber().withAlpha(0.9f));
        g.fillRoundedRectangle(fillArea, 3.0f);
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.fillRoundedRectangle(fillArea.removeFromRight(2.0f), 3.0f);
    }

    g.setColour(NeumorphicTheme::recessDk()); 
    g.fillRoundedRectangle(octDownArea.translated(2, 2), 6.0f); g.fillRoundedRectangle(octUpArea.translated(2, 2), 6.0f);
    g.setColour(NeumorphicTheme::recessLt()); 
    g.fillRoundedRectangle(octDownArea.translated(-1, -1), 6.0f); g.fillRoundedRectangle(octUpArea.translated(-1, -1), 6.0f);
    g.setColour(NeumorphicTheme::bgBase()); 
    g.fillRoundedRectangle(octDownArea, 6.0f); g.fillRoundedRectangle(octUpArea, 6.0f);
    g.setColour(NeumorphicTheme::textMain()); 
    g.setFont(juce::FontOptions("Helvetica Neue", 20.0f, juce::Font::bold));
    g.drawText("-", (int)octDownArea.getX(), (int)octDownArea.getY(), (int)octDownArea.getWidth(), (int)octDownArea.getHeight(), juce::Justification::centred);
    g.drawText("+", (int)octUpArea.getX(), (int)octUpArea.getY(), (int)octUpArea.getWidth(), (int)octUpArea.getHeight(), juce::Justification::centred);
}

void VirtualMidiKeyAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds().reduced(15);
    auto bottomRow = bounds.removeFromBottom(120);
    octDownArea = bottomRow.removeFromLeft(40).reduced(0, 10).toFloat();
    octUpArea = bottomRow.removeFromRight(40).reduced(0, 10).toFloat();
    bottomRow.removeFromLeft(10); bottomRow.removeFromRight(10);
    keyboardComponent.setBounds(bottomRow);
    keyboardComponent.setLowestVisibleKey(currentOctaveOffset); 
    bounds.removeFromBottom(15);

    auto middleRow = bounds.removeFromBottom(45);
    pitchBend.setBounds(middleRow.removeFromLeft(45));
    middleRow.removeFromLeft(20);
    lcdArea = middleRow.toFloat();
    bounds.removeFromBottom(20);

    auto leftPanel = bounds.removeFromLeft((bounds.getWidth() / 2) - 10);
    scaleChordPanel.setBounds(leftPanel);
    bounds.removeFromLeft(20);
    faderPad.setBounds(bounds);
    
    themeToggle.setBounds(getWidth() - 170, 15, 150, 30);
}

int VirtualMidiKeyAudioProcessorEditor::quantizeNote(int semitoneOffset) {
    auto* scaleP = audioProcessor.parameters.getRawParameterValue("GLOBAL_SCALE");
    auto* rootP = audioProcessor.parameters.getRawParameterValue("GLOBAL_ROOT");
    auto* strictP = audioProcessor.parameters.getRawParameterValue("STRICT_QUANTIZE");
    
    int scale = scaleP ? (int)*scaleP : 0;
    if (scale == 0 || (strictP && *strictP < 0.5f)) return semitoneOffset; 

    int root = rootP ? (int)*rootP : 0;
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

void VirtualMidiKeyAudioProcessorEditor::triggerNoteOn(int midiNote, float velocity) {
    auto* toggleP = audioProcessor.parameters.getRawParameterValue("CHORD_MODE_ON");
    bool chordModeActive = toggleP ? (*toggleP > 0.5f) : false;
    
    // Updates UI State AND queues audio properly!
    audioProcessor.keyboardState.noteOn(1, midiNote, velocity);

    if (chordModeActive) {
        auto* chordP = audioProcessor.parameters.getRawParameterValue("GLOBAL_CHORD");
        int activePad = chordP ? (int)*chordP : 0;
        auto* typeP = audioProcessor.parameters.getRawParameterValue("PAD_" + juce::String(activePad) + "_TYPE");
        int mode = typeP ? (int)*typeP : 0;

        float chordVel = velocity * 0.85f; 

        if (mode == 1) { audioProcessor.keyboardState.noteOn(1, midiNote + 4, chordVel); audioProcessor.keyboardState.noteOn(1, midiNote + 7, chordVel); } 
        else if (mode == 2) { audioProcessor.keyboardState.noteOn(1, midiNote + 3, chordVel); audioProcessor.keyboardState.noteOn(1, midiNote + 7, chordVel); } 
        else if (mode == 3) { audioProcessor.keyboardState.noteOn(1, midiNote + 3, chordVel); audioProcessor.keyboardState.noteOn(1, midiNote + 6, chordVel); } 
        else if (mode == 4) { audioProcessor.keyboardState.noteOn(1, midiNote + 5, chordVel); audioProcessor.keyboardState.noteOn(1, midiNote + 7, chordVel); } 
        else if (mode == 5) { audioProcessor.keyboardState.noteOn(1, midiNote + 3, chordVel); audioProcessor.keyboardState.noteOn(1, midiNote + 7, chordVel); audioProcessor.keyboardState.noteOn(1, midiNote + 10, chordVel); } 
        else if (mode == 6) { audioProcessor.keyboardState.noteOn(1, midiNote + 4, chordVel); audioProcessor.keyboardState.noteOn(1, midiNote + 7, chordVel); audioProcessor.keyboardState.noteOn(1, midiNote + 11, chordVel); } 
        else if (mode == 7) { audioProcessor.keyboardState.noteOn(1, midiNote + 7, chordVel); audioProcessor.keyboardState.noteOn(1, midiNote + 12, chordVel); } 
    }
}

void VirtualMidiKeyAudioProcessorEditor::triggerNoteOff(int midiNote) {
    auto* toggleP = audioProcessor.parameters.getRawParameterValue("CHORD_MODE_ON");
    bool chordModeActive = toggleP ? (*toggleP > 0.5f) : false;

    audioProcessor.keyboardState.noteOff(1, midiNote, 0.0f);

    if (chordModeActive) {
        auto* chordP = audioProcessor.parameters.getRawParameterValue("GLOBAL_CHORD");
        int activePad = chordP ? (int)*chordP : 0;
        auto* typeP = audioProcessor.parameters.getRawParameterValue("PAD_" + juce::String(activePad) + "_TYPE");
        int mode = typeP ? (int)*typeP : 0;

        if (mode == 1) { audioProcessor.keyboardState.noteOff(1, midiNote + 4, 0.0f); audioProcessor.keyboardState.noteOff(1, midiNote + 7, 0.0f); }
        else if (mode == 2) { audioProcessor.keyboardState.noteOff(1, midiNote + 3, 0.0f); audioProcessor.keyboardState.noteOff(1, midiNote + 7, 0.0f); }
        else if (mode == 3) { audioProcessor.keyboardState.noteOff(1, midiNote + 3, 0.0f); audioProcessor.keyboardState.noteOff(1, midiNote + 6, 0.0f); }
        else if (mode == 4) { audioProcessor.keyboardState.noteOff(1, midiNote + 5, 0.0f); audioProcessor.keyboardState.noteOff(1, midiNote + 7, 0.0f); }
        else if (mode == 5) { audioProcessor.keyboardState.noteOff(1, midiNote + 3, 0.0f); audioProcessor.keyboardState.noteOff(1, midiNote + 7, 0.0f); audioProcessor.keyboardState.noteOff(1, midiNote + 10, 0.0f); }
        else if (mode == 6) { audioProcessor.keyboardState.noteOff(1, midiNote + 4, 0.0f); audioProcessor.keyboardState.noteOff(1, midiNote + 7, 0.0f); audioProcessor.keyboardState.noteOff(1, midiNote + 11, 0.0f); }
        else if (mode == 7) { audioProcessor.keyboardState.noteOff(1, midiNote + 7, 0.0f); audioProcessor.keyboardState.noteOff(1, midiNote + 12, 0.0f); }
    }
}

bool VirtualMidiKeyAudioProcessorEditor::keyPressed(const juce::KeyPress& key, juce::Component*) {
    int keyCode = key.getKeyCode();

    // 1. CHORD PADS: Number keys 1 through 6 acting as playable performance pads
    if (keyCode >= '1' && keyCode <= '6') {
        int padIndex = keyCode - '1'; // Converts '1'->0, '2'->1, etc.
        
        // Ensure we don't re-trigger if the key is already held down
        if (activeChordKeyToRoot.find(keyCode) == activeChordKeyToRoot.end()) {
            // Select the pad visually in the APVTS
            if (auto* p = audioProcessor.parameters.getParameter("GLOBAL_CHORD")) {
                p->setValueNotifyingHost(padIndex / 5.0f);
                repaint(); 
                scaleChordPanel.repaint(); 
            }

            // Get the root note assigned to this specific pad from parameters
            auto* rootP = audioProcessor.parameters.getRawParameterValue("PAD_" + juce::String(padIndex) + "_ROOT");
            int padRootSemitone = rootP ? (int)*rootP : 0;
            
            // Calculate final MIDI note based on current octave
            int midiNote = currentOctaveOffset + padRootSemitone;
            activeChordKeyToRoot[keyCode] = midiNote;

            // Trigger the note (which automatically handles chords if Chord Mode is ON!)
            float baseVelocity = 0.85f;
            triggerNoteOn(midiNote, baseVelocity);
        }
        return true;
    }

    // 2. OCTAVE SHIFTING: Arrow Keys
    if (keyCode == juce::KeyPress::leftKey) {
        if (currentOctaveOffset > 12) { 
            currentOctaveOffset -= 12; 
            keyboardComponent.setAvailableRange(currentOctaveOffset, 127); 
            repaint(); 
        } 
        return true; 
    }
    
    if (keyCode == juce::KeyPress::rightKey) {
        if (currentOctaveOffset < 96) { 
            currentOctaveOffset += 12; 
            keyboardComponent.setAvailableRange(currentOctaveOffset, 127); 
            repaint(); 
        } 
        return true;
    }

    return false; 
}

bool VirtualMidiKeyAudioProcessorEditor::keyStateChanged(bool, juce::Component*) {
    bool handled = false;
    
    // 1. Process Pitch Bend using Up/Down arrows
    bool upDown = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::upKey);
    bool downDown = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::downKey);
    if (upDown) pitchBend.setPitchBend(1.0f);
    else if (downDown) pitchBend.setPitchBend(0.0f);
    else pitchBend.setPitchBend(0.5f);

    // 2. Calculate BASE Spatial Velocity based on pointer position
    float baseVelocity = 0.8f; 
    auto mousePos = getMouseXYRelative();
    
    if (juce::KeyPress::isKeyCurrentlyDown(juce::ModifierKeys::shiftModifier)) {
        baseVelocity = 1.0f;
    } 
    else if (getLocalBounds().contains(mousePos)) {
        float normalizedY = juce::jlimit(0.0f, 1.0f, (float)mousePos.y / (float)getHeight());
        baseVelocity = juce::jmap(1.0f - normalizedY, 0.1f, 1.0f);
    }

    // 3. Map QWERTY Note Keys (z to m, q to u)
    for (const auto& [keyCode, rawSemitone] : qwertyToSemitone) {
        bool isDown = juce::KeyPress::isKeyCurrentlyDown(keyCode);
        auto it = activeKeyToNote.find(keyCode);
        
        if (isDown && it == activeKeyToNote.end()) {
            int midiNote = currentOctaveOffset + quantizeNote(rawSemitone);
            activeKeyToNote[keyCode] = midiNote; 
            
            float finalVelocity = getHumanizedVelocity(baseVelocity);
            triggerNoteOn(midiNote, finalVelocity); 
            handled = true;
            
        } else if (!isDown && it != activeKeyToNote.end()) {
            triggerNoteOff(it->second); 
            activeKeyToNote.erase(it); 
            handled = true;
        }
    }

    // 4. Map Chord Pad Number Keys (1 to 6) for Note Release
    for (int i = 0; i < 6; ++i) {
        int keyCode = '1' + i;
        bool isDown = juce::KeyPress::isKeyCurrentlyDown(keyCode);
        auto it = activeChordKeyToRoot.find(keyCode);

        if (!isDown && it != activeChordKeyToRoot.end()) {
            triggerNoteOff(it->second);
            activeChordKeyToRoot.erase(it);
            handled = true;
        }
    }
    
    return handled;
}

void VirtualMidiKeyAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    if (octDownArea.contains(e.position)) {
        if (currentOctaveOffset > 12) { currentOctaveOffset -= 12; keyboardComponent.setAvailableRange(currentOctaveOffset, 127); repaint(); }
    } else if (octUpArea.contains(e.position)) {
        if (currentOctaveOffset < 96) { currentOctaveOffset += 12; keyboardComponent.setAvailableRange(currentOctaveOffset, 127); repaint(); }
    }
}

float VirtualMidiKeyAudioProcessorEditor::getHumanizedVelocity(float baseVelocity) {
    auto& random = juce::Random::getSystemRandom();
    float centerWeightedRandom = (random.nextFloat() + random.nextFloat()) / 2.0f; 
    float variance = (centerWeightedRandom * 0.30f) - 0.15f; 
    return juce::jlimit(0.1f, 1.0f, baseVelocity + variance);
}