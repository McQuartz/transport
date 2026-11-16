#include "HeaderView.h"

namespace tp
{
using juce::Colour;
using juce::Rectangle;
using juce::Point;

//==============================================================================
class HeaderView::IconButton final : public juce::Component, public juce::SettableTooltipClient, public Animated
{
public:
    enum class Kind { prev, play, next, repeat, shuffle };

    explicit IconButton (Kind k) : kind (k) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }

    std::function<void()> onClick;
    void setPlaying (bool p) { morph.target = p ? 1.0f : 0.0f; }
    void setRepeat (int m)   { repeatMode = m; morph.target = m > 0 ? 1.0f : 0.0f; repaint(); }
    void setToggled (bool on) { morph.target = on ? 1.0f : 0.0f; repaint(); }

    void animate (float dt) override
    {
        const bool a = state.tick (dt);
        const bool b = morph.tick (dt, 16.0f);
        if (a || b) repaint();
    }

    void mouseEnter (const juce::MouseEvent&) override { state.hover.target = 1.0f; }
    void mouseExit (const juce::MouseEvent&) override  { state.hover.target = 0.0f; state.press.target = 0.0f; }
    void mouseDown (const juce::MouseEvent&) override  { state.press.target = 1.0f; }
    void mouseUp (const juce::MouseEvent& e) override
    {
        state.press.target = 0.0f;
        if (getLocalBounds().contains (e.getPosition()) && onClick) onClick();
    }

