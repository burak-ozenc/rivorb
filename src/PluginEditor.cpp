#include "PluginEditor.h"

// ─────────────────────────────────────────────
// Constants
// ─────────────────────────────────────────────

static constexpr int kKnobW   = 56;
static constexpr int kKnobH   = 56;
static constexpr int kLabelH  = 14;
static constexpr int kCellH   = kKnobH + kLabelH + 2;
static constexpr int kGap     = 6;
static constexpr int kSectionPad = 10;

static const juce::Colour kBg         { 0xff14141a };
static const juce::Colour kPanel      { 0xff1c1c24 };
static const juce::Colour kAccent     { 0xffe05e3a };
static const juce::Colour kDivider    { 0xff2a2a34 };
static const juce::Colour kTextDim    { 0xff666672 };
static const juce::Colour kTextBright { 0xffcccccc };

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────

RivorbEditor::RivorbEditor (RivorbProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&laf);

    // ── Pre-EQ ────────────────────────────────────────────────────────────
    setupKnob (preHPSlider,       preHPLabel,       "HP",    20,    500,   1,   processor.preHPFreq->get());
    setupKnob (preBell1FreqSlider,preBell1FreqLabel,"B1 FQ", 100,  2000,   1,   processor.preBell1Freq->get());
    setupKnob (preBell1GainSlider,preBell1GainLabel,"B1 GN", -12,    12,  0.1,  processor.preBell1Gain->get());
    setupKnob (preBell2FreqSlider,preBell2FreqLabel,"B2 FQ", 500,  5000,   1,   processor.preBell2Freq->get());
    setupKnob (preBell2GainSlider,preBell2GainLabel,"B2 GN", -12,    12,  0.1,  processor.preBell2Gain->get());
    setupKnob (preBell3FreqSlider,preBell3FreqLabel,"B3 FQ", 2000,16000,   1,   processor.preBell3Freq->get());
    setupKnob (preBell3GainSlider,preBell3GainLabel,"B3 GN", -12,    12,  0.1,  processor.preBell3Gain->get());

    preHPSlider      .onValueChange = [&]{ *processor.preHPFreq    = (float) preHPSlider.getValue(); };
    preBell1FreqSlider.onValueChange = [&]{ *processor.preBell1Freq = (float) preBell1FreqSlider.getValue(); };
    preBell1GainSlider.onValueChange = [&]{ *processor.preBell1Gain = (float) preBell1GainSlider.getValue(); };
    preBell2FreqSlider.onValueChange = [&]{ *processor.preBell2Freq = (float) preBell2FreqSlider.getValue(); };
    preBell2GainSlider.onValueChange = [&]{ *processor.preBell2Gain = (float) preBell2GainSlider.getValue(); };
    preBell3FreqSlider.onValueChange = [&]{ *processor.preBell3Freq = (float) preBell3FreqSlider.getValue(); };
    preBell3GainSlider.onValueChange = [&]{ *processor.preBell3Gain = (float) preBell3GainSlider.getValue(); };

    // ── Saturator ─────────────────────────────────────────────────────────
    setupKnob (satDriveSlider, satDriveLabel, "DRIVE", 0, 1, 0.01, processor.satDrive->get());
    setupKnob (satBlendSlider, satBlendLabel, "BLEND", 0, 1, 0.01, processor.satBlend->get());

    satDriveSlider.onValueChange = [&]{ *processor.satDrive = (float) satDriveSlider.getValue(); };
    satBlendSlider.onValueChange = [&]{ *processor.satBlend = (float) satBlendSlider.getValue(); };

    // ── Reverb ────────────────────────────────────────────────────────────
    setupKnob (roomSizeSlider, roomSizeLabel, "SIZE",  0.1,  1.0,   0.01, processor.roomSize->get());
    setupKnob (dampingSlider,  dampingLabel,  "DAMP",  0.0,  1.0,   0.01, processor.damping->get());
    setupKnob (preDelaySlider, preDelayLabel, "PREDLY",0.0,  100.0, 0.5,  processor.preDelayMs->get());
    setupKnob (wetSlider,      wetLabel,      "WET",   0.0,  1.0,   0.01, processor.wetLevel->get());

    roomSizeSlider.onValueChange = [&]{ *processor.roomSize   = (float) roomSizeSlider.getValue(); };
    dampingSlider .onValueChange = [&]{ *processor.damping    = (float) dampingSlider.getValue();  };
    preDelaySlider.onValueChange = [&]{ *processor.preDelayMs = (float) preDelaySlider.getValue(); };
    wetSlider     .onValueChange = [&]{ *processor.wetLevel   = (float) wetSlider.getValue();      };

    // ── Modulation ────────────────────────────────────────────────────────
    setupKnob (modRateSlider,  modRateLabel,  "RATE",  0.01, 5.0,  0.01, processor.modRate->get());
    setupKnob (modDepthSlider, modDepthLabel, "DEPTH", 0.0,  1.0,  0.01, processor.modDepth->get());

    modRateSlider .onValueChange = [&]{ *processor.modRate  = (float) modRateSlider.getValue();  };
    modDepthSlider.onValueChange = [&]{ *processor.modDepth = (float) modDepthSlider.getValue(); };

    // ── Compressor ────────────────────────────────────────────────────────
    setupKnob (compThreshSlider,  compThreshLabel,  "THRESH", -60,  0,    0.5,  processor.compThreshold->get());
    setupKnob (compRatioSlider,   compRatioLabel,   "RATIO",   1,   20,   0.1,  processor.compRatio->get());
    setupKnob (compAttackSlider,  compAttackLabel,  "ATTACK",  1,   200,  0.5,  processor.compAttack->get());
    setupKnob (compReleaseSlider, compReleaseLabel, "RELEASE", 10,  2000, 1.0,  processor.compRelease->get());

    compThreshSlider .onValueChange = [&]{ *processor.compThreshold = (float) compThreshSlider.getValue();  };
    compRatioSlider  .onValueChange = [&]{ *processor.compRatio     = (float) compRatioSlider.getValue();   };
    compAttackSlider .onValueChange = [&]{ *processor.compAttack    = (float) compAttackSlider.getValue();  };
    compReleaseSlider.onValueChange = [&]{ *processor.compRelease   = (float) compReleaseSlider.getValue(); };

    // ── Post-EQ toggle ────────────────────────────────────────────────────
    postEQToggle.setButtonText ("POST EQ");
    postEQToggle.setToggleState (processor.postEQEnabled->get(), juce::dontSendNotification);
    postEQToggle.onStateChange = [&]{ *processor.postEQEnabled = postEQToggle.getToggleState(); };
    postEQToggle.setColour (juce::ToggleButton::textColourId, kTextDim);
    addAndMakeVisible (postEQToggle);

    postEQLabel.setText ("AUTO SPECTRAL EQ ON TAIL", juce::dontSendNotification);
    postEQLabel.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::plain));
    postEQLabel.setColour (juce::Label::textColourId, kTextDim);
    addAndMakeVisible (postEQLabel);

    setSize (688, 290);
}

