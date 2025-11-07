#include "PlayerGUI.h"

// ============ WaveformDisplay Implementation ============
WaveformDisplay::WaveformDisplay(PlayerAudio& audio)
    : playerAudio(audio), thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
{
    formatManager.registerBasicFormats();
    startTimer(40); // 25 FPS update
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background gradient
    g.setGradientFill(ColourGradient(Colour(0xff1a1a2e), 0, 0,
        Colour(0xff16213e), 0, (float)getHeight(), false));
    g.fillAll();

    // Border
    g.setColour(Colours::lightgrey.withAlpha(0.3f));
    g.drawRect(bounds, 2);

    // Waveform
    if (thumbnail.getTotalLength() > 0.0)
    {
        // Draw waveform
        g.setColour(Colour(0xff0f3460));
        thumbnail.drawChannels(g, bounds.reduced(4), 0.0, thumbnail.getTotalLength(), 1.0f);

        // Progress overlay (played portion)
        double progress = currentPosition / thumbnail.getTotalLength();
        int progressX = (int)(progress * bounds.getWidth());
        g.setColour(Colour(0xff00d4ff).withAlpha(0.3f));
        g.fillRect(4, 4, progressX - 4, bounds.getHeight() - 8);

        // A-B Loop markers
        if (hasABLoop && loopPointA >= 0 && loopPointB > loopPointA)
        {
            double totalLength = thumbnail.getTotalLength();
            int xA = (int)((loopPointA / totalLength) * bounds.getWidth());
            int xB = (int)((loopPointB / totalLength) * bounds.getWidth());

            // Loop region highlight
            g.setColour(Colours::orange.withAlpha(0.3f));
            g.fillRect(xA, 0, xB - xA, bounds.getHeight());

            // A and B markers
            g.setColour(Colours::orange);
            g.drawLine((float)xA, 0, (float)xA, (float)bounds.getHeight(), 2.0f);
            g.drawLine((float)xB, 0, (float)xB, (float)bounds.getHeight(), 2.0f);

            g.setFont(Font(14.0f, Font::bold));
            g.drawText("A", xA + 5, 5, 20, 20, Justification::left);
            g.drawText("B", xB - 25, 5, 20, 20, Justification::left);
        }

        // Markers
        double totalLength = thumbnail.getTotalLength();
        for (const auto& marker : markers)
        {
            int x = (int)((marker.timePosition / totalLength) * bounds.getWidth());

            // Marker dot
            g.setColour(marker.colour);
            g.fillEllipse((float)(x - 4), (float)(bounds.getHeight() / 2 - 4), 8, 8);

            // Marker line
            g.setColour(marker.colour.withAlpha(0.6f));
            g.drawLine((float)x, 0, (float)x, (float)bounds.getHeight(), 1.5f);
        }

        // Current position line (red)
        g.setColour(Colour(0xffff6b6b));
        g.drawLine((float)progressX, 0, (float)progressX, (float)bounds.getHeight(), 3.0f);
    }
    else
    {
        // No audio loaded message
        g.setColour(Colours::grey);
        g.setFont(16.0f);
        g.drawText("Load an audio file to see waveform", bounds, Justification::centred);
    }
}

void WaveformDisplay::setWaveform(const juce::File& file)
{
    thumbnail.clear();
    if (file.existsAsFile())
    {
        thumbnail.setSource(new juce::FileInputSource(file));
    }
    repaint();
}

void WaveformDisplay::setPosition(double pos)
{
    if (currentPosition != pos)
    {
        currentPosition = pos;
    }
}

void WaveformDisplay::mouseDown(const juce::MouseEvent& event)
{
    if (thumbnail.getTotalLength() > 0.0)
    {
        double clickedTime = getClickedTime(event.x);
        playerAudio.setPosition(clickedTime);
    }
}

double WaveformDisplay::getClickedTime(int x) const
{
    double ratio = (double)x / getWidth();
    return ratio * thumbnail.getTotalLength();
}

void WaveformDisplay::timerCallback()
{
    repaint();
}

void WaveformDisplay::addMarker(double time, const juce::String& name)
{
    markers.push_back(AudioMarker(time, name, Colours::yellow));
    repaint();
}

void WaveformDisplay::clearMarkers()
{
    markers.clear();
    repaint();
}

void WaveformDisplay::setABLoopPoints(double pointA, double pointB)
{
    loopPointA = pointA;
    loopPointB = pointB;
    hasABLoop = true;
    repaint();
}

void WaveformDisplay::clearABLoop()
{
    hasABLoop = false;
    loopPointA = -1.0;
    loopPointB = -1.0;
    repaint();
}

