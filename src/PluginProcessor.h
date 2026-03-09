#pragma once
#include <JuceHeader.h>

class RivorbProcessor : public juce::AudioProcessor
{
public:
    RivorbProcessor();
    ~RivorbProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Rivorb"; }
    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    // ── Parameters ────────────────────────────────────────────────────────

    // Reverb
    juce::AudioParameterFloat* roomSize;
    juce::AudioParameterFloat* damping;
    juce::AudioParameterFloat* preDelayMs;
    juce::AudioParameterFloat* wetLevel;

    // Pre-EQ
    juce::AudioParameterFloat* preHPFreq;
    juce::AudioParameterFloat* preBell1Freq;
    juce::AudioParameterFloat* preBell1Gain;
    juce::AudioParameterFloat* preBell2Freq;
    juce::AudioParameterFloat* preBell2Gain;
    juce::AudioParameterFloat* preBell3Freq;
    juce::AudioParameterFloat* preBell3Gain;

    // Harmonic saturation
    // Saturator
    juce::AudioParameterFloat* satDrive;
    juce::AudioParameterFloat* satBlend;  // 0 = all odd, 1 = all even

    // Modulation
    juce::AudioParameterFloat* modRate;
    juce::AudioParameterFloat* modDepth;
    // Post-EQ
    juce::AudioParameterBool*  postEQEnabled;

    // Compressor
    juce::AudioParameterFloat* compThreshold;
    juce::AudioParameterFloat* compRatio;
    juce::AudioParameterFloat* compAttack;
    juce::AudioParameterFloat* compRelease;
    // juce::AudioParameterBool*  compSidechain;  // false = self, true = external

private:
    void updatePreEQ();

    // Pre-EQ filter chain — 4 filters (HP + 3 bells), stereo
    using Filter = juce::dsp::IIR::Filter<float>;
    using PreEQChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    PreEQChain preEQLeft, preEQRight;

    juce::Reverb             reverb;
    juce::Reverb::Parameters reverbParams;

    // Pre-delay ring buffer
    std::vector<float> preDelayBufL, preDelayBufR;
    int                preDelayWritePos { 0 };

    // Harmonic saturation
    static float saturate (float sample, float drive, float blend);

    // Modulation
    juce::dsp::Chorus<float> chorus;

    double currentSampleRate { 44100.0 };

    // Post-EQ
    struct EQDecision
    {
        enum class Type { HighPass, Bell };
        Type  type;
        float frequency;
        float gainDb;
        float q;
    };

    void analyzeAndUpdatePostEQ (const juce::AudioBuffer<float>& buffer);
    void rebuildPostEQChain();

    using PostFilter  = juce::dsp::IIR::Filter<float>;
    using PostEQChain = juce::dsp::ProcessorChain<PostFilter, PostFilter, PostFilter, PostFilter>;
    PostEQChain postEQLeft, postEQRight;

    std::vector<EQDecision> postEQDecisions;

    int  analysisCountdown { 0 };
    int  analysisInterval  { 0 };
    juce::AudioBuffer<float> analysisBuffer;

    // Compressor state
    float envelopeL    { 0.0f };
    float envelopeR    { 0.0f };
    float gainSmoothed { 1.0f };

    float attackCoeff()  const;
    float releaseCoeff() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RivorbProcessor);
};