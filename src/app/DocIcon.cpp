#include "DocIcon.h"
#include "Logo.h"

namespace tp::docicon
{
namespace
{
struct Scheme
{
    juce::Colour page, fold, text, accent, rim;
};

Scheme schemeFor (juce::Colour page)
{
    const bool lightPage = page.getPerceivedBrightness() > 0.62f;
    Scheme s;
    s.page = page.withAlpha (1.0f);
    s.fold = lightPage ? s.page.darker (0.22f) : s.page.brighter (0.45f);
    s.text = lightPage ? juce::Colour (0xff1c1c1c) : juce::Colours::white;
    s.rim  = lightPage ? juce::Colours::black.withAlpha (0.18f) : juce::Colours::white.withAlpha (0.39f);

    // the orange marks would vanish on an orange-ish page: use the text colour there
    const float hueGap = std::abs (page.getHue() - theme::accent.getHue());
    const bool nearAccent = juce::jmin (hueGap, 1.0f - hueGap) < 0.07f && page.getSaturation() > 0.35f;
    s.accent = nearAccent ? s.text : theme::accent;
    return s;
}

/** Polygon through `corners` (clockwise), each corner rounded with its own radius. */
juce::Path roundedPolygon (const std::vector<juce::Point<float>>& pts, const std::vector<float>& radii)
{
    juce::Path p;
    const int n = (int) pts.size();
    for (int i = 0; i < n; ++i)
    {
        const auto c = pts[(size_t) i];
        const auto prev = pts[(size_t) ((i + n - 1) % n)];
        const auto next = pts[(size_t) ((i + 1) % n)];
        const float r = radii[(size_t) i];
        const auto a = c + (prev - c) / prev.getDistanceFrom (c) * r;
        const auto b = c + (next - c) / next.getDistanceFrom (c) * r;
        if (i == 0) p.startNewSubPath (a); else p.lineTo (a);
        p.quadraticTo (c, b);
    }
    p.closeSubPath();
    return p;
}

juce::Font labelFont (float height)
{
   #if JUCE_WINDOWS
    return juce::Font (juce::FontOptions ("Segoe UI Black", height, juce::Font::plain));
   #else
    return juce::Font (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), height, juce::Font::bold));
   #endif
}

/** Largest font whose ink fits in maxW x maxH; returns the glyphs, laid out at (0, 0). */
juce::GlyphArrangement fitLabel (const juce::String& text, float maxW, float maxH)
{
    juce::GlyphArrangement ga;
    for (float h = maxH * 1.6f; h > 4.0f; h *= 0.96f)
    {
        ga.clear();
        ga.addLineOfText (labelFont (h), text, 0.0f, 0.0f);
        const auto ink = ga.getBoundingBox (0, -1, true);
        if (ink.getWidth() <= maxW && ink.getHeight() <= maxH)
            return ga;
    }
    return ga;
}
} // namespace

