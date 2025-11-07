#include "MainComponent.h"

MainComponent::MainComponent()
{
    // Add players
    addAndMakeVisible(player1);

    if (useDualPlayer)
    {
        addAndMakeVisible(player2);

        // Mixer sliders
        mixerSlider1.setRange(0.0, 1.0, 0.01);
        mixerSlider1.setValue(0.7);
        mixerSlider1.setSliderStyle(Slider::LinearVertical);
        mixerSlider1.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
        mixerSlider1.addListener(this);
        addAndMakeVisible(mixerSlider1);

        mixerSlider2.setRange(0.0, 1.0, 0.01);
        mixerSlider2.setValue(0.7);
        mixerSlider2.setSliderStyle(Slider::LinearVertical);
        mixerSlider2.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
        mixerSlider2.addListener(this);
        addAndMakeVisible(mixerSlider2);

        // Crossfade slider
        crossfadeSlider.setRange(0.0, 1.0, 0.01);
        crossfadeSlider.setValue(0.5);
        crossfadeSlider.setSliderStyle(Slider::LinearHorizontal);
        crossfadeSlider.setTextBoxStyle(Slider::TextBoxRight, false, 50, 20);
        crossfadeSlider.addListener(this);
        addAndMakeVisible(crossfadeSlider);

        // Labels
        mixerLabel.setText("MIXER", dontSendNotification);
        mixerLabel.setFont(Font(20.0f, Font::bold));
        mixerLabel.setColour(Label::textColourId, Colour(0xff00d4ff));
        mixerLabel.setJustificationType(Justification::centred);
        addAndMakeVisible(mixerLabel);

        player1Label.setText("Player 1", dontSendNotification);
        player1Label.setColour(Label::textColourId, Colours::white);
        player1Label.setJustificationType(Justification::centred);
        addAndMakeVisible(player1Label);

        player2Label.setText("Player 2", dontSendNotification);
        player2Label.setColour(Label::textColourId, Colours::white);
        player2Label.setJustificationType(Justification::centred);
        addAndMakeVisible(player2Label);

        crossfadeLabel.setText("Crossfade: Player 1 ← → Player 2", dontSendNotification);
        crossfadeLabel.setColour(Label::textColourId, Colours::white);
        addAndMakeVisible(crossfadeLabel);

        // Link button
        linkButton.onClick = [this]() {
            linked = !linked;
            linkButton.setButtonText(linked ? "Linked" : "Link");
            linkButton.setColour(TextButton::buttonColourId,
                linked ? Colour(0xff00ff88) : Colour(0xff786fa6));
            };
        linkButton.setColour(TextButton::buttonColourId, Colour(0xff786fa6));
        addAndMakeVisible(linkButton);

        setSize(900, 1200);
    }
    else
    {
        setSize(750, 600);
    }

    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.setGradientFill(ColourGradient(Colour(0xff0a0a0a), 0, 0,
        Colour(0xff1a1a1a), 0, (float)getHeight(), false));
    g.fillAll();

    if (useDualPlayer)
    {
        // Mixer panel background
        int mixerX = getWidth() / 2 - 150;
        int mixerY = getHeight() / 2 - 95;
        g.setColour(Colour(0xff16213e).withAlpha(0.8f));
        g.fillRoundedRectangle((float)mixerX, (float)mixerY, 300, 180, 10);

        // Divider line
        g.setColour(Colour(0xff00d4ff).withAlpha(0.3f));
        g.drawLine(10, (float)(getHeight() / 2), (float)(getWidth() - 10), (float)(getHeight() / 2), 2);
    }
}

void MainComponent::resized()
{
    if (useDualPlayer)
    {
        int halfHeight = getHeight() / 2;

        // Player 1 on top
        player1.setBounds(10, 10, getWidth() - 20, halfHeight - 15);

        // Player 2 on bottom
        player2.setBounds(10, halfHeight + 15, getWidth() - 20, halfHeight - 25);

        // Mixer controls in center
        int mixerX = getWidth() / 2 - 150;
        int mixerY = halfHeight - 85;

        mixerLabel.setBounds(mixerX, mixerY - 30, 300, 30);

        player1Label.setBounds(mixerX + 30, mixerY, 80, 20);
        mixerSlider1.setBounds(mixerX + 40, mixerY + 25, 60, 100);

        player2Label.setBounds(mixerX + 190, mixerY, 80, 20);
        mixerSlider2.setBounds(mixerX + 200, mixerY + 25, 60, 100);

        crossfadeLabel.setBounds(mixerX, mixerY + 135, 240, 20);
        crossfadeSlider.setBounds(mixerX + 10, mixerY + 160, 210, 25);

        linkButton.setBounds(mixerX + 230, mixerY + 160, 60, 25);
    }
    else
    {
        // Single player mode
        player1.setBounds(getLocalBounds().reduced(10));
    }
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);

    if (useDualPlayer)
    {
        player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (useDualPlayer)
    {
        // Clear buffer first
        bufferToFill.clearActiveBufferRegion();

        // Create temp buffers for each player
        juce::AudioBuffer<float> tempBuffer1(bufferToFill.buffer->getNumChannels(),
            bufferToFill.numSamples);
        juce::AudioBuffer<float> tempBuffer2(bufferToFill.buffer->getNumChannels(),
            bufferToFill.numSamples);

        juce::AudioSourceChannelInfo tempInfo1(&tempBuffer1, 0, bufferToFill.numSamples);
        juce::AudioSourceChannelInfo tempInfo2(&tempBuffer2, 0, bufferToFill.numSamples);

        // Get audio from both players
        player1.getNextAudioBlock(tempInfo1);
        player2.getNextAudioBlock(tempInfo2);

        // Mix with crossfade
        float crossfade = (float)crossfadeSlider.getValue();
        float gain1 = (1.0f - crossfade) * (float)mixerSlider1.getValue();
        float gain2 = crossfade * (float)mixerSlider2.getValue();

        // Apply gains and mix
        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            auto* outputData = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
            auto* input1Data = tempBuffer1.getReadPointer(channel);
            auto* input2Data = tempBuffer2.getReadPointer(channel);

            for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                outputData[sample] = (input1Data[sample] * gain1) + (input2Data[sample] * gain2);
            }
        }
    }
    else
    {
        // Single player mode
        player1.getNextAudioBlock(bufferToFill);
    }
}

void MainComponent::releaseResources()
{
    player1.releaseResources();

    if (useDualPlayer)
    {
        player2.releaseResources();
    }
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (!useDualPlayer) return;

    if (slider == &mixerSlider1)
    {
        float gain = (float)slider->getValue();
        player1.setGain(gain * (1.0f - (float)crossfadeSlider.getValue()));
    }

    if (slider == &mixerSlider2)
    {
        float gain = (float)slider->getValue();
        player2.setGain(gain * (float)crossfadeSlider.getValue());
    }

    if (slider == &crossfadeSlider)
    {
        if (linked)
        {
            // Linked mode: inverse relationship
            float value = (float)slider->getValue();
            mixerSlider1.setValue(1.0 - value, dontSendNotification);
            mixerSlider2.setValue(value, dontSendNotification);
        }

        // Update gains
        player1.setGain((float)mixerSlider1.getValue() * (1.0f - (float)slider->getValue()));
        player2.setGain((float)mixerSlider2.getValue() * (float)slider->getValue());
    }
}
