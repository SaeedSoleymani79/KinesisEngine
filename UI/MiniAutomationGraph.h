#pragma once
#include <JuceHeader.h>

class MiniAutomationGraph : public juce::Component {
public:
    MiniAutomationGraph() {
        history.resize(numPoints, 0.0f);
    }

    void pushValue(float newValue) {
        for (int i = 0; i < numPoints - 1; ++i) {
            history[i] = history[i + 1];
        }
        history[numPoints - 1] = newValue;
        repaint();
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // 1. LCD Background
        g.setColour(juce::Colour(0xff121315)); 
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Dark recess border (replaces NeumorphicTheme::recessLt())
        g.setColour(juce::Colour(0xff2a2b2e).withAlpha(0.1f)); 
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // 2. Build Path
        juce::Path path;
        float stepX = bounds.getWidth() / (numPoints - 1);
        
        for (int i = 0; i < numPoints; ++i) {
            float x = bounds.getX() + (i * stepX);
            float y = bounds.getBottom() - 2.0f - (history[i] * (bounds.getHeight() - 4.0f)); 
            if (i == 0) path.startNewSubPath(x, y);
            else path.lineTo(x, y);
        }

        // 3. Glowing Amber Line (replaces NeumorphicTheme::ledAmber())
        juce::Colour ledAmber(0xffffb000);
        g.setColour(ledAmber);
        g.strokePath(path, juce::PathStrokeType(1.5f));
        
        // 4. Glow Fill
        path.lineTo(bounds.getBottomRight().x, bounds.getBottom());
        path.lineTo(bounds.getBottomLeft().x, bounds.getBottom());
        path.closeSubPath();
        
        g.setColour(ledAmber.withAlpha(0.15f));
        g.fillPath(path);
    }

private:
    int numPoints = 60; 
    std::vector<float> history;
};