juce::Image render (const juce::String& ext, bool video, juce::Colour pageColour, int size)
{
    const auto sc = schemeFor (pageColour);
    const float S = (float) size;
    const bool small = size <= 32;

    juce::Image img (juce::Image::ARGB, size, size, true);
    juce::Graphics g (img);

    const float w = S * (small ? 0.80f : 0.74f), h = S * (small ? 0.96f : 0.92f);
    const float x0 = (S - w) / 2.0f - (small ? 0.0f : S * 0.03f), y0 = (S - h) / 2.0f;
    const float x1 = x0 + w, y1 = y0 + h;
    const float radius = S * (small ? 0.05f : 0.06f);
    const float fold = w * (small ? 0.22f : 0.26f);
    const auto page = roundedPolygon ({ { x0, y0 }, { x1 - fold, y0 }, { x1, y0 + fold }, { x1, y1 }, { x0, y1 } },
                                      { radius, radius * 0.3f, radius * 0.3f, radius, radius });
    const float rimW = juce::jmax (1.0f, S * 0.007f);

    if (! small)
        juce::DropShadow (juce::Colours::black.withAlpha (0.35f), juce::roundToInt (S * 0.03f), { 0, juce::roundToInt (S * 0.012f) }).drawForPath (g, page);

    g.setColour (sc.page);
    g.fillPath (page);
    juce::Path foldTri;
    foldTri.addTriangle (x1 - fold, y0, x1 - fold, y0 + fold, x1, y0 + fold);
    {
        juce::Graphics::ScopedSaveState s (g);
        g.reduceClipRegion (page);
        g.setColour (sc.fold);
        g.fillPath (foldTri);
    }
    g.setColour (sc.rim);
    g.strokePath (page, juce::PathStrokeType (rimW, juce::PathStrokeType::curved));

    const float cx = x0 + w / 2.0f;
    if (! small)
    {
        const float my = y0 + h * 0.28f;
        g.setColour (sc.accent);
        if (video)
        {
            const float t = w * 0.15f;
            juce::Path tri;
            tri.addTriangle (cx - t * 0.45f, my - t * 0.6f, cx + t * 0.62f, my, cx - t * 0.45f, my + t * 0.6f);
            g.fillPath (tri);
        }
        else
        {
            const float bars[] = { 0.35f, 0.7f, 1.0f, 0.55f, 0.85f, 0.4f, 0.65f };
            const float bw = w * 0.045f, gap = w * 0.032f;
            float bx = cx - (7.0f * bw + 6.0f * gap) / 2.0f;
            for (float v : bars)
            {
                const float bh = w * 0.22f * v;
                g.fillRoundedRectangle (bx, my - bh / 2.0f, bw, bh, bw / 2.0f);
                bx += bw + gap;
            }
        }
    }

    const auto label = ext.toUpperCase();
    const float maxW = w * (small ? 0.90f : 0.84f);
    const float maxH = h * (small ? 0.46f : 0.27f) * (label.length() <= 3 ? 1.0f : 0.88f);
    auto glyphs = fitLabel (label, maxW, maxH);
    const auto ink = glyphs.getBoundingBox (0, -1, true);
    const float ty = y0 + h * (small ? 0.56f : 0.57f);
    glyphs.moveRangeOfGlyphs (0, -1, cx - ink.getCentreX(), ty - ink.getCentreY());
    g.setColour (sc.text);
    glyphs.draw (g);

    if (! small)
    {
        const float rw = w * 0.22f;
        const float ry = ty + ink.getHeight() / 2.0f + h * 0.055f;
        g.setColour (sc.accent);
        g.fillRoundedRectangle (cx - rw / 2.0f, ry, rw, S * 0.013f, S * 0.0065f);

        // the logo as a small stamp, bottom right, in a page-coloured ring with the rim
        const float ls = S * 0.36f, lx = S - ls, ly = S - ls;
        const float inset = ls * 0.0625f, lr = ls * 0.19f, gapR = S * 0.014f;
        const auto ring = juce::Rectangle<float> (lx + inset - gapR, ly + inset - gapR, ls - 2.0f * (inset - gapR), ls - 2.0f * (inset - gapR));
        g.setColour (sc.page);
        g.fillRoundedRectangle (ring, lr + gapR);
        g.setColour (sc.rim);
        g.drawRoundedRectangle (ring.reduced (rimW * 0.5f), lr + gapR, rimW);
        logo::paintIcon (g, { lx, ly, ls, ls });
    }
    else
    {
        const float rr = S * 0.075f;
        g.setColour (sc.accent);
        g.fillEllipse (x1 - rr * 2.7f, y1 - rr * 2.7f, rr * 2.0f, rr * 2.0f);
    }
    return img;
}

juce::MemoryBlock makeIco (const juce::String& ext, bool video, juce::Colour page)
{
    const int sizes[] = { 16, 24, 32, 48, 64, 128, 256 };
    constexpr int count = (int) std::size (sizes);

    std::vector<juce::MemoryBlock> pngs;
    juce::PNGImageFormat png;
    for (int s : sizes)
    {
        juce::MemoryOutputStream out;
        png.writeImageToStream (render (ext, video, page, s), out);
        pngs.push_back (out.getMemoryBlock());
    }

    // ICONDIR + one ICONDIRENTRY per image, then the PNG data (Windows Vista and later read PNG entries)
    juce::MemoryOutputStream ico;
    ico.writeShort (0);
    ico.writeShort (1);
    ico.writeShort ((short) count);
    int offset = 6 + 16 * count;
    for (int i = 0; i < count; ++i)
    {
        const int s = sizes[i];
        ico.writeByte ((char) (s >= 256 ? 0 : s));
        ico.writeByte ((char) (s >= 256 ? 0 : s));
        ico.writeByte (0);              // palette size
        ico.writeByte (0);              // reserved
        ico.writeShort (1);             // planes
        ico.writeShort (32);            // bits per pixel
        ico.writeInt ((int) pngs[(size_t) i].getSize());
        ico.writeInt (offset);
        offset += (int) pngs[(size_t) i].getSize();
    }
    for (auto& p : pngs)
        ico.write (p.getData(), p.getSize());
    return ico.getMemoryBlock();
}
} // namespace tp::docicon