    void paint (juce::Graphics& g) override
    {
        // glyphs are designed in a 34x34 (prev/next) or 46x50 (play) box and scaled to fit
        const float boxW = kind == Kind::play ? 46.0f : 34.0f, boxH = kind == Kind::play ? 50.0f : 34.0f;
        const auto b = getLocalBounds().toFloat();
        const float fit = juce::jmin (b.getWidth() / (boxW + 8.0f), b.getHeight() / (boxH + 8.0f));
        const float scale = fit * (1.0f + 0.06f * state.hover.current - 0.08f * state.press.current);
        const auto c = b.getCentre();
        g.addTransform (juce::AffineTransform::translation (-boxW * 0.5f, -boxH * 0.5f).scaled (scale).translated (c.x, c.y));

        if (kind == Kind::shuffle)
        {
            // two crossing arrows; accent when on
            const auto base = theme::icon.interpolatedWith (theme::iconBright, state.hover.current * 0.75f);
            g.setColour (base.interpolatedWith (theme::accent, morph.current));
            juce::Path lines;
            lines.startNewSubPath (4, 9);  lines.cubicTo (14, 9, 18, 25, 25, 25);
            lines.startNewSubPath (4, 25); lines.cubicTo (10, 25, 12, 20, 14, 17.5f);
            lines.startNewSubPath (19, 12); lines.cubicTo (21, 10, 22, 9, 25, 9);
            g.strokePath (lines, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            juce::Path heads;
            heads.addTriangle (24, 3.5f, 31, 9, 24, 14.5f);
            heads.addTriangle (24, 19.5f, 31, 25, 24, 30.5f);
            g.fillPath (heads);
            return;
        }
        if (kind == Kind::repeat)
        {
            // two arrows chasing each other round a loop; accent when on, a "1" inside for repeat one
            const auto base = theme::icon.interpolatedWith (theme::iconBright, state.hover.current * 0.75f);
            g.setColour (base.interpolatedWith (theme::accent, morph.current));
            juce::Path loop;
            loop.startNewSubPath (6, 19); loop.lineTo (6, 14); loop.quadraticTo (6, 9, 11, 9); loop.lineTo (24, 9);
            loop.startNewSubPath (28, 15); loop.lineTo (28, 20); loop.quadraticTo (28, 25, 23, 25); loop.lineTo (10, 25);
            g.strokePath (loop, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            juce::Path heads;
            heads.addTriangle (23, 3.5f, 30, 9, 23, 14.5f);
            heads.addTriangle (11, 19.5f, 4, 25, 11, 30.5f);
            g.fillPath (heads);
            if (repeatMode == 2)
            {
                g.setFont (theme::font (9.0f, true));
                g.drawText ("1", juce::Rectangle<float> (11, 11, 12, 12), juce::Justification::centred, false);
            }
            return;
        }

        g.setColour (theme::icon.interpolatedWith (theme::iconBright, juce::jmax (state.hover.current * 0.75f, kind == Kind::play ? morph.current * 0.7f : 0.0f)));
        if (kind == Kind::play)
        {
            const int st = tp::style();
            const auto ring = juce::Rectangle<float> (-2.0f, 0.0f, 50.0f, 50.0f);
            if (false) {}
            else if (st == 3 && tp::headerVariant() == 0)
            {
                g.drawRoundedRectangle (ring.reduced (2.0f), 8.0f, 2.0f);
                g.addTransform (juce::AffineTransform::scale (0.6f, 0.6f, 23.0f, 25.0f));
            }
            if (tp::headerVariant() == 3)
            {
                g.drawEllipse (ring.reduced (1.0f), 1.5f);
                g.addTransform (juce::AffineTransform::scale (0.5f, 0.5f, 23.0f, 25.0f).translated (morph.current > 0.5f ? 0.0f : 2.0f, 0.0f));
            }
            else if (tp::headerVariant() == 4)
            {
                g.setColour (tp::C (0xff626262).interpolatedWith (tp::C (0xff6e6e6e), state.hover.current));
                g.fillRoundedRectangle (ring, 9.0f);
                g.setColour (juce::Colours::white);
                g.addTransform (juce::AffineTransform::scale (0.5f, 0.5f, 23.0f, 25.0f).translated (morph.current > 0.5f ? 0.0f : 2.0f, 0.0f));
            }
            else if (st == 2)
            {
                g.drawEllipse (ring.reduced (1.0f), 2.0f);
                g.addTransform (juce::AffineTransform::scale (0.62f, 0.62f, 23.0f, 25.0f));
            }
            else if (st == 4)
            {
                g.setColour (tp::C (0xfff2f2f2).interpolatedWith (juce::Colours::white, state.hover.current));
                g.fillEllipse (ring);
                g.setColour (tp::C (0xff1b1b1b));
                g.addTransform (juce::AffineTransform::scale (0.55f, 0.55f, 23.0f, 25.0f).translated (morph.current > 0.5f ? 0.0f : 2.0f, 0.0f));
            }
        }
        juce::Path p;
        if (kind == Kind::prev)      { p.addRectangle (3, 5, 4, 24); p.addTriangle (31, 5, 9, 17, 31, 29); }
        else if (kind == Kind::next) { p.addTriangle (3, 5, 25, 17, 3, 29); p.addRectangle (27, 5, 4, 24); }
        else
        {
            const float t = easeInOutCubic (morph.current);
            auto lerp = [t] (Point<float> a, Point<float> bb) { return a + (bb - a) * t; };
            const Point<float> playL[] = { { 4, 2 }, { 24, 13.5f }, { 24, 36.5f }, { 4, 48 } };
            const Point<float> pauseL[] = { { 8, 4 }, { 18, 4 }, { 18, 46 }, { 8, 46 } };
            const Point<float> playR[] = { { 24, 13.5f }, { 44, 25 }, { 44, 25 }, { 24, 36.5f } };
            const Point<float> pauseR[] = { { 28, 4 }, { 38, 4 }, { 38, 46 }, { 28, 46 } };
            for (auto [a, bb] : { std::pair { playL, pauseL }, std::pair { playR, pauseR } })
            {
                p.startNewSubPath (lerp (a[0], bb[0]));
                for (int i = 1; i < 4; ++i) p.lineTo (lerp (a[i], bb[i]));
                p.closeSubPath();
            }
        }
        g.fillPath (p);
    }

private:
    Kind kind;
    HoverState state;
    Smoothed morph;
    int repeatMode = 0;
};

//==============================================================================
class HeaderView::VolumeSlider final : public juce::Component, public Animated
{
public:
    VolumeSlider() { setMouseCursor (juce::MouseCursor::PointingHandCursor); shown.snap (value); setTitle ("Volume"); }

    std::function<void (float)> onChange;
    bool iconOnly = false;

    void setValue (float v, bool notify)
    {
        value = juce::jlimit (0.0f, 1.0f, v);
        shown.target = value;
        if (notify && onChange) onChange (value);
    }

    void animate (float dt) override
    {
        const bool a = shown.tick (dt, 22.0f), b = hover.tick (dt, 16.0f), c = drag.tick (dt, 20.0f);
        if (a || b || c) repaint();
    }

    void mouseEnter (const juce::MouseEvent&) override { hover.target = 1.0f; }
    void mouseExit (const juce::MouseEvent&) override  { hover.target = 0.0f; }
    void mouseDown (const juce::MouseEvent& e) override
    {
        drag.target = 1.0f;
        if (iconOnly || e.x < trackArea().getX() - 4) { toggleMute(); return; }
        setFromX ((float) e.x);
    }
    void mouseDrag (const juce::MouseEvent& e) override { if (! iconOnly && e.getMouseDownX() >= trackArea().getX() - 4) setFromX ((float) e.x); }
    void mouseUp (const juce::MouseEvent&) override    { drag.target = 0.0f; }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w) override { setValue (value + (w.deltaY + w.deltaX) * 0.25f, true); }

    void paint (juce::Graphics& g) override
    {
        const float cy = trackY();

        juce::Path spk;
        spk.startNewSubPath (2, cy - 3.5f); spk.lineTo (6, cy - 3.5f); spk.lineTo (12, cy - 8);
        spk.lineTo (12, cy + 8); spk.lineTo (6, cy + 3.5f); spk.lineTo (2, cy + 3.5f); spk.closeSubPath();
        g.setColour (theme::icon.interpolatedWith (theme::iconBright, hover.current * 0.5f));
        g.fillPath (spk);
        if (value <= 0.0f)
        {
            // muted: a cross where the waves were
            g.setColour (theme::icon.interpolatedWith (theme::iconBright, hover.current * 0.5f));
            g.drawLine (15.5f, cy - 4.0f, 23.5f, cy + 4.0f, 1.6f);
            g.drawLine (23.5f, cy - 4.0f, 15.5f, cy + 4.0f, 1.6f);
        }
        else for (int i = 0; i < 2; ++i)
        {
            const float a = juce::jlimit (0.0f, 1.0f, shown.current * 2.5f - (float) i * 0.9f);
            if (a <= 0.0f) continue;
            juce::Path wave;
            const float r = 4.5f + (float) i * 4.5f;
            wave.addCentredArc (12, cy, r, r, 0, 0.75f, 2.39f, true);
            g.setColour (theme::icon.withAlpha (a));
            g.strokePath (wave, juce::PathStrokeType (1.6f));
        }
        if (iconOnly) return;

        const int hv = tp::headerVariant();
        const auto track = trackArea();
        if (hv == 3 || hv == 4)
        {
            // readout always visible on the right
            const float db = value <= 0.0f ? -100.0f : 40.0f * std::log10 (value);
            g.setColour (theme::label);
            g.setFont (theme::mono (10.5f, false));
            g.drawText (value <= 0.0f ? utf8 ("\xe2\x88\x92\xe2\x88\x9e dB") : juce::String (db, 1) + " dB",
                        Rectangle<float> (track.getRight() + 8.0f, cy - 8.0f, 56.0f, 16.0f), juce::Justification::centredLeft, false);
            const float th = hv == 3 ? 1.5f : 4.0f;
            const auto line = Rectangle<float> (track.getX(), cy - th * 0.5f, track.getWidth(), th);
            g.setColour (tp::C (0xff595959));
            g.fillRect (line);
            g.setColour (tp::C (0xffdedede));
            g.fillRect (line.withWidth (line.getWidth() * shown.current));
            const float x = line.getX() + line.getWidth() * shown.current;
            if (hv == 4)
            {
                g.setColour (tp::C (0xff6a6a6a));
                for (int i = 0; i <= 10; ++i)
                {
                    const float tx = line.getX() + line.getWidth() * (float) i / 10.0f;
                    g.fillRect (tx - 0.5f, cy + 7.0f, 1.0f, i % 5 == 0 ? 6.0f : 3.0f);
                }
                const auto cap = Rectangle<float> (10.0f, 22.0f).withCentre ({ x, cy });
                g.setColour (tp::C (0xffcfcfcf).interpolatedWith (juce::Colours::white, hover.current));
                g.fillRoundedRectangle (cap, 2.0f);
                g.setColour (tp::C (0xff2a2a2a));
                g.fillRect (cap.withSizeKeepingCentre (cap.getWidth(), 1.5f));
            }
            else
            {
                g.setColour (juce::Colours::white);
                g.fillRect (Rectangle<float> (3.0f, 18.0f).withCentre ({ x, cy }));
            }
            return;
        }
        const float th = 3.0f + hover.current * 1.0f;
        const auto line = Rectangle<float> (track.getX(), cy - th * 0.5f, track.getWidth(), th);
        g.setColour (tp::C (0xff595959));
        g.fillRoundedRectangle (line, th * 0.5f);
        g.setColour (tp::C (0xffdedede));
        g.fillRoundedRectangle (line.withWidth (line.getWidth() * shown.current), th * 0.5f);

        const float r = 5.5f + hover.current * 1.5f + drag.current;
        const Point<float> thumb (line.getX() + line.getWidth() * shown.current, cy);
        if (drag.current > 0.01f)
        {
            g.setColour (juce::Colours::white.withAlpha (0.12f * drag.current));
            g.fillEllipse (Rectangle<float> (r * 3.2f, r * 3.2f).withCentre (thumb));
        }
        g.setColour (juce::Colours::white);
        g.fillEllipse (Rectangle<float> (r * 2, r * 2).withCentre (thumb));

        // value label sits fully inside the component, above the track
        const float a = juce::jmax (hover.current, drag.current);
        if (a > 0.01f)
        {
            g.setColour (theme::label.withAlpha (a));
            g.setFont (theme::mono (10.0f, false));
            g.drawText (juce::String (juce::roundToInt (value * 100.0f)) + "%",
                        Rectangle<float> (juce::jlimit (0.0f, (float) getWidth() - 40.0f, thumb.x - 20.0f), 0.0f, 40.0f, cy - 9.0f),
                        juce::Justification::centredBottom, false);
        }
    }

private:
    float trackY() const { return (float) getHeight() * 0.5f; }   // centred, so it lines up with the buttons
    Rectangle<float> trackArea() const { return getLocalBounds().toFloat().withTrimmedLeft (28).withTrimmedRight (tp::headerVariant() >= 3 ? 70 : 8); }
    void setFromX (float x) { const auto t = trackArea(); setValue ((x - t.getX()) / t.getWidth(), true); }
    void toggleMute()
    {
        if (value > 0.0f) { unmuted = value; setValue (0.0f, true); }
        else setValue (unmuted > 0.0f ? unmuted : 0.6f, true);
    }

    float value = 0.8f, unmuted = 0.8f;
    Smoothed shown, hover, drag;
};

//==============================================================================
class HeaderView::QuickButton final : public juce::Component, public juce::SettableTooltipClient, public Animated
{
public:
    QuickButton() { setMouseCursor (juce::MouseCursor::PointingHandCursor); }