// ============ PlayerGUI Implementation ============
PlayerGUI::PlayerGUI() : waveformDisplay(playerAudio)
{
    // Setup all buttons
    for (auto* btn : { &loadButton, &playPauseButton, &stopButton, &prevTrackButton,
                       &nextTrackButton, &backward10Button, &forward10Button,
                       &startButton, &endButton, &muteButton, &loopButton,
                       &setPointAButton, &setPointBButton, &clearABButton, &addMarkerButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    // Modern button colors
    playPauseButton.setColour(TextButton::buttonColourId, Colour(0xff00d4ff));
    stopButton.setColour(TextButton::buttonColourId, Colour(0xffff6b6b));
    loadButton.setColour(TextButton::buttonColourId, Colour(0xff4ecdc4));
    prevTrackButton.setColour(TextButton::buttonColourId, Colour(0xff95e1d3));
    nextTrackButton.setColour(TextButton::buttonColourId, Colour(0xff95e1d3));
    backward10Button.setColour(TextButton::buttonColourId, Colour(0xff38ada9));
    forward10Button.setColour(TextButton::buttonColourId, Colour(0xff38ada9));
    setPointAButton.setColour(TextButton::buttonColourId, Colour(0xffffa502));
    setPointBButton.setColour(TextButton::buttonColourId, Colour(0xffffa502));
    clearABButton.setColour(TextButton::buttonColourId, Colour(0xff786fa6));
    addMarkerButton.setColour(TextButton::buttonColourId, Colour(0xfff8b500));
    muteButton.setColour(TextButton::buttonColourId, Colour(0xff6c5ce7));
    loopButton.setColour(TextButton::buttonColourId, Colour(0xff786fa6));

    // Volume slider
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.7);
    volumeSlider.setSliderStyle(Slider::LinearVertical);
    volumeSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    // Speed slider
    speedSlider.setRange(0.5, 2.0, 0.1);
    speedSlider.setValue(1.0);
    speedSlider.setSliderStyle(Slider::Rotary);
    speedSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
    speedSlider.addListener(this);
    addAndMakeVisible(speedSlider);

    // Labels setup
    volumeLabel.setText("Volume", dontSendNotification);
    volumeLabel.setJustificationType(Justification::centred);
    volumeLabel.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(volumeLabel);

    speedLabel.setText("Speed", dontSendNotification);
    speedLabel.setJustificationType(Justification::centred);
    speedLabel.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(speedLabel);

    fileNameLabel.setJustificationType(Justification::centred);
    fileNameLabel.setColour(Label::textColourId, Colour(0xff00d4ff));
    fileNameLabel.setFont(Font(18.0f, Font::bold));
    addAndMakeVisible(fileNameLabel);

    timeLabel.setText("00:00 / 00:00", dontSendNotification);
    timeLabel.setJustificationType(Justification::centred);
    timeLabel.setColour(Label::textColourId, Colours::white);
    timeLabel.setFont(Font(16.0f));
    addAndMakeVisible(timeLabel);

    markerListLabel.setText("Markers", dontSendNotification);
    markerListLabel.setColour(Label::textColourId, Colours::white);
    markerListLabel.setFont(Font(14.0f, Font::bold));
    addAndMakeVisible(markerListLabel);

    // Waveform display
    addAndMakeVisible(waveformDisplay);

    // Marker list
    markerListModel = std::make_unique<MarkerListModel>(*this);
    markerListBox.setModel(markerListModel.get());
    markerListBox.setColour(ListBox::backgroundColourId, Colour(0xff1a1a2e));
    markerListBox.setColour(ListBox::outlineColourId, Colours::lightgrey.withAlpha(0.3f));
    addAndMakeVisible(markerListBox);

    // Load last session
    loadSession();

    startTimer(100); // Update every 100ms
}

PlayerGUI::~PlayerGUI()
{
    saveSession();
    stopTimer();
}

void PlayerGUI::paint(juce::Graphics& g)
{
    // Modern gradient background
    g.setGradientFill(ColourGradient(Colour(0xff0f0f1e), 0, 0,
        Colour(0xff1a1a2e), 0, (float)getHeight(), false));
    g.fillAll();

    // Panel backgrounds
    g.setColour(Colour(0xff16213e).withAlpha(0.8f));
    g.fillRoundedRectangle(10, 10, (float)(getWidth() - 20), 80, 10);
    g.fillRoundedRectangle(10, 100, (float)(getWidth() - 280), 200, 10);
    g.fillRoundedRectangle((float)(getWidth() - 260), 100, 250, (float)(getHeight() - 110), 10);
}

void PlayerGUI::resized()
{
    int margin = 20;
    int rightPanelX = getWidth() - 250;

    // Top info bar
    fileNameLabel.setBounds(margin, margin, getWidth() - 40, 30);
    timeLabel.setBounds(margin, margin + 35, getWidth() - 40, 25);

    // Waveform display
    waveformDisplay.setBounds(margin, 110, getWidth() - 290, 180);

    // Main control buttons (centered row) - أكبر حجماً
    int btnY = 310;
    int centerX = (getWidth() - 290) / 2;
    prevTrackButton.setBounds(centerX - 180, btnY, 70, 55);
    backward10Button.setBounds(centerX - 100, btnY, 70, 55);
    playPauseButton.setBounds(centerX - 35, btnY - 5, 85, 65); // أكبر زر
    forward10Button.setBounds(centerX + 55, btnY, 70, 55);
    nextTrackButton.setBounds(centerX + 135, btnY, 70, 55);

    // Secondary controls - أوسع
    btnY = 380;
    int btnWidth = 90; // عرض موحد لجميع الأزرار
    int btnHeight = 40; // ارتفاع موحد
    int btnSpacing = 10; // مسافة بين الأزرار

    loadButton.setBounds(margin, btnY, btnWidth, btnHeight);
    stopButton.setBounds(margin + btnWidth + btnSpacing, btnY, btnWidth, btnHeight);
    startButton.setBounds(margin + (btnWidth + btnSpacing) * 2, btnY, btnWidth, btnHeight);
    endButton.setBounds(margin + (btnWidth + btnSpacing) * 3, btnY, btnWidth, btnHeight);
    muteButton.setBounds(margin + (btnWidth + btnSpacing) * 4, btnY, btnWidth, btnHeight);
    loopButton.setBounds(margin + (btnWidth + btnSpacing) * 5, btnY, btnWidth, btnHeight);

    // A-B Loop controls - أوسع
    btnY = 430;
    int abBtnWidth = 110; // أوسع للأزرار المهمة
    int abBtnHeight = 40;

    setPointAButton.setBounds(margin, btnY, abBtnWidth, abBtnHeight);
    setPointBButton.setBounds(margin + abBtnWidth + btnSpacing, btnY, abBtnWidth, abBtnHeight);
    clearABButton.setBounds(margin + (abBtnWidth + btnSpacing) * 2, btnY, abBtnWidth, abBtnHeight);
    addMarkerButton.setBounds(margin + (abBtnWidth + btnSpacing) * 3, btnY, abBtnWidth + 20, abBtnHeight);

    // Right panel - Volume and Speed
    volumeLabel.setBounds(rightPanelX, 110, 100, 20);
    volumeSlider.setBounds(rightPanelX + 20, 135, 60, 150);

    speedLabel.setBounds(rightPanelX + 120, 110, 100, 20);
    speedSlider.setBounds(rightPanelX + 110, 135, 100, 100);

    // Marker list
    markerListLabel.setBounds(rightPanelX, 310, 230, 25);
    markerListBox.setBounds(rightPanelX, 340, 230, getHeight() - 360);
}

void PlayerGUI::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    playerAudio.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerGUI::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    playerAudio.getNextAudioBlock(bufferToFill);

    // A-B Loop handling
    if (hasABLoop && abLoopPointB > abLoopPointA)
    {
        double pos = playerAudio.getPosition();
        if (pos >= abLoopPointB)
        {
            playerAudio.setPosition(abLoopPointA);
        }
    }
    // Normal loop
    else if (loopEnabled)
    {
        double pos = playerAudio.getPosition();
        double length = playerAudio.getLength();
        if (length > 0 && pos >= length - 0.05)
        {
            playerAudio.setPosition(0.0);
        }
    }
}