RivorbEditor::~RivorbEditor()
{
    setLookAndFeel (nullptr);
}

// ─────────────────────────────────────────────
// setupKnob
// ─────────────────────────────────────────────

void RivorbEditor::setupKnob (juce::Slider& s, juce::Label& l,
                               const juce::String& text,
                               double lo, double hi, double step, double val)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, kKnobW, kLabelH - 1);
    s.setRange (lo, hi, step);
    s.setValue (val, juce::dontSendNotification);
    s.setColour (juce::Slider::textBoxTextColourId, kTextDim);
    addAndMakeVisible (s);

    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::plain));
    l.setColour (juce::Label::textColourId, kTextDim);
    addAndMakeVisible (l);
}

// ─────────────────────────────────────────────
// paint
// ─────────────────────────────────────────────

void RivorbEditor::drawSectionHeader (juce::Graphics& g,
                                       const juce::String& text,
                                       int x, int y, int w)
{
    g.setColour (kAccent.withAlpha (0.85f));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::plain));
    g.drawText (text, x + 2, y, w, 12, juce::Justification::left);

    g.setColour (kDivider);
    g.drawHorizontalLine (y + 13, (float) x, (float) (x + w));
}

void RivorbEditor::paint (juce::Graphics& g)
{
    g.fillAll (kBg);

    // Top bar
    g.setColour (kPanel);
    g.fillRect (0, 0, getWidth(), 28);
    g.setColour (kAccent);
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::bold));
    g.drawText ("RIVORB", 14, 0, 120, 28, juce::Justification::centredLeft);
    g.setColour (kTextDim);
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::plain));
    g.drawText ("SPECTRAL ADAPTIVE REVERB", 100, 0, 240, 28, juce::Justification::centredLeft);

    // Bottom strip
    g.setColour (kPanel);
    g.fillRect (0, getHeight() - 24, getWidth(), 24);
    g.setColour (kDivider);
    g.drawHorizontalLine (getHeight() - 24, 0.0f, (float) getWidth());

    auto drawPanel = [&](int x, int y, int w, int h)
    {
        g.setColour (kPanel);
        g.fillRoundedRectangle ((float)x, (float)y, (float)w, (float)h, 4.0f);
        g.setColour (kDivider);
        g.drawRoundedRectangle ((float)x, (float)y, (float)w, (float)h, 4.0f, 0.8f);
    };

    // Row 1 panels
    drawPanel (  6,  32, 462, 106);   // Pre-EQ
    drawPanel (470,  32, 136, 106);   // Saturator

    // Row 2 panels — height increased by 12 to accommodate header + same knob clearance
    drawPanel (  6, 140, 278, 118);   // Reverb
    drawPanel (288, 140, 124, 118);   // Modulation
    drawPanel (414, 140, 264, 118);   // Compressor

    // Section headers
    drawSectionHeader (g, "PRE-EQ",     10,  36, 200);
    drawSectionHeader (g, "SATURATOR", 474,  36, 120);
    drawSectionHeader (g, "REVERB",     10, 144, 80);
    drawSectionHeader (g, "MOD",       292, 144, 60);
    drawSectionHeader (g, "COMP",      418, 144, 108);
}