    QuickAction action = QuickAction::playlist;
    std::function<void()> onClick, onRightClick;

    void setActive (bool a) { active.target = a ? 1.0f : 0.0f; }
    void setAction (QuickAction a) { action = a; setTooltip (utf8 (infoFor (a).name) + utf8 (" \xe2\x80\x94 right-click to change")); repaint(); }

    void animate (float dt) override
    {
        const bool a = state.tick (dt), b = active.tick (dt, 14.0f);
        if (a || b) repaint();
    }

    void mouseEnter (const juce::MouseEvent&) override { state.hover.target = 1.0f; }
    void mouseExit (const juce::MouseEvent&) override  { state.hover.target = 0.0f; state.press.target = 0.0f; }
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu()) { if (onRightClick) onRightClick(); return; }
        state.press.target = 1.0f;
    }
    void mouseUp (const juce::MouseEvent& e) override
    {
        if (state.press.target > 0.5f && getLocalBounds().contains (e.getPosition()) && onClick) onClick();
        state.press.target = 0.0f;
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        const float s = 1.0f - 0.04f * state.press.current;
        r = r.withSizeKeepingCentre (r.getWidth() * s, r.getHeight() * s);

        g.setColour (tp::C (0xff424242).interpolatedWith (tp::C (0xff4d4d4d), state.hover.current));
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (juce::Colours::white.withAlpha (0.05f + 0.04f * state.hover.current));
        g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);
        if (active.current > 0.01f)
        {
            g.setColour (theme::accent.withAlpha (active.current));
            g.fillRoundedRectangle (Rectangle<float> (r.getWidth() * 0.4f, 2.0f).withCentre ({ r.getCentreX(), r.getBottom() - 4.0f }), 1.0f);
        }

        // icons are drawn for a ~60px tile; scale them to this tile
        const float k = juce::jmin (r.getWidth(), r.getHeight()) / 62.0f;
        const auto iconColour = tp::C (0xff9e9e9e).interpolatedWith (juce::Colours::white, state.hover.current * 0.5f)
                                                   .interpolatedWith (theme::accent, active.current);
        juce::Graphics::ScopedSaveState save (g);
        g.addTransform (juce::AffineTransform::scale (k * 1.05f, k * 1.05f, r.getCentreX(), r.getCentreY()));
        drawQuickActionIcon (g, action, r, iconColour);
    }

