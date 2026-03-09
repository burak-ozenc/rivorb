#include "PluginProcessor.h"
#include "PluginEditor.h"

RivorbProcessor::RivorbProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Reverb
    addParameter (roomSize   = new juce::AudioParameterFloat ("roomSize",  "Room Size",  0.1f, 1.0f,   0.55f));
    addParameter (damping    = new juce::AudioParameterFloat ("damping",   "Damping",    0.0f, 1.0f,   0.45f));
    addParameter (preDelayMs = new juce::AudioParameterFloat ("preDelay",  "Pre-Delay",  0.0f, 100.0f, 20.0f));
    addParameter (wetLevel   = new juce::AudioParameterFloat ("wetLevel",  "Wet Level",  0.0f, 1.0f,   0.35f));

    // Pre-EQ
    addParameter (preHPFreq    = new juce::AudioParameterFloat ("preHPFreq",    "Pre HP Freq",    20.0f,  500.0f, 80.0f));
    addParameter (preBell1Freq = new juce::AudioParameterFloat ("preBell1Freq", "Pre Bell1 Freq", 100.0f, 2000.0f, 500.0f));
    addParameter (preBell1Gain = new juce::AudioParameterFloat ("preBell1Gain", "Pre Bell1 Gain", -12.0f, 12.0f,  0.0f));
    addParameter (preBell2Freq = new juce::AudioParameterFloat ("preBell2Freq", "Pre Bell2 Freq", 500.0f, 5000.0f, 2000.0f));
    addParameter (preBell2Gain = new juce::AudioParameterFloat ("preBell2Gain", "Pre Bell2 Gain", -12.0f, 12.0f,  0.0f));
    addParameter (preBell3Freq = new juce::AudioParameterFloat ("preBell3Freq", "Pre Bell3 Freq", 2000.0f, 16000.0f, 8000.0f));
    addParameter (preBell3Gain = new juce::AudioParameterFloat ("preBell3Gain", "Pre Bell3 Gain", -12.0f, 12.0f,  0.0f));

    // Harmonic saturation
    // Saturator
    addParameter (satDrive = new juce::AudioParameterFloat ("satDrive", "Sat Drive", 0.0f, 1.0f, 0.0f));
    addParameter (satBlend = new juce::AudioParameterFloat ("satBlend", "Sat Blend", 0.0f, 1.0f, 1.0f));

    // Modulation
    addParameter (modRate  = new juce::AudioParameterFloat ("modRate",  "Mod Rate",  0.01f, 5.0f,  0.5f));
    addParameter (modDepth = new juce::AudioParameterFloat ("modDepth", "Mod Depth", 0.0f,  1.0f,  0.0f));

    // Post-EQ
    addParameter (postEQEnabled = new juce::AudioParameterBool ("postEQ", "Post EQ", true));

    // Compressor
    addParameter (compThreshold = new juce::AudioParameterFloat ("compThresh",   "Comp Threshold", -60.0f, 0.0f,   -24.0f));
    addParameter (compRatio     = new juce::AudioParameterFloat ("compRatio",    "Comp Ratio",      1.0f,  20.0f,   4.0f));
    addParameter (compAttack    = new juce::AudioParameterFloat ("compAttack",   "Comp Attack",     1.0f,  200.0f, 10.0f));
    addParameter (compRelease   = new juce::AudioParameterFloat ("compRelease",  "Comp Release",    10.0f, 2000.0f,200.0f));
    // addParameter (compSidechain = new juce::AudioParameterBool  ("compSidechain","Ext Sidechain",  false));
}

void RivorbProcessor::updatePreEQ()
{
    using Coeffs = juce::dsp::IIR::Coefficients<float>;

    // Slot 0 — High pass
    auto hp = Coeffs::makeHighPass (currentSampleRate, preHPFreq->get(), 0.7f);
    *preEQLeft.get<0>().coefficients  = *hp;
    *preEQRight.get<0>().coefficients = *hp;

    // Slot 1 — Bell 1
    auto b1 = Coeffs::makePeakFilter (currentSampleRate, preBell1Freq->get(), 1.0f,
                                       juce::Decibels::decibelsToGain (preBell1Gain->get()));
    *preEQLeft.get<1>().coefficients  = *b1;
    *preEQRight.get<1>().coefficients = *b1;

    // Slot 2 — Bell 2
    auto b2 = Coeffs::makePeakFilter (currentSampleRate, preBell2Freq->get(), 1.0f,
                                       juce::Decibels::decibelsToGain (preBell2Gain->get()));
    *preEQLeft.get<2>().coefficients  = *b2;
    *preEQRight.get<2>().coefficients = *b2;

    // Slot 3 — Bell 3
    auto b3 = Coeffs::makePeakFilter (currentSampleRate, preBell3Freq->get(), 1.0f,
                                       juce::Decibels::decibelsToGain (preBell3Gain->get()));
    *preEQLeft.get<3>().coefficients  = *b3;
    *preEQRight.get<3>().coefficients = *b3;
}

void RivorbProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Prepare Pre-EQ chains
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = 1;

    preEQLeft.prepare  (spec);
    preEQRight.prepare (spec);

    // Prepare chorus
    juce::dsp::ProcessSpec stereoSpec;
    stereoSpec.sampleRate       = sampleRate;
    stereoSpec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    stereoSpec.numChannels      = 2;
    chorus.prepare (stereoSpec);

    // Post-EQ
    juce::dsp::ProcessSpec postSpec;
    postSpec.sampleRate       = sampleRate;
    postSpec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    postSpec.numChannels      = 1;

    postEQLeft.prepare  (postSpec);
    postEQRight.prepare (postSpec);

    analysisInterval  = (int) (sampleRate * 2.0);
    analysisCountdown = analysisInterval;
    int analysisSize  = (int) sampleRate;
    analysisBuffer.setSize (1, analysisSize);
    analysisBuffer.clear();

    updatePreEQ();

    // Pre-delay buffer — max 100ms
    int maxDelaySamples = (int) (sampleRate * 0.1);
    preDelayBufL.assign (maxDelaySamples, 0.0f);
    preDelayBufR.assign (maxDelaySamples, 0.0f);
    preDelayWritePos = 0;

    reverb.reset();
}

float RivorbProcessor::saturate (float x, float drive, float blend)
{
    if (drive < 0.001f) return x;

    // Odd — symmetric tanh shaper
    float odd  = std::tanh (x * (1.0f + drive * 4.0f))
                 / std::tanh (1.0f + drive * 4.0f);

    // Even — asymmetric quadratic shaper
    // x^2 term adds even harmonics, DC offset removed later
    float evenGain = drive * 0.5f;
    float even     = x + evenGain * x * x;
    // Soft clip to prevent runaway
    even = juce::jlimit (-1.0f, 1.0f, even);

    return odd * (1.0f - blend) + even * blend;
}

static float getPostBandEnergy (const std::vector<float>& magDb,
                                 const std::vector<float>& freqs,
                                 float lo, float hi)
{
    float sum = 0.0f; int count = 0;
    for (size_t i = 0; i < freqs.size(); ++i)
        if (freqs[i] >= lo && freqs[i] <= hi) { sum += magDb[i]; ++count; }
    return count > 0 ? sum / count : -96.0f;
}

void RivorbProcessor::analyzeAndUpdatePostEQ (const juce::AudioBuffer<float>& buffer)
{
    int bufSize    = analysisBuffer.getNumSamples();
    int numSamples = buffer.getNumSamples();

    if (numSamples < bufSize)
    {
        auto* data = analysisBuffer.getWritePointer (0);
        std::move (data + numSamples, data + bufSize, data);
    }

    int copyStart = std::max (0, bufSize - numSamples);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        analysisBuffer.addFrom (0, copyStart, buffer, ch,
                                std::max (0, numSamples - bufSize),
                                std::min (numSamples, bufSize));

    const int fftOrder = 11;
    const int fftSize  = 1 << fftOrder;

    juce::dsp::FFT fft (fftOrder);

    std::vector<float> fftData (fftSize * 2, 0.0f);
    int copyLen = std::min (bufSize, fftSize);
    std::copy (analysisBuffer.getReadPointer (0),
               analysisBuffer.getReadPointer (0) + copyLen,
               fftData.begin());

    for (int i = 0; i < fftSize; ++i)
        fftData[i] *= 0.5f * (1.0f - std::cos (2.0f * juce::MathConstants<float>::pi
                                                * i / fftSize));

    fft.performFrequencyOnlyForwardTransform (fftData.data());

    int numBins = fftSize / 2 + 1;
    std::vector<float> frequencies (numBins);
    std::vector<float> magDb (numBins);

    for (int i = 0; i < numBins; ++i)
    {
        frequencies[i] = (float) i * (float) currentSampleRate / (float) fftSize;
        float mag      = fftData[i] / (float) fftSize;
        magDb[i]       = mag > 0.0f ? 20.0f * std::log10 (mag) : -96.0f;
    }

    struct Band {
        EQDecision::Type type;
        float center, lo, hi, q, maxCut;
        bool  alwaysCut;
    };

    static const Band bands[] = {
        { EQDecision::Type::HighPass, 100,   60,    180,  0.7f, -6.0f, true  },
        { EQDecision::Type::Bell,     500,   350,   700,  1.0f, -9.0f, false },
        { EQDecision::Type::Bell,     3500,  2000,  5000, 0.8f, -9.0f, false },
        { EQDecision::Type::Bell,     8000,  6000, 10000, 2.0f, -9.0f, false },
    };

    float avgEnergy = 0.0f;
    for (auto v : magDb) avgEnergy += v;
    avgEnergy /= (float) magDb.size();

    postEQDecisions.clear();

    for (auto& band : bands)
    {
        float energy = getPostBandEnergy (magDb, frequencies, band.lo, band.hi);
        float diff   = energy - avgEnergy;

        if (band.alwaysCut)
            postEQDecisions.push_back ({ band.type, band.center, band.maxCut, band.q });
        else if (diff > 3.0f)
            postEQDecisions.push_back ({ band.type, band.center,
                                         std::max (band.maxCut, -diff), band.q });
    }

    rebuildPostEQChain();
}

