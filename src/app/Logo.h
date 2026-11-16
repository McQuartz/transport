#pragma once

#include "Theme.h"

namespace tp::logo
{
// The Transport logo: two overlapping discs, light bottom left and grey top right, the overlap in the
// accent colour. Same geometry as resources/icon.png (tools/make_icon.py), in a 1024 box.
inline const juce::Colour tile  { 0xff1c1c1c };
inline const juce::Colour light { 0xfff2f2f2 };
inline const juce::Colour grey  { 0xff555555 };

/** Just the two discs, scaled so the pair fills `box` (no tile). */
inline void paintMark (juce::Graphics& g, juce::Rectangle<float> box,
                       juce::Colour lightColour = light, juce::Colour greyColour = grey, juce::Colour overlap = theme::accent)
{
    // in the 1024 box the discs span x 210..814, y 234..790; centre that span in `box`
    const float k = juce::jmin (box.getWidth() / 604.0f, box.getHeight() / 556.0f);
    const auto c = box.getCentre();
    auto disc = [&] (float x, float y)
    {
        juce::Path p;
        p.addEllipse (c.x + (x - 512.0f) * k - 220.0f * k, c.y + (y - 512.0f) * k - 220.0f * k, 440.0f * k, 440.0f * k);
        return p;
    };
    const auto a = disc (430.1f, 569.4f), b = disc (593.9f, 454.6f);
    g.setColour (lightColour);
    g.fillPath (a);
    g.setColour (greyColour);
    g.fillPath (b);
    juce::Graphics::ScopedSaveState s (g);
    g.reduceClipRegion (a);
    g.setColour (overlap);
    g.fillPath (b);
}

/** The full app icon: dark rounded tile with the mark, as in icon.png. `box` is the 1024 canvas. */
inline void paintIcon (juce::Graphics& g, juce::Rectangle<float> box)
{
    const float k = box.getWidth() / 1024.0f;
    g.setColour (tile);
    g.fillRoundedRectangle (box.reduced (64.0f * k), 200.0f * k);
    // mark centre in the 1024 box is (512, 512)
    paintMark (g, juce::Rectangle<float> (604.0f * k, 556.0f * k).withCentre (box.getCentre()));
}
} // namespace tp::logo