private:
    HoverState state;
    Smoothed active;
};

//==============================================================================
HeaderView::HeaderView()
{
    prevButton = std::make_unique<IconButton> (IconButton::Kind::prev);
    playButton = std::make_unique<IconButton> (IconButton::Kind::play);
    nextButton = std::make_unique<IconButton> (IconButton::Kind::next);
    repeatButton = std::make_unique<IconButton> (IconButton::Kind::repeat);
    repeatButton->onClick = [this] { if (onRepeat) onRepeat(); };
    repeatButton->setTitle ("Repeat");
    setRepeat (0);
    shuffleButton = std::make_unique<IconButton> (IconButton::Kind::shuffle);
    shuffleButton->onClick = [this] { if (onShuffle) onShuffle(); };
    shuffleButton->setTitle ("Shuffle");
    setShuffle (false);
    prevButton->onClick = [this] { if (onPrev) onPrev(); };
    playButton->onClick = [this] { if (onPlayPause) onPlayPause(); };
    nextButton->onClick = [this] { if (onNext) onNext(); };
    prevButton->setTitle ("Previous track");
    playButton->setTitle ("Play / pause");
    nextButton->setTitle ("Next track");

    volume = std::make_unique<VolumeSlider>();
    volume->onChange = [this] (float v) { if (onVolumeChanged) onVolumeChanged (v); };

    for (auto* c : std::initializer_list<juce::Component*> { prevButton.get(), playButton.get(), nextButton.get(), repeatButton.get(), shuffleButton.get(), volume.get() })
        addAndMakeVisible (c);

    artFade.snap (1.0f);
    textFade.snap (1.0f);
    title = "Drop a track here";
    artist = "or click the cover to open one";
}