void PlayerGUI::releaseResources()
{
    playerAudio.releaseResources();
}

void PlayerGUI::timerCallback()
{
    if (currentDuration > 0)
    {
        double currentPos = playerAudio.getPosition();
        waveformDisplay.setPosition(currentPos);
        updateTimeDisplay();
    }
}

void PlayerGUI::setGain(float gain)
{
    playerAudio.setGain(gain);
}

void PlayerGUI::updateTimeDisplay()
{
    double currentPos = playerAudio.getPosition();
    juce::String timeStr = formatTime(currentPos) + " / " + formatTime(currentDuration);
    timeLabel.setText(timeStr, dontSendNotification);
}

juce::String PlayerGUI::formatTime(double seconds)
{
    int mins = (int)seconds / 60;
    int secs = (int)seconds % 60;
    return juce::String::formatted("%02d:%02d", mins, secs);
}

void PlayerGUI::loadAudioFile(const juce::File& file)
{
    if (playerAudio.loadFile(file))
    {
        currentFileName = file.getFileNameWithoutExtension();
        currentDuration = playerAudio.getLength();

        fileNameLabel.setText("♪ " + currentFileName, dontSendNotification);
        waveformDisplay.setWaveform(file);
        waveformDisplay.clearMarkers();

        playerAudio.play();
        isPlaying = true;
        playPauseButton.setButtonText("⏸");

        // Add to playlist if not already there
        bool inPlaylist = false;
        for (const auto& f : playlistFiles)
        {
            if (f == file)
            {
                inPlaylist = true;
                currentPlaylistIndex = playlistFiles.indexOf(f);
                break;
            }
        }

        if (!inPlaylist)
        {
            playlistFiles.add(file);
            playlist.add(file.getFileNameWithoutExtension());
            currentPlaylistIndex = playlistFiles.size() - 1;
        }
    }
}

