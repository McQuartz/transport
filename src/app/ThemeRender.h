#pragma once

#include "Theme.h"

namespace tp::theme
{
/** A graphics context that passes every call on to another one, but sends each colour through
    theme::map first. Everything the app paints is designed in the dark palette; this is how the
    Ash and Light themes reach every widget and plugin without touching their paint code.
    Images (cover art, video frames) are drawn as they are. */
class MappedContext final : public juce::LowLevelGraphicsContext
{
public:
    explicit MappedContext (juce::LowLevelGraphicsContext& target) : c (target) {}

    bool isVectorDevice() const override                                  { return c.isVectorDevice(); }
    void setOrigin (juce::Point<int> p) override                          { c.setOrigin (p); }
    void addTransform (const juce::AffineTransform& t) override           { c.addTransform (t); }
    float getPhysicalPixelScaleFactor() const override                    { return c.getPhysicalPixelScaleFactor(); }
    bool clipToRectangle (const juce::Rectangle<int>& r) override         { return c.clipToRectangle (r); }
    bool clipToRectangleList (const juce::RectangleList<int>& r) override { return c.clipToRectangleList (r); }
    void excludeClipRectangle (const juce::Rectangle<int>& r) override    { c.excludeClipRectangle (r); }
    void clipToPath (const juce::Path& p, const juce::AffineTransform& t) override { c.clipToPath (p, t); }
    void clipToImageAlpha (const juce::Image& i, const juce::AffineTransform& t) override { c.clipToImageAlpha (i, t); }
    bool clipRegionIntersects (const juce::Rectangle<int>& r) override    { return c.clipRegionIntersects (r); }
    juce::Rectangle<int> getClipBounds() const override                   { return c.getClipBounds(); }
    bool isClipEmpty() const override                                     { return c.isClipEmpty(); }
    void saveState() override                                             { c.saveState(); }
    void restoreState() override                                          { c.restoreState(); }
    void beginTransparencyLayer (float o) override                        { c.beginTransparencyLayer (o); }
    void endTransparencyLayer() override                                  { c.endTransparencyLayer(); }

    void setFill (const juce::FillType& f) override
    {
        if (f.isColour())
        {
            c.setFill (juce::FillType (map (f.colour)));
        }
        else if (f.isGradient())
        {
            auto grad = *f.gradient;
            for (int i = 0; i < grad.getNumColours(); ++i)
                grad.setColour (i, map (grad.getColour (i)));
            juce::FillType mapped (grad);
            mapped.setOpacity (f.getOpacity());
            mapped.transform = f.transform;
            c.setFill (mapped);
        }
        else
        {
            c.setFill (f);
        }
    }

    void setOpacity (float o) override                                    { c.setOpacity (o); }
    void setInterpolationQuality (juce::Graphics::ResamplingQuality q) override { c.setInterpolationQuality (q); }
    void fillAll() override                                               { c.fillAll(); }
    void fillRect (const juce::Rectangle<int>& r, bool replace) override  { c.fillRect (r, replace); }
    void fillRect (const juce::Rectangle<float>& r) override              { c.fillRect (r); }
    void fillRectList (const juce::RectangleList<float>& r) override      { c.fillRectList (r); }
    void fillPath (const juce::Path& p, const juce::AffineTransform& t) override { c.fillPath (p, t); }
    void drawRect (const juce::Rectangle<float>& r, float t) override     { c.drawRect (r, t); }
    void strokePath (const juce::Path& p, const juce::PathStrokeType& s, const juce::AffineTransform& t) override { c.strokePath (p, s, t); }
    void drawImage (const juce::Image& i, const juce::AffineTransform& t) override { c.drawImage (i, t); }
    void drawLine (const juce::Line<float>& l) override                   { c.drawLine (l); }
    void drawLineWithThickness (const juce::Line<float>& l, float t) override { c.drawLineWithThickness (l, t); }
    void setFont (const juce::Font& f) override                           { c.setFont (f); }
    const juce::Font& getFont() override                                  { return c.getFont(); }
    void drawGlyphs (juce::Span<const uint16_t> g, juce::Span<const juce::Point<float>> p, const juce::AffineTransform& t) override { c.drawGlyphs (g, p, t); }
    std::unique_ptr<juce::ImageType> getPreferredImageTypeForTemporaryImages() const override { return c.getPreferredImageTypeForTemporaryImages(); }
    void drawRoundedRectangle (const juce::Rectangle<float>& r, float k, float t) override { c.drawRoundedRectangle (r, k, t); }
    void fillRoundedRectangle (const juce::Rectangle<float>& r, float k) override { c.fillRoundedRectangle (r, k); }
    void drawEllipse (const juce::Rectangle<float>& r, float t) override  { c.drawEllipse (r, t); }
    void fillEllipse (const juce::Rectangle<float>& r) override           { c.fillEllipse (r); }
    uint64_t getFrameId() const override                                  { return c.getFrameId(); }

private:
    juce::LowLevelGraphicsContext& c;
};

/** Paints a component (and everything inside it) through MappedContext. Installed with
    Component::setCachedComponentImage on the window's content; caches nothing. */
class MappedPainter final : public juce::CachedComponentImage
{
public:
    explicit MappedPainter (juce::Component& owner) : comp (owner) {}