HeaderView::~HeaderView() = default;

juce::Rectangle<int> HeaderView::artArea() const
{
    const int size = getHeight() - 2 * kPad;
    return { kPad, kPad, size, size };
}

void HeaderView::setTrack (const juce::Image& newCover, const juce::String& newTitle, const juce::String& newArtist, const juce::StringArray& newMeta)
{
    previousCover = cover;
    cover = newCover;
    artFade.snap (0.0f);
    artFade.target = 1.0f;

    previousTitle = title;
    previousArtist = artist;
    previousMeta = meta;
    title = newTitle;
    artist = newArtist;
    meta = newMeta;
    textFade.snap (0.0f);
    textFade.target = 1.0f;
    repaint();
}

void HeaderView::setCover (const juce::Image& newCover)
{
    previousCover = cover;
    cover = newCover;
    artFade.snap (0.0f);
    artFade.target = 1.0f;
    repaint();
}

void HeaderView::setPlaying (bool p)
{
    playing = p;
    playButton->setPlaying (p);
}

void HeaderView::setRepeat (int mode)
{
    repeatButton->setRepeat (mode);
    repeatButton->setTooltip (mode == 0 ? "Repeat: off" : (mode == 1 ? "Repeat: all tracks in the folder" : "Repeat: this track"));
}

void HeaderView::setShuffle (bool on)
{
    shuffleButton->setToggled (on);
    shuffleButton->setTooltip (on ? "Shuffle: on" : "Shuffle: off");
}

void HeaderView::setVolume (float v) { volume->setValue (v, false); }

void HeaderView::setTimeLine (const juce::String& line)
{
    if (line != timeLine) { timeLine = line; repaint (textArea().expanded (0, 10)); }
}

