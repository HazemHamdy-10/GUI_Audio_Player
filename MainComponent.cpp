#include "MainComponent.h"

MainComponent::MainComponent()
{
    addAndMakeVisible(player1);

    // Increased size to accommodate new UI elements
    setSize(700, 400);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    player1.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    player1.releaseResources();
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // Split vertically for two players
    player1.setBounds(area.removeFromTop(300).reduced(5));
}