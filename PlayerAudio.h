#pragma once						// PlayerAudio.h
#include <JuceHeader.h>

class PlayerAudio
{
public:
    PlayerAudio();
    ~PlayerAudio();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    bool loadFile(const juce::File& file);
    void start();
    void stop();
    void setGain(float gain);
    void setSpeed(float speedRatio);
    void setPosition(double pos);
    double getPosition() const;
    double getLength() const;

    // New features
    void setLoopStart(double pos);
    void setLoopEnd(double pos);
    void setLooping(bool shouldLoop);
    bool isLooping() const { return isLoopingEnabled; }
    double getLoopStart() const { return loopStart; }
    double getLoopEnd() const { return loopEnd; }

    // Waveform data
    const std::vector<float>& getWaveformData() const { return waveformData; }
    int getWaveformSize() const { return (int)waveformData.size(); }
    bool hasWaveform() const { return !waveformData.empty(); }

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    // Speed control
    juce::ResamplingAudioSource resampleSource;

    // Looping
    double loopStart = 0.0;
    double loopEnd = 0.0;
    bool isLoopingEnabled = false;

    // Waveform
    std::vector<float> waveformData;
    void generateWaveform(juce::AudioFormatReader* reader);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerAudio)
};