void HeaderView::setQuickActions (const std::vector<QuickAction>& actions, int columns)
{
    quickColumns = columns;
    while (quick.size() > actions.size()) quick.pop_back();
    while (quick.size() < actions.size())
    {
        auto b = std::make_unique<QuickButton>();
        const int index = (int) quick.size();
        b->onClick = [this, index] { if (onQuickAction) onQuickAction (index, quick[(size_t) index]->action); };
        b->onRightClick = [this, index] { if (onQuickRightClick) onQuickRightClick (index); };
        addAndMakeVisible (*b);
        quick.push_back (std::move (b));
    }
    for (size_t i = 0; i < actions.size(); ++i)
        quick[i]->setAction (actions[i]);
    resized();
}

void HeaderView::setQuickActive (QuickAction a, bool active)
{
    for (auto& q : quick)
        if (q->action == a)
            q->setActive (active);
}

void HeaderView::animate (float dt)
{
    for (auto* a : std::initializer_list<Animated*> { prevButton.get(), playButton.get(), nextButton.get(), repeatButton.get(), shuffleButton.get(), volume.get() })
        a->animate (dt);
    for (auto& q : quick)
        q->animate (dt);

    bool dirtyArt = artFade.tick (dt, 7.0f);
    if (! cover.isValid() && playing)
    {
        vinylAngle += dt * 3.5f;
        dirtyArt = true;
    }
    if (dirtyArt) repaint (artArea());
    if (textFade.tick (dt, 9.0f)) repaint (textArea().expanded (0, 10));   // the text slides 8 px out of its box
}

void HeaderView::paintArt (juce::Graphics& g, Rectangle<float> r, const juce::Image& img, float alpha)
{
    if (alpha <= 0.0f) return;
    g.setOpacity (alpha);
    if (img.isValid())
    {
        g.drawImage (img, r, juce::RectanglePlacement::fillDestination);
        return;
    }

    const float s = r.getWidth() / 136.0f;
    g.setColour (tp::C (0xff262626).withMultipliedAlpha (alpha));
    g.fillRect (r);
    const auto c = r.getCentre();
    g.setColour (tp::C (0xff1c1c1c).withMultipliedAlpha (alpha));
    g.fillEllipse (Rectangle<float> (112 * s, 112 * s).withCentre (c));
    g.setColour (tp::C (0xff2f2f2f).withMultipliedAlpha (alpha));
    for (float rad : { 48.0f, 41.0f, 34.0f, 27.0f })
        g.drawEllipse (Rectangle<float> (rad * 2 * s, rad * 2 * s).withCentre (c), 1.0f);

    juce::Path sheen;
    sheen.addPieSegment (Rectangle<float> (108 * s, 108 * s).withCentre (c), vinylAngle, vinylAngle + 0.5f, 0.32f);
    sheen.addPieSegment (Rectangle<float> (108 * s, 108 * s).withCentre (c), vinylAngle + juce::MathConstants<float>::pi,
                         vinylAngle + juce::MathConstants<float>::pi + 0.5f, 0.32f);
    g.setColour (juce::Colours::white.withAlpha (0.05f * alpha));
    g.fillPath (sheen);

    g.setColour (theme::accent.withMultipliedAlpha (alpha));
    g.fillEllipse (Rectangle<float> (34 * s, 34 * s).withCentre (c));
    g.setColour (tp::C (0xff1c1c1c).withMultipliedAlpha (alpha));
    g.fillEllipse (Rectangle<float> (5 * s, 5 * s).withCentre (c));
}