    void paint (juce::Graphics& g) override
    {
        if (current() == Mode::dark) { comp.paintEntireComponent (g, false); return; }
        MappedContext mapped (g.getInternalContext());
        juce::Graphics mg (mapped);
        comp.paintEntireComponent (mg, false);
    }
    bool invalidateAll() override                      { return true; }
    bool invalidate (const juce::Rectangle<int>&) override { return true; }
    void releaseResources() override                   {}

private:
    juce::Component& comp;
};

/** For code that paints into its own image (trails, thumbnails): a Graphics in the current theme. */
class ThemedGraphics
{
public:
    explicit ThemedGraphics (juce::Image& img) : plain (img), mapped (plain.getInternalContext()), themed (mapped) {}
    juce::Graphics& get() { return current() == Mode::dark ? plain : themed; }

private:
    juce::Graphics plain;
    MappedContext mapped;
    juce::Graphics themed;
};
/** The colours of popup menus, tooltips and dialogs (separate windows) in the current theme. */
inline void applyToLookAndFeel (juce::LookAndFeel& lf)
{
    using juce::Colour;
    lf.setColour (juce::PopupMenu::backgroundColourId, map (Colour (0xff19191c)));
    lf.setColour (juce::PopupMenu::highlightedBackgroundColourId, map (Colour (0xff2a2a2e)));
    lf.setColour (juce::PopupMenu::textColourId, map (juce::Colours::white));
    lf.setColour (juce::PopupMenu::highlightedTextColourId, map (juce::Colours::white));
    lf.setColour (juce::PopupMenu::headerTextColourId, accent);
    lf.setColour (juce::TooltipWindow::backgroundColourId, map (Colour (0xff121215)));
    lf.setColour (juce::TooltipWindow::textColourId, map (juce::Colours::white));
    lf.setColour (juce::TooltipWindow::outlineColourId, map (Colour (0xff28282b)));
    lf.setColour (juce::ResizableWindow::backgroundColourId, map (dialog));
    lf.setColour (juce::AlertWindow::backgroundColourId, map (dialog));
    lf.setColour (juce::AlertWindow::textColourId, map (juce::Colours::white));
    lf.setColour (juce::TextButton::buttonColourId, map (button));
    lf.setColour (juce::TextButton::textColourOffId, map (juce::Colours::white));
    lf.setColour (juce::TextButton::textColourOnId, map (juce::Colours::white));
    lf.setColour (juce::ComboBox::backgroundColourId, map (field));
    lf.setColour (juce::ComboBox::outlineColourId, map (Colour (0xff28282b)));
    lf.setColour (juce::ComboBox::textColourId, map (juce::Colours::white));
    lf.setColour (juce::ComboBox::arrowColourId, map (Colour (0xffb0b0b7)));
    lf.setColour (juce::Label::textColourId, map (juce::Colours::white));
    lf.setColour (juce::ListBox::backgroundColourId, map (field));
    lf.setColour (juce::ListBox::textColourId, map (juce::Colours::white));
    lf.setColour (juce::ToggleButton::textColourId, map (juce::Colours::white));
    lf.setColour (juce::ToggleButton::tickColourId, map (juce::Colours::white));
    lf.setColour (juce::ToggleButton::tickDisabledColourId, map (Colour (0xff7a7a82)));
    lf.setColour (juce::Slider::backgroundColourId, map (Colour (0xff2f2f33)));
    lf.setColour (juce::Slider::trackColourId, map (Colour (0xffb0b0b7)));
    lf.setColour (juce::Slider::thumbColourId, accent);
    lf.setColour (juce::TextEditor::backgroundColourId, map (field));
    lf.setColour (juce::TextEditor::textColourId, map (juce::Colours::white));
    lf.setColour (juce::TextEditor::outlineColourId, map (Colour (0xff28282b)));
    lf.setColour (juce::ScrollBar::thumbColourId, map (Colour (0xff3d3d42)));
}

/** A component painted into an image, in the current theme (snapshots). */
inline juce::Image snapshot (juce::Component& comp, float factor)
{
    const auto b = comp.getLocalBounds();
    juce::Image img (juce::Image::ARGB, juce::jmax (1, juce::roundToInt ((float) b.getWidth() * factor)),
                     juce::jmax (1, juce::roundToInt ((float) b.getHeight() * factor)), true);
    {
        ThemedGraphics tg (img);
        auto& g = tg.get();
        g.addTransform (juce::AffineTransform::scale (factor));
        comp.paintEntireComponent (g, true);
    }
    return img;
}
} // namespace tp::theme
