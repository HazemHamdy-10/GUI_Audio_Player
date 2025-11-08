
#include "PlayerGUI.h"

PlayerGUI::PlayerGUI()
{
    // Set up buttons
    for (auto* btn : { &loadButton, &playButton, &stopButton, &setAButton, &setBButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    loopButton.addListener(this);
    addAndMakeVisible(&loopButton);

    // Volume slider
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.7);
    volumeSlider.addListener(this);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible(&volumeSlider);
    addAndMakeVisible(&volumeLabel);

    volumeLabel.setText("Volume", juce::dontSendNotification);                // Set label text

    // Speed slider
    speedSlider.setRange(0.25, 4.0, 0.05);
    speedSlider.setValue(1.0);
    speedSlider.addListener(this);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible(&speedSlider);
    addAndMakeVisible(&speedLabel);

    speedSlider.setTextValueSuffix("x");

    speedLabel.setText("Speed", juce::dontSendNotification);                // Set label text
    

    // Position slider
    positionSlider.setRange(0.0, 1.0, 0.001);
    positionSlider.addListener(this);
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(&positionSlider);

    // Time label
    addAndMakeVisible(&timeLabel);
    timeLabel.setJustificationType(juce::Justification::centred);

    // Waveform display
    addAndMakeVisible(&waveformDisplay);

    // Start timer for UI updates
    startTimer(50);

    // Set colors for professional look
    getLookAndFeel().setColour(juce::Slider::thumbColourId, juce::Colours::lightblue);
    getLookAndFeel().setColour(juce::Slider::trackColourId, juce::Colour(0xff404040));
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
    getLookAndFeel().setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

PlayerGUI::~PlayerGUI()
{
}

void PlayerGUI::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    playerAudio.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerGUI::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    playerAudio.getNextAudioBlock(bufferToFill);
}

void PlayerGUI::releaseResources()
{
    playerAudio.releaseResources();
}

void PlayerGUI::paint(juce::Graphics& g)
{
    // Professional dark theme
    g.fillAll(juce::Colour(0xff2b2b2b));

    // Draw header
    auto area = getLocalBounds();
    auto header = area.removeFromTop(20);
    g.setColour(juce::Colour(0xff404040));
    g.fillRect(header);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("Audio Player", header, juce::Justification::centred, true);
}

void PlayerGUI::resized()
{
    auto area = getLocalBounds();
    int width = getWidth();
    int height = getHeight();

    // Load button and time
    loadButton.setBounds(20, 40, 100, 30);

    timeLabel.setBounds((width - 200) / 2, 40, 200, 30);   // Centered

    // Waveform
    waveformDisplay.setBounds(10, 75, width - 20, 80);

    // Position slider
    positionSlider.setBounds(10, 160, width - 20, 20);

    // Buttons
    int buttonWidth = (width - 40) / 5;
    int buttonY = 185;

    

    playButton.setBounds(10, buttonY, buttonWidth, 35);
    stopButton.setBounds(10 + buttonWidth + 5, buttonY, buttonWidth, 35);
    setAButton.setBounds(10 + 2 * (buttonWidth + 5), buttonY, buttonWidth, 35);
    setBButton.setBounds(10 + 3 * (buttonWidth + 5), buttonY, buttonWidth, 35);
    loopButton.setBounds(10 + 4 * (buttonWidth + 5), buttonY, buttonWidth, 35);

    // Sliders
    int sliderY = buttonY + 45;
    volumeLabel.setBounds(20, sliderY, 80, 25);
    volumeSlider.setBounds(110, sliderY, width - 130, 25);

    speedLabel.setBounds(20, sliderY + 35, 80, 25);
    speedSlider.setBounds(110, sliderY + 35, width - 130, 25);
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
            juce::File{},
            "*.wav;*.mp3;*.aiff");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    playerAudio.loadFile(file);
                    positionSlider.setRange(0.0, playerAudio.getLength(), 0.1);
                }
            });
    }
    else if (button == &playButton)
    {
        playerAudio.start();
    }
    else if (button == &stopButton)
    {
        playerAudio.stop();
    }
    else if (button == &setAButton)
    {
        playerAudio.setLoopStart(playerAudio.getPosition());
        waveformDisplay.repaint();
        repaint();
    }
    else if (button == &setBButton)
    {
        playerAudio.setLoopEnd(playerAudio.getPosition());
        waveformDisplay.repaint();
        repaint();
    }
    else if (button == &loopButton)
    {
        playerAudio.setLooping(loopButton.getToggleState());
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        playerAudio.setGain((float)slider->getValue());
    }
    else if (slider == &speedSlider)
    {
        playerAudio.setSpeed((float)slider->getValue());
    }
    else if (slider == &positionSlider)
    {
        playerAudio.setPosition(slider->getValue());
    }
}

void PlayerGUI::timerCallback()
{
    updateTimeDisplay();

    // Update position slider without triggering change
    if (!positionSlider.isMouseButtonDown())
    {
        positionSlider.setValue(playerAudio.getPosition(), juce::dontSendNotification);
    }

    // Repaint waveform to update position pointer
        waveformDisplay.repaint();
}

void PlayerGUI::updateTimeDisplay()
{
    double currentTime = playerAudio.getPosition();
    double totalTime = playerAudio.getLength();

    juce::String text = formatTime(currentTime) + " / " + formatTime(totalTime);

    // Add loop markers if looping is enabled
    if (playerAudio.isLooping())
    {
        text += " [A:" + formatTime(playerAudio.getLoopStart()) +
            " B:" + formatTime(playerAudio.getLoopEnd()) + "]";
    }

    timeLabel.setText(text, juce::dontSendNotification);
}