void HeaderView::paint (juce::Graphics& g)
{
    g.fillAll (theme::header);

    const auto art = artArea().toFloat();
    {
        juce::Graphics::ScopedSaveState s (g);
        g.reduceClipRegion (artArea());
        paintArt (g, art, previousCover, 1.0f - artFade.current);
        paintArt (g, art, cover, artFade.current);
        g.setOpacity (1.0f);
    }

    const auto text = textArea().toFloat();
    if (text.getWidth() < 40.0f) return;
    const bool roomy = text.getHeight() >= 90.0f;
    const float titlePx = roomy ? 17.0f : 15.0f;

    auto drawText = [&] (const juce::String& t, const juce::String& a, const juce::StringArray& lines, float alpha, float yOffset, bool withTime)
    {
        if (alpha <= 0.01f) return;
        auto r = text.translated (0, yOffset);
        g.setColour (theme::text.withAlpha (alpha));
        g.setFont (theme::font (titlePx, true));
        g.drawText (t, r.withHeight (titlePx * 1.45f), juce::Justification::topLeft, true);
        if (a.isNotEmpty())
        {
            g.setColour (tp::C (0xffc8c8c8).withAlpha (alpha));
            g.setFont (theme::font (titlePx * 0.78f));
            g.drawText (a, r.withTrimmedTop (titlePx * 1.45f).withHeight (titlePx * 1.2f), juce::Justification::topLeft, true);
        }

        g.setFont (tp::studio() || tp::style() == 2 ? theme::mono (10.5f, false) : theme::font (11.0f));
        g.setColour (tp::C (0xffb8b8b8).withAlpha (alpha));
        juce::StringArray all;
        if (roomy)
        {
            all = lines;
            if (withTime && timeLine.isNotEmpty()) all.add (timeLine);
        }
        else
        {
            // the time comes first: when the line gets cut, the file details go, not the clock
            juce::String one;
            if (withTime && timeLine.isNotEmpty()) one << timeLine << utf8 ("  \xc2\xb7  ");
            one << lines.joinIntoString (utf8 ("  \xc2\xb7  "));
            all.add (one);
        }
        float y = r.getBottom() - 15.0f * (float) all.size();
        for (auto& l : all)
        {
            g.drawText (l, Rectangle<float> (r.getX(), y, r.getWidth(), 15.0f), juce::Justification::centredLeft, true);
            y += 15.0f;
        }
    };

    const int hv = tp::headerVariant();
    if (hv == 1)
    {
        // transport block: one recessed capsule holding shuffle .. repeat
        auto cap = Rectangle<float> ((float) shuffleButton->getX() - 12.0f, (float) playButton->getY() - 4.0f,
                                     (float) (repeatButton->getRight() - shuffleButton->getX()) + 24.0f, (float) playButton->getHeight() + 8.0f);
        g.setColour (tp::C (0xff2a2a2a));
        g.fillRoundedRectangle (cap, cap.getHeight() * 0.5f);
        g.setColour (tp::C (0xff505050));
        g.drawRoundedRectangle (cap.reduced (0.5f), cap.getHeight() * 0.5f, 1.0f);
    }
    if (hv == 2 && ! bigTime.isEmpty() && timeLine.isNotEmpty())
    {
        const auto now = timeLine.upToFirstOccurrenceOf (" / ", false, false);
        const auto len = timeLine.fromFirstOccurrenceOf (" / ", false, false);
        auto r = bigTime.toFloat();
        g.setColour (theme::text);
        g.setFont (theme::mono (19.0f, false));
        g.drawText (now, r.withTrimmedBottom (14.0f), juce::Justification::centredLeft, false);
        g.setColour (theme::textDim);
        g.setFont (theme::mono (10.0f, false));
        g.drawText (utf8 ("von ") + len, r.withTrimmedTop (r.getHeight() - 14.0f), juce::Justification::centredLeft, false);
    }

    const float t = easeOutCubic (textFade.current);
    drawText (previousTitle, previousArtist, previousMeta, 1.0f - t, -8.0f * t, false);
    drawText (title, artist, meta, t, 8.0f * (1.0f - t), hv != 2 || bigTime.isEmpty());
}

