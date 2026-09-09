#pragma once
#include <JuceHeader.h>

// --- GLOBAL COLOR PALETTE ---
struct NeumorphicTheme {
    // The master switch
    inline static bool isLightMode = false; 

    static juce::Colour bgBase() { 
        return isLightMode ? juce::Colour(0xffe6e8eb) : juce::Colour(0xff1e1f22); 
    }
    static juce::Colour recessLt() { 
        return isLightMode ? juce::Colour(0xffffffff) : juce::Colour(0xff2a2b2e); 
    }
    static juce::Colour recessDk() { 
        return isLightMode ? juce::Colour(0xffd1d5db) : juce::Colour(0xff121315); 
    }
    static juce::Colour textMain() { 
        return isLightMode ? juce::Colour(0xff1e1f22) : juce::Colour(0xffdcdcdc); 
    }
    static juce::Colour ledAmber() { 
        // The amber LED looks good in both modes, but you can tweak it here!
        return juce::Colour(0xffffb000); 
    }
};

// --- CUSTOM COMPONENT STYLING ---
class HardwareLookAndFeel : public juce::LookAndFeel_V4 {
public:
    HardwareLookAndFeel() {
        setColour(juce::Slider::trackColourId, NeumorphicTheme::recessDk());
        setColour(juce::Slider::backgroundColourId, NeumorphicTheme::recessLt());
        setColour(juce::Slider::thumbColourId, NeumorphicTheme::textMain());
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override {
        
        auto trackRect = juce::Rectangle<float>((float)x + (float)width * 0.5f - 4.0f, (float)y, 8.0f, (float)height);
        
        // Fader Track
        g.setColour(NeumorphicTheme::recessDk());
        g.fillRoundedRectangle(trackRect, 4.0f);

        // Fader Cap (The physical knob)
        juce::Rectangle<float> thumbRect((float)x, sliderPos - 12.0f, (float)width, 24.0f);
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRoundedRectangle(thumbRect, 4.0f);
        
        // Grip line on the fader cap
        g.setColour(NeumorphicTheme::textMain().withAlpha(0.5f));
        g.fillRect(thumbRect.getX() + 5.0f, thumbRect.getCentreY() - 1.0f, thumbRect.getWidth() - 10.0f, 2.0f);
    }
};

// --- MAIN EDITOR LOOK AND FEEL ---
class NeumorphicLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void updateColors() {
        setColour(juce::ComboBox::backgroundColourId, NeumorphicTheme::bgBase());
        setColour(juce::ComboBox::textColourId, NeumorphicTheme::textMain());
        setColour(juce::ComboBox::arrowColourId, NeumorphicTheme::textMain());
        setColour(juce::ComboBox::outlineColourId, NeumorphicTheme::recessLt()); // Replaced missing outline()
        
        setColour(juce::PopupMenu::backgroundColourId, NeumorphicTheme::bgBase());
        setColour(juce::PopupMenu::textColourId, NeumorphicTheme::textMain());
        setColour(juce::PopupMenu::highlightedBackgroundColourId, NeumorphicTheme::recessDk());
        setColour(juce::PopupMenu::highlightedTextColourId, NeumorphicTheme::ledAmber());
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool isMouseOverButton, bool isButtonDown) override {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
        
        if (button.getToggleState() || isButtonDown) {
            g.setColour(NeumorphicTheme::bgBase().darker(0.04f));
            g.fillRoundedRectangle(bounds, bounds.getHeight() / 2.0f);
            g.setColour(NeumorphicTheme::recessDk());
            g.drawRoundedRectangle(bounds.translated(1, 1), bounds.getHeight() / 2.0f, 1.5f);
        } else {
            g.setColour(NeumorphicTheme::recessDk());
            g.fillRoundedRectangle(bounds.translated(2, 2), bounds.getHeight() / 2.0f);
            g.setColour(NeumorphicTheme::recessLt());
            g.fillRoundedRectangle(bounds.translated(-1, -1), bounds.getHeight() / 2.0f);
            g.setColour(NeumorphicTheme::bgBase());
            g.fillRoundedRectangle(bounds, bounds.getHeight() / 2.0f);
        }

        if (isMouseOverButton) {
            g.setColour(NeumorphicTheme::recessLt().withAlpha(0.2f));
            g.fillRoundedRectangle(bounds, bounds.getHeight() / 2.0f);
        }

        float indSize = bounds.getHeight() - 8.0f;
        juce::Rectangle<float> indicator(bounds.getX() + 6.0f, bounds.getY() + 4.0f, indSize, indSize);
        g.setColour(button.getToggleState() ? NeumorphicTheme::ledAmber() : NeumorphicTheme::recessDk());
        g.fillEllipse(indicator);

        g.setColour((isMouseOverButton || button.getToggleState()) ? NeumorphicTheme::ledAmber() : NeumorphicTheme::textMain());
        g.setFont(juce::FontOptions("Helvetica Neue", 12.0f, juce::Font::bold));
        auto textArea = bounds.withTrimmedLeft(indSize + 14.0f);
        g.drawText(button.getButtonText(), textArea, juce::Justification::centredLeft, false);
    }
};