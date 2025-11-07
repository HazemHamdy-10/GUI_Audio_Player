#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

using namespace juce;

// ============ Audio Marker Structure ============
struct AudioMarker
{
    double timePosition;
    juce::String name;
    juce::Colour colour;

    AudioMarker(double time, const juce::String& n, juce::Colour c = Colours::yellow)
        : timePosition(time), name(n), colour(c) {
    }
};

// ============ Waveform Display Component ============
class WaveformDisplay : public juce::Component, public juce::Timer
{
public:
    WaveformDisplay(PlayerAudio& audio);
    ~WaveformDisplay() override {}

    void paint(juce::Graphics& g) override;
    void setWaveform(const juce::File& file);
    void setPosition(double pos);
    void mouseDown(const juce::MouseEvent& event) override;
    void timerCallback() override;

    void addMarker(double time, const juce::String& name);
    void clearMarkers();
    const std::vector<AudioMarker>& getMarkers() const { return markers; }

    void setABLoopPoints(double pointA, double pointB);
    void clearABLoop();
    double getClickedTime(int x) const;

private:
    PlayerAudio& playerAudio;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnail thumbnail;
    juce::AudioThumbnailCache thumbnailCache;
    double currentPosition = 0.0;
    std::vector<AudioMarker> markers;
    double loopPointA = -1.0;
    double loopPointB = -1.0;
    bool hasABLoop = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

// ============ Player GUI Component ============
class PlayerGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::Timer
{
public:
    PlayerGUI();
    ~PlayerGUI() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    void timerCallback() override;
    void setGain(float gain);

private:
    PlayerAudio playerAudio;
    WaveformDisplay waveformDisplay;

    // Main control buttons
    juce::TextButton loadButton{ "Load" };
    juce::TextButton playPauseButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton prevTrackButton{ "previous" };
    juce::TextButton nextTrackButton{ "Next" };

    // Navigation buttons
    juce::TextButton backward10Button{ "-10s" };
    juce::TextButton forward10Button{ "+10s" };
    juce::TextButton startButton{ "End" };
    juce::TextButton endButton{ "Strat" };

    // Feature buttons
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop" };
    juce::TextButton setPointAButton{ "Set A" };
    juce::TextButton setPointBButton{ "Set B" };
    juce::TextButton clearABButton{ "Clear AB" };
    juce::TextButton addMarkerButton{ "Add Marker" };

    // Sliders
    juce::Slider volumeSlider;
    juce::Slider speedSlider;

    // Labels
    juce::Label fileNameLabel;
    juce::Label timeLabel;
    juce::Label volumeLabel;
    juce::Label speedLabel;
    juce::Label markerListLabel;

    // Marker list
    juce::ListBox markerListBox;

    // Playlist
    juce::StringArray playlist;
    juce::Array<juce::File> playlistFiles;
    int currentPlaylistIndex = -1;

    // A-B Loop
    double abLoopPointA = -1.0;
    double abLoopPointB = -1.0;
    bool hasABLoop = false;

    juce::String currentFileName;
    double currentDuration = 0.0;

    std::unique_ptr<juce::FileChooser> fileChooser;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void loadAudioFile(const juce::File& file);
    void loadNextTrack();
    void loadPreviousTrack();
    void updateTimeDisplay();
    void jumpForward(double seconds);
    void jumpBackward(double seconds);
    void addMarkerAtCurrentPosition();
    void saveSession();
    void loadSession();
    juce::String formatTime(double seconds);

    bool isPlaying = false;
    bool isMuted = false;
    bool loopEnabled = false;
    float previousVolume = 0.7f;

    // Marker list model
    class MarkerListModel : public juce::ListBoxModel
    {
    public:
        MarkerListModel(PlayerGUI& owner) : parent(owner) {}
        int getNumRows() override;
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    private:
        PlayerGUI& parent;
    };

    std::unique_ptr<MarkerListModel> markerListModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};