void HeaderView::resized()
{
    // spacing on a 4 px grid: 8 inside a group, 16 between groups
    constexpr int gapS = 8, gapL = 16;
    const int w = getWidth(), h = getHeight();
    const int inner = h - 2 * kPad;

    // quick-access grid: secondary actions, so smaller than the transport (36 px tiles, 18 px icons)
    const int rows = inner >= 2 * 32 + gapS ? 2 : 1;
    const int tile = juce::jlimit (28, 36, (inner - gapS * (rows - 1)) / rows);
    const int cols = juce::jmax (1, quickColumns);
    const int all = (int) quick.size();
    // one row: show every button if they fit in a third of the width, otherwise the first row
    const int perRow = rows == 2 ? cols : (all * (tile + gapS) <= w / 3 ? all : cols);
    const int visible = juce::jmin (all, perRow * rows);
    const int qw = perRow * tile + (perRow - 1) * gapS;
    const int qh = rows * tile + (rows - 1) * gapS;
    const int qx = w - kPad - qw;
    for (int i = 0; i < all; ++i)
    {
        const bool show = i < visible;
        quick[(size_t) i]->setVisible (show);
        if (show)
            quick[(size_t) i]->setBounds (qx + (i % perRow) * (tile + gapS), (h - qh) / 2 + (i / perRow) * (tile + gapS), tile, tile);
    }

    // transport: play, prev/next at 70 %, repeat at 64 %
    const int playSize = juce::jlimit (36, 52, inner / 2 + 8);
    const int sideSize = juce::roundToInt ((float) playSize * 0.70f);
    const int repeatSize = juce::roundToInt ((float) playSize * 0.64f);
    const int core = sideSize * 2 + playSize + 2 * gapL;           // prev, play, next
    const int wing = gapS + repeatSize;                             // repeat on the right
    const int textX = artArea().getRight() + gapL;
    const int minText = 140, volIcon = 28;

    // play sits in the middle of the window (under the title) whenever that leaves room for the text
    // on the left and the volume icon on the right; otherwise it moves just as far as needed
    const int leftLimit = textX + minText + gapL + wing;           // shuffle sits left of prev
    const int rightLimit = qx - gapL - volIcon - gapL - wing;      // right edge of the core
    int coreX = (w - core) / 2;
    coreX = juce::jmin (coreX, rightLimit - core);
    coreX = juce::jmax (coreX, juce::jmin (leftLimit, rightLimit - core));
    coreX = juce::jmax (coreX, textX + wing);

    prevButton->setBounds (coreX, (h - sideSize) / 2, sideSize, sideSize);
    playButton->setBounds (prevButton->getRight() + gapL, (h - playSize) / 2, playSize, playSize);
    nextButton->setBounds (playButton->getRight() + gapL, (h - sideSize) / 2, sideSize, sideSize);
    repeatButton->setBounds (nextButton->getRight() + gapS, (h - repeatSize) / 2, repeatSize, repeatSize);
    shuffleButton->setBounds (coreX - gapS - repeatSize, (h - repeatSize) / 2, repeatSize, repeatSize);

    // volume: right-aligned before the grid, a full slider when there is room, else just the icon
    const int volSpace = qx - gapL - (repeatButton->getRight() + gapL);
    if (tp::headerVariant() > 0)
    {
        // mockup: the volume sits centred in the space between transport and quick buttons, and longer
        int left = repeatButton->getRight() + 2 * gapL, right = qx - 2 * gapL;
        bigTime = {};
        if (tp::headerVariant() == 2 && right - left > 360)
        {
            bigTime = { left, h / 2 - 22, 150, 44 };
            left += 150 + gapL;
        }
        int volW2 = juce::jlimit (0, tp::headerVariant() >= 3 ? 320 : 260, right - left);
        volume->iconOnly = volW2 < 100;
        if (volume->iconOnly) volW2 = volIcon;
        volume->setBounds (left + (right - left - volW2) / 2, h / 2 - 22, volW2, 44);
    }
    else
    {
    int volW = juce::jlimit (0, 180, volSpace);
    volume->iconOnly = volW < 100;
    if (volume->iconOnly) volW = volIcon;
    volume->setBounds (qx - gapL - volW, h / 2 - 22, volW, 44);
    }

    // text gets everything left of the transport
    textBounds = { textX, kPad + 2, juce::jmax (0, juce::jmin (420, shuffleButton->getX() - gapL - textX)), inner - 4 };

    if (onLayoutChanged) onLayoutChanged();
}

void HeaderView::mouseUp (const juce::MouseEvent& e)
{
    if (artArea().contains (e.getPosition()) && onOpen)
        onOpen();
    // the clock is always on the bottom line of the text: click switches elapsed / remaining
    else if (textBounds.withTrimmedTop (textBounds.getHeight() - 16).contains (e.getPosition()) && timeLine.isNotEmpty() && onTimeClicked)
    {
        onTimeClicked();
        repaint (textArea().expanded (0, 10));
    }
}
} // namespace tp