void RivorbProcessor::rebuildPostEQChain()
{
    using Coeffs = juce::dsp::IIR::Coefficients<float>;

    auto applySlot = [&](int slot, Coeffs::Ptr c)
    {
        if (slot == 0) { *postEQLeft.get<0>().coefficients = *c;
                         *postEQRight.get<0>().coefficients = *c; }
        else if (slot == 1) { *postEQLeft.get<1>().coefficients = *c;
                              *postEQRight.get<1>().coefficients = *c; }
        else if (slot == 2) { *postEQLeft.get<2>().coefficients = *c;
                              *postEQRight.get<2>().coefficients = *c; }
        else if (slot == 3) { *postEQLeft.get<3>().coefficients = *c;
                              *postEQRight.get<3>().coefficients = *c; }
    };

    int slot = 0;
    for (auto& d : postEQDecisions)
    {
        if (slot >= 4) break;
        Coeffs::Ptr c;
        if (d.type == EQDecision::Type::HighPass)
            c = Coeffs::makeHighPass (currentSampleRate, d.frequency, d.q);
        else
            c = Coeffs::makePeakFilter (currentSampleRate, d.frequency, d.q,
                                         juce::Decibels::decibelsToGain (d.gainDb));
        applySlot (slot, c);
        ++slot;
    }
}

float RivorbProcessor::attackCoeff() const
{
    return std::exp (-1.0f / ((compAttack->get() / 1000.0f) * (float) currentSampleRate));
}

float RivorbProcessor::releaseCoeff() const
{
    return std::exp (-1.0f / ((compRelease->get() / 1000.0f) * (float) currentSampleRate));
}

void RivorbProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                     juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    int numSamples = buffer.getNumSamples();

    // ── Update reverb params ──────────────────────────────────────────────
    reverbParams.roomSize = roomSize->get();
    reverbParams.damping  = damping->get();
    reverbParams.wetLevel = 1.0f;
    reverbParams.dryLevel = 0.0f;
    reverbParams.width    = 1.0f;
    reverb.setParameters (reverbParams);

    // ── Update Pre-EQ (every block, cheap operation) ──────────────────────
    updatePreEQ();

    // ── Save dry copy ─────────────────────────────────────────────────────
    juce::AudioBuffer<float> dry (buffer.getNumChannels(), numSamples);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        dry.copyFrom (ch, 0, buffer, ch, 0, numSamples);

    // ── Harmonic Saturator ────────────────────────────────────────────────
    {
        float drive = satDrive->get();
        float blend = satBlend->get();

        auto* satL = buffer.getWritePointer (0);
        auto* satR = buffer.getWritePointer (1);

        for (int i = 0; i < numSamples; ++i)
        {
            satL[i] = saturate (satL[i], drive, blend);
            satR[i] = saturate (satR[i], drive, blend);
        }

        // Remove DC offset introduced by even harmonic shaper
        // Simple first order high pass at ~20Hz
        static float dcL = 0.0f, dcR = 0.0f;
        const float  dcCoeff = 1.0f - (2.0f * juce::MathConstants<float>::pi * 20.0f
                                       / (float) currentSampleRate);
        for (int i = 0; i < numSamples; ++i)
        {
            dcL    = dcCoeff * dcL + (1.0f - dcCoeff) * satL[i];
            satL[i] -= dcL;
            dcR    = dcCoeff * dcR + (1.0f - dcCoeff) * satR[i];
            satR[i] -= dcR;
        }
    }

    // ── Pre-EQ ────────────────────────────────────────────────────────────
    {
        juce::dsp::AudioBlock<float> block (buffer);
        auto leftBlock  = block.getSingleChannelBlock (0);
        auto rightBlock = block.getSingleChannelBlock (1);
        preEQLeft.process  (juce::dsp::ProcessContextReplacing<float> (leftBlock));
        preEQRight.process (juce::dsp::ProcessContextReplacing<float> (rightBlock));
    }

    // ── Pre-delay ─────────────────────────────────────────────────────────
    int delaySamples = (int) (currentSampleRate * preDelayMs->get() / 1000.0f);
    delaySamples     = juce::jlimit (0, (int) preDelayBufL.size() - 1, delaySamples);

    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getWritePointer (1);

    for (int i = 0; i < numSamples; ++i)
    {
        int readPos = (preDelayWritePos - delaySamples + (int) preDelayBufL.size())
                      % (int) preDelayBufL.size();

        float delayedL = preDelayBufL[readPos];
        float delayedR = preDelayBufR[readPos];

        preDelayBufL[preDelayWritePos] = L[i];
        preDelayBufR[preDelayWritePos] = R[i];

        L[i] = delayedL;
        R[i] = delayedR;

        preDelayWritePos = (preDelayWritePos + 1) % (int) preDelayBufL.size();
    }

    // ── Reverb (wet only) ─────────────────────────────────────────────────
    reverb.processStereo (L, R, numSamples);

    // ── Modulation (chorus on wet tail) ──────────────────────────────────
    chorus.setRate  (modRate->get());
    chorus.setDepth (modDepth->get());
    chorus.setMix   (1.0f);

    juce::dsp::AudioBlock<float>            modBlock (buffer);
    juce::dsp::ProcessContextReplacing<float> modCtx (modBlock);
    chorus.process (modCtx);

    // ── Post-EQ (optional, spectral analysis driven) ──────────────────────
    analysisCountdown -= numSamples;
    if (analysisCountdown <= 0)
    {
        analyzeAndUpdatePostEQ (buffer);
        analysisCountdown = analysisInterval;
    }

    if (postEQEnabled->get() && !postEQDecisions.empty())
    {
        juce::dsp::AudioBlock<float> postBlock (buffer);
        auto postL = postBlock.getSingleChannelBlock (0);
        auto postR = postBlock.getSingleChannelBlock (1);
        postEQLeft.process  (juce::dsp::ProcessContextReplacing<float> (postL));
        postEQRight.process (juce::dsp::ProcessContextReplacing<float> (postR));
    }

    // ── Sidechain Compressor ──────────────────────────────────────────────
    {
        const float threshold  = juce::Decibels::decibelsToGain (compThreshold->get());
        const float ratio      = compRatio->get();
        const float attCoeff   = attackCoeff();
        const float relCoeff   = releaseCoeff();


        // Always use dry copy as detector (self mode)
        // External sidechain will be added in a later stage
        const float* detL = dry.getReadPointer (0);
        const float* detR = dry.getReadPointer (1);

        auto* wetL = buffer.getWritePointer (0);
        auto* wetR = buffer.getWritePointer (1);

        for (int i = 0; i < numSamples; ++i)
        {
            // Peak detector on sidechain signal
            float detPeak = std::max (std::abs (detL[i]), std::abs (detR[i]));

            // Envelope follower
            float coeff   = detPeak > envelopeL ? attCoeff : relCoeff;
            envelopeL     = coeff * envelopeL + (1.0f - coeff) * detPeak;

            // Gain computer
            float gainDb  = 0.0f;
            if (envelopeL > threshold)
            {
                float envDb   = juce::Decibels::gainToDecibels (envelopeL);
                float threshDb = juce::Decibels::gainToDecibels (threshold);
                gainDb        = (threshDb - envDb) * (1.0f - 1.0f / ratio);
            }

            float targetGain = juce::Decibels::decibelsToGain (gainDb);

            // Smooth gain changes
            gainSmoothed  = gainSmoothed * 0.999f + targetGain * 0.001f;

            wetL[i] *= gainSmoothed;
            wetR[i] *= gainSmoothed;
        }
    }

    // ── Mix dry + wet ─────────────────────────────────────────────────────
    float wl = wetLevel->get();
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* wet = buffer.getWritePointer (ch);
        auto* d   = dry.getReadPointer (ch);
        for (int i = 0; i < numSamples; ++i)
            wet[i] = d[i] * (1.0f - wl) + wet[i] * wl;
    }
}

juce::AudioProcessorEditor* RivorbProcessor::createEditor()
{
    return new RivorbEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RivorbProcessor();
}