juce::String PlayerGUI::formatTime(double seconds)
{
    if (seconds < 0) return "00:00";

    int minutes = (int)(seconds / 60);
    int secs = (int)seconds % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}

// WaveformDisplay implementation

void PlayerGUI::WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Draw background
    g.setColour(juce::Colour(0xff2d2d2d));
    g.fillRect(bounds);

    // Draw border
    g.setColour(juce::Colours::grey);
    g.drawRect(bounds, 1.0f);

    // Draw waveform if available
    if (playerAudio.hasWaveform())
    {
        auto waveformData = playerAudio.getWaveformData();
        int numPoints = playerAudio.getWaveformSize();

        // Draw the waveform
        juce::Path waveformPath;
        bool pathStarted = false;

        for (int i = 0; i < numPoints; ++i)
        {
            float x = bounds.getX() + (float)i / numPoints * bounds.getWidth();
            float height = waveformData[i] * bounds.getHeight();
            float y = bounds.getCentreY() - height / 2.0f;

            if (!pathStarted)
            {
                waveformPath.startNewSubPath(x, y);
                pathStarted = true;
            }
            else
            {
                waveformPath.lineTo(x, y);
            }
        }

        // Draw the bottom half (mirrored)
        for (int i = numPoints - 1; i >= 0; --i)
        {
            float x = bounds.getX() + (float)i / numPoints * bounds.getWidth();
            float height = waveformData[i] * bounds.getHeight();
            float y = bounds.getCentreY() + height / 2.0f;
            waveformPath.lineTo(x, y);
        }

        waveformPath.closeSubPath();

        // Fill the waveform
        g.setColour(juce::Colour(0xff4a90e2).withAlpha(0.6f));
        g.fillPath(waveformPath);

        // Draw waveform outline
        g.setColour(juce::Colour(0xff4a90e2));
        g.strokePath(waveformPath, juce::PathStrokeType(1.0f));
    }
    else
    {
        // Draw placeholder when no audio is loaded
        g.setColour(juce::Colours::grey);
        g.setFont(17.0f);
        g.drawText("Load an audio file to play", bounds,
            juce::Justification::centred, true);
    }

    // Draw playback position pointer
    if (playerAudio.getLength() > 0)
    {
        double progress = playerAudio.getPosition() / playerAudio.getLength();
        float xPos = bounds.getX() + progress * bounds.getWidth();

        // Draw position line
        g.setColour(juce::Colours::red);
        g.drawLine(xPos, bounds.getY(), xPos, bounds.getBottom(), 2.0f);

        // Draw position pointer (triangle at top)
        juce::Path pointer;
        pointer.addTriangle(xPos - 6, bounds.getY() + 2,
            xPos + 6, bounds.getY() + 2,
            xPos, bounds.getY() + 10);
        g.setColour(juce::Colours::red);
        g.fillPath(pointer);

        // Draw time at current position
        double currentTime = playerAudio.getPosition();
        juce::String timeText = formatTime(currentTime);
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText(timeText, xPos - 25, bounds.getY() + 12, 50, 15,
            juce::Justification::centred, true);
    }

    // Draw loop markers if looping is enabled
    if (playerAudio.getLength() > 0)
    {
        double loopStartProgress = playerAudio.getLoopStart() / playerAudio.getLength();
        double loopEndProgress = playerAudio.getLoopEnd() / playerAudio.getLength();

        float startX = bounds.getX() + loopStartProgress * bounds.getWidth();
        float endX = bounds.getX() + loopEndProgress * bounds.getWidth();

        // Draw loop start marker
        g.setColour(juce::Colours::yellow);
        juce::Path startMarker;
        startMarker.addTriangle(startX - 5, bounds.getBottom() - 2,
            startX + 5, bounds.getBottom() - 2,
            startX, bounds.getBottom() - 10);
        g.fillPath(startMarker);
        g.drawLine(startX, bounds.getY(), startX, bounds.getBottom(), 2.0f);

        // Draw loop end marker
        juce::Path endMarker;
        endMarker.addTriangle(endX - 5, bounds.getBottom() - 2,
            endX + 5, bounds.getBottom() - 2,
            endX, bounds.getBottom() - 10);
        g.fillPath(endMarker);
        g.drawLine(endX, bounds.getY(), endX, bounds.getBottom(), 2.0f);

        // Draw loop region background
        if (playerAudio.isLooping() && playerAudio.getLength() > 0) {
            g.setColour(juce::Colours::green.withAlpha(0.3f));
            g.fillRect(startX, bounds.getY(), endX - startX, bounds.getHeight());
        }
    }
}


// format time (add this to PlayerGUI class)
juce::String PlayerGUI::WaveformDisplay::formatTime(double seconds)
{
    if (seconds < 0) return "00:00";

    int minutes = (int)(seconds / 60);
    int secs = (int)seconds % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}

//------------------------------------------------------------------------

void PlayerGUI::WaveformDisplay::mouseDown(const juce::MouseEvent& event)
{
    if (playerAudio.getLength() > 0)
    {
        auto bounds = getLocalBounds().toFloat();
        float clickX = event.position.x;
        double newPosition = (clickX / bounds.getWidth()) * playerAudio.getLength();
        playerAudio.setPosition(newPosition);
        repaint();
    }
}

void PlayerGUI::WaveformDisplay::mouseDrag(const juce::MouseEvent& event)
{
    if (playerAudio.getLength() > 0)
    {
        auto bounds = getLocalBounds().toFloat();
        float dragX = event.position.x;
        double newPosition = (dragX / bounds.getWidth()) * playerAudio.getLength();
        playerAudio.setPosition(newPosition);
        repaint();
    }
}
//------------------------------------------------------------------------