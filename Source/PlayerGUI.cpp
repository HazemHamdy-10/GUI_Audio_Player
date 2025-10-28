#include "PlayerGUI.h"

void PlayerGUI::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    playerAudio.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerGUI::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    playerAudio.getNextAudioBlock(bufferToFill);

    if (loopEnabled)
    {
        double pos = playerAudio.getPosition();
        double length = playerAudio.getLength();

        if (length > 0 && pos >= length - 0.05)
        {
            playerAudio.setPosition(0.0);
            playerAudio.play();
        }
    }
}

void PlayerGUI::releaseResources()
{
    playerAudio.releaseResources();
}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.setGradientFill(juce::ColourGradient::vertical(
        juce::Colours::darkslategrey,
        juce::Colours::black,
        getLocalBounds().toFloat()
    ));

    g.fillAll();
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 26.0f, juce::Font::bold));
    auto titleArea = juce::Rectangle<int>(0, 10, getWidth(), 40);
    g.drawText("Simple Audio Player", titleArea, juce::Justification::centred, true);

}

PlayerGUI::PlayerGUI()
{
    for (auto* btn : { &loadButton, &restartButton, &stopButton, &playPauseButton, &startButton, &endButton, &muteButton, &loopButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    playPauseButton.setButtonText("Play");
    startButton.setButtonText("|<  Start");
    endButton.setButtonText("End  >|");
    loadButton.setButtonText("Load");
    restartButton.setButtonText("Restart");
    stopButton.setButtonText("Stop");

    playPauseButton.setColour(TextButton::buttonColourId, Colours::green);
    stopButton.setColour(TextButton::buttonColourId, Colours::red);
    restartButton.setColour(TextButton::buttonColourId, Colours::orange);
    loadButton.setColour(TextButton::buttonColourId, Colours::cornflowerblue);
    startButton.setColour(TextButton::buttonColourId, Colours::darkcyan);
    endButton.setColour(TextButton::buttonColourId, Colours::darkcyan);

    volumeSlider.setRange(0.0, 1.0, 0.01);

    volumeSlider.setValue(0.5);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);
}

void PlayerGUI::resized()
{
    int y = 50;

    loadButton.setBounds(20, 60, 100, 40);
    restartButton.setBounds(130, 60, 80, 40);
    stopButton.setBounds(220, 60, 80, 40);
    playPauseButton.setBounds(310, 60, 80, 40);
    startButton.setBounds(400, 60, 100, 40);
    endButton.setBounds(510, 60, 100, 40);
    muteButton.setBounds(620, 60, 80, 40);
    loopButton.setBounds(710, 60, 100, 40);

    volumeSlider.setBounds(20, 130, getWidth() - 40, 30);
}

PlayerGUI::~PlayerGUI() {}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...", juce::File{}, "*.wav;*.mp3");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    playerAudio.loadFile(file);
                    playerAudio.play();
                    isPlaying = true;
                    playPauseButton.setButtonText("Pause");
                }
            });
    }

    if (button == &restartButton)
    {
        playerAudio.setPosition(0.0);
        playerAudio.play();
        isPlaying = true;
        playPauseButton.setButtonText("Pause");
    }

    if (button == &stopButton)
    {
        playerAudio.stop();
        isPlaying = false;
        playPauseButton.setButtonText("Play");
    }

    if (button == &playPauseButton)
    {
        if (isPlaying)
        {
            playerAudio.stop();
            playPauseButton.setButtonText("Play");
        }
        else
        {
            playerAudio.play();
            playPauseButton.setButtonText("Pause");
        }
        isPlaying = !isPlaying;
    }

    if (button == &startButton)
        playerAudio.setPosition(0.0);

    if (button == &endButton)
        playerAudio.setPosition(playerAudio.getLength());

    if (button == &loopButton)
    {
        loopEnabled = !loopEnabled;
        loopButton.setColour(TextButton::buttonColourId,
            loopEnabled ? Colours::green : Colours::lightgrey);
    }


    if (button == &muteButton)
    {
        if (isMuted)
        {
            playerAudio.setGain(previousVolume);
            muteButton.setButtonText("Mute");
        }
        else
        {
            previousVolume = (float)volumeSlider.getValue();
            playerAudio.setGain(0.0f);
            muteButton.setButtonText("Unmute");
        }
        isMuted = !isMuted;
    }


    if (button == &loopButton)
    {
        isLooping = !isLooping;
        loopButton.setButtonText(isLooping ? "Loop On" : "Loop Off");
        playerAudio.setLooping(isLooping);
    }

    if (button == &endButton)
    {
        double length = playerAudio.getLength();
        playerAudio.setPosition(length);

        if (loopEnabled)
        {
            playerAudio.setPosition(0.0);
            playerAudio.play();
        }
    }

}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
        playerAudio.setGain((float)slider->getValue());
}