// ─────────────────────────────────────────────
// resized
// ─────────────────────────────────────────────

void RivorbEditor::resized()
{
    const int kw    = kKnobW;
    const int gap   = kGap;
    const int row1y = 52;
    const int row2y = 160;   // was 148 — now matches row1's 20px clearance from panel top

    auto placeKnob = [&](juce::Slider& s, juce::Label& l, int x, int y)
    {
        s.setBounds (x, y,          kw, kKnobH);
        l.setBounds (x, y + kKnobH, kw, kLabelH);
    };

    // ── Row 1: Pre-EQ ─────────────────────────────────────────────────────
    int x = 12;
    placeKnob (preHPSlider,        preHPLabel,        x, row1y); x += kw + gap;
    placeKnob (preBell1FreqSlider, preBell1FreqLabel, x, row1y); x += kw + gap;
    placeKnob (preBell1GainSlider, preBell1GainLabel, x, row1y); x += kw + gap + 8;
    placeKnob (preBell2FreqSlider, preBell2FreqLabel, x, row1y); x += kw + gap;
    placeKnob (preBell2GainSlider, preBell2GainLabel, x, row1y); x += kw + gap + 8;
    placeKnob (preBell3FreqSlider, preBell3FreqLabel, x, row1y); x += kw + gap;
    placeKnob (preBell3GainSlider, preBell3GainLabel, x, row1y);

    // ── Row 1: Saturator ──────────────────────────────────────────────────
    x = 476;
    placeKnob (satDriveSlider, satDriveLabel, x, row1y); x += kw + gap;
    placeKnob (satBlendSlider, satBlendLabel, x, row1y);

    // ── Row 2: Reverb ─────────────────────────────────────────────────────
    x = 12;
    placeKnob (roomSizeSlider, roomSizeLabel, x, row2y); x += kw + gap;
    placeKnob (dampingSlider,  dampingLabel,  x, row2y); x += kw + gap;
    placeKnob (preDelaySlider, preDelayLabel, x, row2y); x += kw + gap;
    placeKnob (wetSlider,      wetLabel,      x, row2y);

    // ── Row 2: Modulation ─────────────────────────────────────────────────
    x = 294;
    placeKnob (modRateSlider,  modRateLabel,  x, row2y); x += kw + gap;
    placeKnob (modDepthSlider, modDepthLabel, x, row2y);

    // ── Row 2: Compressor ─────────────────────────────────────────────────
    x = 420;
    placeKnob (compThreshSlider,  compThreshLabel,  x, row2y); x += kw + gap;
    placeKnob (compRatioSlider,   compRatioLabel,   x, row2y); x += kw + gap;
    placeKnob (compAttackSlider,  compAttackLabel,  x, row2y); x += kw + gap;
    placeKnob (compReleaseSlider, compReleaseLabel, x, row2y);

    // ── Post-EQ toggle ────────────────────────────────────────────────────
    postEQToggle.setBounds (10,  getHeight() - 20, 90,  16);
    postEQLabel .setBounds (106, getHeight() - 20, 260, 16);
}