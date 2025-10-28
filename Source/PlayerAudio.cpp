#include "PlayerAudio.h"  // PlayerAudio.cpp

PlayerAudio::PlayerAudio()
{
    formatManager.registerBasicFormats();
}

PlayerAudio::~PlayerAudio()
{
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    transportSource.getNextAudioBlock(bufferToFill);
}

void PlayerAudio::releaseResources()
{
    transportSource.releaseResources();
}
void PlayerAudio::setLooping(bool shouldLoop)
{
    transportSource.setLooping(shouldLoop);
}

bool PlayerAudio::loadFile(const juce::File& file)
{
    if (file.existsAsFile())
    {
        if (auto* reader = formatManager.createReaderFor(file))
        {
            transportSource.stop();
            transportSource.setSource(nullptr);
            readerSource.reset();

            readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

            transportSource.setSource(readerSource.get(),
                0,
                nullptr,
                reader->sampleRate);
            transportSource.start();
        }
    }
    return true;
}

void PlayerAudio::play()
{
    transportSource.start();
}

void PlayerAudio::stop()
{
    transportSource.stop();
}

void PlayerAudio::setGain(float gain)
{
    transportSource.setGain(gain);
}

void PlayerAudio::setPosition(double pos)
{
    transportSource.setPosition(pos);
}
double PlayerAudio::getPosition() const
{
    return transportSource.getCurrentPosition();
}
double PlayerAudio::getLength() const
{
    return transportSource.getLengthInSeconds();
}