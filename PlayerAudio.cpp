#include "PlayerAudio.h"

PlayerAudio::PlayerAudio()
    :resampleSource(&transportSource, false)
{
    formatManager.registerBasicFormats();
}

PlayerAudio::~PlayerAudio()
{
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    resampleSource.getNextAudioBlock(bufferToFill);

    // Check for looping
    if (isLoopingEnabled && transportSource.getCurrentPosition() >= loopEnd)
    {
        transportSource.setPosition(loopStart);
    }
}

void PlayerAudio::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
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

            // Generate waveform data
            generateWaveform(reader);

            // Reset loop points
            loopStart = 0.0;
            loopEnd = transportSource.getLengthInSeconds();

            return true;
        }
    }
    return false;
}

void PlayerAudio::generateWaveform(juce::AudioFormatReader* reader)
{
    waveformData.clear();

    if (!reader)
        return;

    const int numSamples = (int)reader->lengthInSamples;
    const int numChannels = reader->numChannels;

    if (numSamples == 0)
        return;

    // We'll create a waveform with 1000 points (adjustable)
    const int waveformPoints = 1000;
    waveformData.resize(waveformPoints);

    // Calculate samples per waveform point
    const int samplesPerPoint = numSamples / waveformPoints;

    // Buffer to read audio data
    juce::AudioBuffer<float> tempBuffer(numChannels, samplesPerPoint);

    for (int point = 0; point < waveformPoints; ++point)
    {
        // Read a chunk of audio data
        const int startSample = point * samplesPerPoint;
        reader->read(&tempBuffer, 0, samplesPerPoint, startSample, true, true);

        // Calculate RMS value for this segment (across all channels)
        float rms = 0.0f;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float* channelData = tempBuffer.getReadPointer(channel);
            float channelRms = 0.0f;

            for (int i = 0; i < samplesPerPoint; ++i)
            {
                channelRms += channelData[i] * channelData[i];
            }

            channelRms = std::sqrt(channelRms / samplesPerPoint);
            rms += channelRms;
        }

        // Average across channels
        rms /= numChannels;

        // Store the RMS value (you might want to apply some scaling)
        waveformData[point] = rms;
    }

    // Normalize the waveform data to [0, 1] range
    float maxValue = 0.0f;
    for (int i = 0; i < waveformPoints; ++i)
    {
        if (waveformData[i] > maxValue)
            maxValue = waveformData[i];
    }

    if (maxValue > 0.0f)
    {
        for (int i = 0; i < waveformPoints; ++i)
        {
            waveformData[i] /= maxValue;
        }
    }
}

    /*
    // Create a demo waveform with interesting pattern
    int numPoints = 500;
    waveformData.resize(numPoints);

    for (int i = 0; i < numPoints; ++i)
    {
        double progress = (double)i / numPoints;

        // Create a complex waveform pattern
        double x = progress * 8.0 * juce::MathConstants<double>::pi;

        waveformData[i] = 0.5f +
            0.4f * std::sin(x) * std::exp(-progress * 2.0) +
            0.2f * std::sin(x * 3.0) * std::exp(-progress * 3.0) +
            0.1f * std::sin(x * 8.0) * std::exp(-progress * 5.0);

        // Ensure values are between 0 and 1
        waveformData[i] = juce::jlimit(0.0f, 1.0f, waveformData[i]);
    }
}
*/


void PlayerAudio::start()
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

void PlayerAudio::setSpeed(float speedRatio)
{
    resampleSource.setResamplingRatio(speedRatio);
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

void PlayerAudio::setLoopStart(double pos)
{
    loopStart = juce::jlimit(0.0, getLength(), pos);
}

void PlayerAudio::setLoopEnd(double pos)
{
    loopEnd = juce::jlimit(0.0, getLength(), pos);
}

void PlayerAudio::setLooping(bool shouldLoop)
{
    isLoopingEnabled = shouldLoop;
}
