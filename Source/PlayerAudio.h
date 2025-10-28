#pragma once       // PlayerAudio.h 
#include <JuceHeader.h> 

class PlayerAudio
{
public:
    PlayerAudio();
    ~PlayerAudio();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    void setLooping(bool shouldLoop);

    bool loadFile(const juce::File& file);
    void play();
    void stop();
    void setGain(float gain);
    void setPosition(double pos);
    double getPosition() const;
    double getLength() const;
    
private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplerSource; 
    juce::AudioTransportSource transportSource;
   
};