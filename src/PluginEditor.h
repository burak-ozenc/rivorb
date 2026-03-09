#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ─────────────────────────────────────────────
// Font helper (JUCE 8)
// ─────────────────────────────────────────────

static inline juce::Font monoFont (float height, bool bold = false)
{
    auto opts = juce::FontOptions()
                    .withName (juce::Font::getDefaultMonospacedFontName())
                    .withHeight (height);
    if (bold)
        opts = opts.withStyle ("Bold");
    return juce::Font (opts);
}

// ─────────────────────────────────────────────
// Custom LookAndFeel
// ─────────────────────────────────────────────

class RivorbLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RivorbLookAndFeel()
    {
        setColour (juce::Slider::thumbColourId,         juce::Colour (0xffe05e3a));
        setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffe05e3a));
        setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff2a2a2e));
        setColour (juce::Slider::textBoxTextColourId,   juce::Colour (0xffaaaaaa));
        setColour (juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::ToggleButton::tickColourId,    juce::Colour (0xffe05e3a));
        setColour (juce::ToggleButton::tickDisabledColourId, juce::Colour (0xff444444));
    }

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override
    {
        float radius    = (float) std::min (width, height) * 0.5f - 4.0f;
        float centreX   = (float) x + (float) width  * 0.5f;
        float centreY   = (float) y + (float) height * 0.5f;
        float angle     = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Outer ring track
        juce::Path track;
        track.addCentredArc (centreX, centreY, radius, radius,
                             0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xff2a2a2e));
        g.strokePath (track, juce::PathStrokeType (2.0f,
                      juce::PathStrokeType::curved,
                      juce::PathStrokeType::rounded));

        // Filled arc
        juce::Path fill;
        fill.addCentredArc (centreX, centreY, radius, radius,
                            0.0f, rotaryStartAngle, angle, true);
        g.setColour (juce::Colour (0xffe05e3a));
        g.strokePath (fill, juce::PathStrokeType (2.0f,
                      juce::PathStrokeType::curved,
                      juce::PathStrokeType::rounded));

        // Inner circle
        float innerR = radius * 0.62f;
        g.setColour (juce::Colour (0xff1a1a1e));
        g.fillEllipse (centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f);

        g.setColour (juce::Colour (0xff2e2e32));
        g.drawEllipse (centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f, 1.0f);

        // Pointer line
        float lineLen = innerR * 0.65f;
        float px      = centreX + std::sin (angle) * lineLen;
        float py      = centreY - std::cos (angle) * lineLen;
        g.setColour (juce::Colour (0xffe05e3a));
        g.drawLine (centreX, centreY, px, py, 1.5f);

        // Center dot
        g.setColour (juce::Colour (0xffe05e3a));
        g.fillEllipse (centreX - 1.5f, centreY - 1.5f, 3.0f, 3.0f);
    }

    juce::Font getLabelFont (juce::Label&) override
    {
        return monoFont (9.5f);
    }
};


// ─────────────────────────────────────────────
// Editor
// ─────────────────────────────────────────────

class RivorbEditor : public juce::AudioProcessorEditor
{
public:
    explicit RivorbEditor (RivorbProcessor&);
    ~RivorbEditor() override;

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    RivorbProcessor&    processor;
    RivorbLookAndFeel   laf;

    // ── Helpers ───────────────────────────────────────────────────────────
    void setupKnob (juce::Slider& s, juce::Label& l,
                    const juce::String& text, double lo, double hi,
                    double step, double val);

    void drawSectionHeader (juce::Graphics& g, const juce::String& text,
                            int x, int y, int w);

    // ── Pre-EQ ────────────────────────────────────────────────────────────
    juce::Slider preHPSlider;
    juce::Slider preBell1FreqSlider, preBell1GainSlider;
    juce::Slider preBell2FreqSlider, preBell2GainSlider;
    juce::Slider preBell3FreqSlider, preBell3GainSlider;
    juce::Label  preHPLabel;
    juce::Label  preBell1FreqLabel, preBell1GainLabel;
    juce::Label  preBell2FreqLabel, preBell2GainLabel;
    juce::Label  preBell3FreqLabel, preBell3GainLabel;

    // ── Saturator ─────────────────────────────────────────────────────────
    juce::Slider satDriveSlider, satBlendSlider;
    juce::Label  satDriveLabel,  satBlendLabel;

    // ── Reverb ────────────────────────────────────────────────────────────
    juce::Slider roomSizeSlider, dampingSlider, preDelaySlider, wetSlider;
    juce::Label  roomSizeLabel,  dampingLabel,  preDelayLabel,  wetLabel;

    // ── Modulation ────────────────────────────────────────────────────────
    juce::Slider modRateSlider, modDepthSlider;
    juce::Label  modRateLabel,  modDepthLabel;

    // ── Compressor ────────────────────────────────────────────────────────
    juce::Slider compThreshSlider, compRatioSlider;
    juce::Slider compAttackSlider, compReleaseSlider;
    juce::Label  compThreshLabel,  compRatioLabel;
    juce::Label  compAttackLabel,  compReleaseLabel;

    // ── Post-EQ toggle ────────────────────────────────────────────────────
    juce::ToggleButton postEQToggle;
    juce::Label        postEQLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RivorbEditor)
};