void PlayerGUI::loadNextTrack()
{
    if (currentPlaylistIndex >= 0 && currentPlaylistIndex < playlistFiles.size() - 1)
    {
        loadAudioFile(playlistFiles[currentPlaylistIndex + 1]);
    }
}

void PlayerGUI::loadPreviousTrack()
{
    if (currentPlaylistIndex > 0)
    {
        loadAudioFile(playlistFiles[currentPlaylistIndex - 1]);
    }
}

void PlayerGUI::jumpForward(double seconds)
{
    double newPos = playerAudio.getPosition() + seconds;
    if (newPos > currentDuration) newPos = currentDuration;
    playerAudio.setPosition(newPos);
}

void PlayerGUI::jumpBackward(double seconds)
{
    double newPos = playerAudio.getPosition() - seconds;
    if (newPos < 0.0) newPos = 0.0;
    playerAudio.setPosition(newPos);
}

void PlayerGUI::addMarkerAtCurrentPosition()
{
    double currentPos = playerAudio.getPosition();
    int markerNum = (int)waveformDisplay.getMarkers().size() + 1;
    juce::String markerName = "Marker " + juce::String(markerNum) + " (" + formatTime(currentPos) + ")";
    waveformDisplay.addMarker(currentPos, markerName);
    markerListBox.updateContent();
}

void PlayerGUI::saveSession()
{
    juce::File sessionFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("AudioPlayer").getChildFile("session.xml");

    sessionFile.getParentDirectory().createDirectory();

    juce::XmlElement session("Session");

    if (currentPlaylistIndex >= 0 && currentPlaylistIndex < playlistFiles.size())
    {
        session.setAttribute("lastFile", playlistFiles[currentPlaylistIndex].getFullPathName());
        session.setAttribute("lastPosition", playerAudio.getPosition());
    }

    // Save playlist
    auto* playlistXml = session.createNewChildElement("Playlist");
    for (const auto& file : playlistFiles)
    {
        auto* fileXml = playlistXml->createNewChildElement("File");
        fileXml->setAttribute("path", file.getFullPathName());
    }

    session.writeTo(sessionFile);
}

