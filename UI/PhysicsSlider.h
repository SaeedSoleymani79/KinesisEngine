#pragma once
#include <JuceHeader.h>

class PhysicsSlider : public juce::Slider, private juce::Timer {
public:
    PhysicsSlider() {
        setSliderStyle(juce::Slider::LinearVertical);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setRange(0.0, 1.0);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        juce::Slider::mouseDown(e);
        stopTimer(); 
        lastTime = juce::Time::getMillisecondCounterHiRes();
        lastNormVal = getValue();
        velocity = 0.0;
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        juce::Slider::mouseDrag(e);
        double now = juce::Time::getMillisecondCounterHiRes();
        double deltaTime = now - lastTime;
        if (deltaTime > 0) {
            double currentNormVal = getValue();
            velocity = currentNormVal - lastNormVal; 
        }
        lastTime = now;
        lastNormVal = getValue();
    }

    void mouseUp(const juce::MouseEvent& e) override {
        juce::Slider::mouseUp(e);
        if (std::abs(velocity) > 0.001) { 
            startTimerHz(60); 
        }
    }

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override {
        velocity -= wheel.deltaY * 0.30; 
        if (!isTimerRunning() && std::abs(velocity) > 0.001) {
            startTimerHz(60);
        }
    }

    void timerCallback() override {
        double newVal = getValue() + velocity;
        velocity *= 0.80; // Heavy friction
        
        if (std::abs(velocity) < 0.0001 || newVal <= getMinimum() || newVal >= getMaximum()) {
            stopTimer();
        }
        setValue(juce::jlimit(getMinimum(), getMaximum(), newVal), juce::sendNotificationSync);
    }

private:
    double lastTime{0};
    double lastNormVal{0};
    double velocity{0};
};