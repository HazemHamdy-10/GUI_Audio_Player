#pragma once
#include <JuceHeader.h>
#include "PlayerGUI.h"

class MainComponent : public juce::AudioAppComponent,
    public juce::Slider::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    // Two players for mixing
    PlayerGUI player1;
    PlayerGUI player2;

    // Mixer controls
    juce::Slider mixerSlider1;
    juce::Slider mixerSlider2;
    juce::Slider crossfadeSlider;
    juce::Label mixerLabel;
    juce::Label player1Label;
    juce::Label player2Label;
    juce::Label crossfadeLabel;
    juce::TextButton linkButton{ "Link" };

    bool useDualPlayer = true;  // Set to false for single player mode
    bool linked = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