void PlayerGUI::loadSession()
{
    juce::File sessionFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("AudioPlayer").getChildFile("session.xml");

    if (sessionFile.existsAsFile())
    {
        auto session = juce::XmlDocument::parse(sessionFile);
        if (session != nullptr)
        {
            // Load playlist
            if (auto* playlistXml = session->getChildByName("Playlist"))
            {
                for (auto* fileXml : playlistXml->getChildIterator())
                {
                    juce::String path = fileXml->getStringAttribute("path");
                    juce::File file(path);
                    if (file.existsAsFile())
                    {
                        playlistFiles.add(file);
                        playlist.add(file.getFileNameWithoutExtension());
                    }
                }
            }

            // Load last file
            juce::String lastFilePath = session->getStringAttribute("lastFile");
            double lastPosition = session->getDoubleAttribute("lastPosition", 0.0);

            if (lastFilePath.isNotEmpty())
            {
                juce::File lastFile(lastFilePath);
                if (lastFile.existsAsFile())
                {
                    if (playerAudio.loadFile(lastFile))
                    {
                        currentFileName = lastFile.getFileNameWithoutExtension();
                        currentDuration = playerAudio.getLength();
                        fileNameLabel.setText("♪ " + currentFileName, dontSendNotification);
                        waveformDisplay.setWaveform(lastFile);
                        playerAudio.setPosition(lastPosition);

                        for (int i = 0; i < playlistFiles.size(); ++i)
                        {
                            if (playlistFiles[i] == lastFile)
                            {
                                currentPlaylistIndex = i;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select audio files...",
            juce::File{},
            "*.wav;*.mp3;*.aiff;*.flac;*.ogg;*.m4a");

        auto flags = juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles |
            juce::FileBrowserComponent::canSelectMultipleItems;

        fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc)
            {
                auto files = fc.getResults();
                if (files.size() > 0)
                {
                    // Clear existing playlist
                    playlistFiles.clear();
                    playlist.clear();

                    // Add all selected files
                    for (const auto& file : files)
                    {
                        if (file.existsAsFile())
                        {
                            playlistFiles.add(file);
                            playlist.add(file.getFileNameWithoutExtension());
                        }
                    }

                    // Load first file
                    if (playlistFiles.size() > 0)
                    {
                        loadAudioFile(playlistFiles[0]);
                    }
                }
            });
    }

    if (button == &playPauseButton)
    {
        if (isPlaying)
        {
            playerAudio.stop();
            playPauseButton.setButtonText("▶");
        }
        else
        {
            playerAudio.play();
            playPauseButton.setButtonText("⏸");
        }
        isPlaying = !isPlaying;
    }

    if (button == &stopButton)
    {
        playerAudio.stop();
        playerAudio.setPosition(0.0);
        isPlaying = false;
        playPauseButton.setButtonText("▶");
    }

    if (button == &prevTrackButton) loadPreviousTrack();
    if (button == &nextTrackButton) loadNextTrack();
    if (button == &forward10Button) jumpForward(10.0);
    if (button == &backward10Button) jumpBackward(10.0);
    if (button == &startButton) playerAudio.setPosition(0.0);
    if (button == &endButton) playerAudio.setPosition(currentDuration);

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
        loopEnabled = !loopEnabled;
        loopButton.setButtonText(loopEnabled ? "Loop On" : "Loop");
        loopButton.setColour(TextButton::buttonColourId,
            loopEnabled ? Colour(0xff00ff88) : Colour(0xff786fa6));
    }

    if (button == &setPointAButton)
    {
        abLoopPointA = playerAudio.getPosition();
        if (abLoopPointB > abLoopPointA)
        {
            hasABLoop = true;
            waveformDisplay.setABLoopPoints(abLoopPointA, abLoopPointB);
        }
    }

    if (button == &setPointBButton)
    {
        abLoopPointB = playerAudio.getPosition();
        if (abLoopPointB > abLoopPointA && abLoopPointA >= 0)
        {
            hasABLoop = true;
            waveformDisplay.setABLoopPoints(abLoopPointA, abLoopPointB);
        }
    }

    if (button == &clearABButton)
    {
        hasABLoop = false;
        abLoopPointA = -1.0;
        abLoopPointB = -1.0;
        waveformDisplay.clearABLoop();
    }

    if (button == &addMarkerButton)
    {
        addMarkerAtCurrentPosition();
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        float volume = (float)slider->getValue();
        playerAudio.setGain(volume);
        if (!isMuted) previousVolume = volume;
    }

    if (slider == &speedSlider)
    {
        playerAudio.setSpeed((float)slider->getValue());
    }
}

// ============ Marker List Model Implementation ============
int PlayerGUI::MarkerListModel::getNumRows()
{
    return (int)parent.waveformDisplay.getMarkers().size();
}

void PlayerGUI::MarkerListModel::paintListBoxItem(int row, juce::Graphics& g,
    int width, int height, bool selected)
{
    if (selected)
        g.fillAll(Colour(0xff00d4ff).withAlpha(0.5f));
    else
        g.fillAll(row % 2 ? Colour(0xff16213e) : Colour(0xff1a1a2e));

    const auto& markers = parent.waveformDisplay.getMarkers();
    if (row < markers.size())
    {
        g.setColour(Colours::white);
        g.setFont(13.0f);
        g.drawText(markers[row].name, 10, 0, width - 20, height, Justification::centredLeft);

        // Marker indicator dot
        g.setColour(markers[row].colour);
        g.fillEllipse(5, (float)(height / 2 - 3), 6, 6);
    }
}

void PlayerGUI::MarkerListModel::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    const auto& markers = parent.waveformDisplay.getMarkers();
    if (row < markers.size())
    {
        parent.playerAudio.setPosition(markers[row].timePosition);
        if (!parent.isPlaying)
        {
            parent.playerAudio.play();
            parent.isPlaying = true;
            parent.playPauseButton.setButtonText("⏸");
        }
    }
}

