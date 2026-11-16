#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace tp
{
/** Where settings live: ~/.config/Transport on Linux, the usual app-data folders elsewhere. */
inline juce::String configFolder()
{
   #if JUCE_LINUX
    return ".config/Transport";
   #else
    return "Transport";
   #endif
}

/** Literal UTF-8 text (juce::String treats plain char* as ASCII). */
inline juce::String utf8 (const char* s) { return juce::String::fromUTF8 (s); }


/** MOCKUP ONLY: TP_STYLE=0..5 picks a visual identity. 0 = current look. */
inline int style()
{
    static const int s = [] { const char* e = std::getenv ("TP_STYLE"); return e ? std::atoi (e) : 0; }();
    return s;
}

/** Maps the current neutral grey ramp + amber accent onto the chosen style's palette. */
inline juce::Colour C (juce::uint32 argb)
{
    const juce::Colour in (argb);
    const int st = style();
    if (st == 0) return in;

    struct P { float l; juce::uint32 c; };
    // anchors: source grey level -> target colour
    static const std::vector<std::vector<P>> ramps = {
        {},
        // 1 Studio-Schwarz: near black, neutral
        { {0,0xff000000},{27,0xff070708},{42,0xff0e0e0f},{56,0xff111113},{62,0xff09090a},{75,0xff141416},{86,0xff161618},{99,0xff222226},{120,0xff3a3a40},{140,0xff6c6c74},{180,0xffa6a6ad},{212,0xffd4d4d8},{255,0xffffffff} },
        // 2 Messgeraet: cool blue graphite
        { {0,0xff000000},{27,0xff0a0d10},{42,0xff12171c},{56,0xff161c22},{62,0xff10151a},{75,0xff1a2129},{86,0xff1c242c},{99,0xff28323d},{120,0xff3c4856},{140,0xff6a7a8c},{180,0xffa4b2c2},{212,0xffd2dbe6},{255,0xfff4f8ff} },
        // 3 Werkstatt: warm graphite, cream text
        { {0,0xff000000},{27,0xff0f0d0b},{42,0xff181512},{56,0xff1c1915},{62,0xff141210},{75,0xff221e1a},{86,0xff26221d},{99,0xff342e27},{120,0xff4a4239},{140,0xff7e7466},{180,0xffb5aa98},{212,0xffdcd2c0},{255,0xfffaf3e6} },
        // 4 Nah dran: today's greys unchanged, only accent and shapes move
        { {0,0xff000000},{255,0xffffffff} },
        // 5 Eine Flaeche: everything on one surface, hairlines between
        { {0,0xff000000},{27,0xff0e0e10},{42,0xff17171a},{56,0xff1d1d20},{62,0xff1d1d20},{75,0xff1d1d20},{86,0xff1d1d20},{99,0xff2e2e33},{120,0xff45454b},{140,0xff76767f},{180,0xffadadb5},{212,0xffd8d8de},{255,0xffffffff} },
    };
    static const juce::uint32 accents[] = { 0xffe8a33a, 0xffff5a1f, 0xffff7a2e, 0xffe8582e, 0xffff6a2b, 0xffff6a1f };

    const float r = in.getFloatRed(), g = in.getFloatGreen(), b = in.getFloatBlue();
    const float mx = juce::jmax (r, g, b), mn = juce::jmin (r, g, b);
    const float lum = (r + g + b) / 3.0f * 255.0f;

    auto ramp = [&] (float l)
    {
        const auto& rp = ramps[(size_t) st];
        if (l <= rp.front().l) return juce::Colour (rp.front().c);
        for (size_t i = 1; i < rp.size(); ++i)
            if (l <= rp[i].l)
            {
                const float t = (l - rp[i - 1].l) / (rp[i].l - rp[i - 1].l);
                return juce::Colour (rp[i - 1].c).interpolatedWith (juce::Colour (rp[i].c), t);
            }
        return juce::Colour (rp.back().c);
    };

    juce::Colour out = ramp (lum);
    const float sat = mx - mn;
    // warm/orange-ish colours (r >= g >= b): pull toward the new accent by how saturated they are
    if (sat > 0.06f && r >= g && g >= b)
    {
        const float t = juce::jlimit (0.0f, 1.0f, sat / 0.68f);
        out = out.interpolatedWith (juce::Colour (accents[st]), t);
    }
    else if (sat > 0.06f)
        out = in; // other hues (spectral blues, traffic lights, red close) stay
    return out.withAlpha (in.getFloatAlpha());
}

namespace theme
{
// Palette taken from the prototype
inline const juce::Colour background = C (0xff3e3e3e);
inline const juce::Colour titleBar = C (0xff3a3a3a);
inline const juce::Colour titleBlock = C (0xff1b1b1b);
inline const juce::Colour header = C (0xff383838);
inline const juce::Colour waveBg = C (0xff4b4b4b);
inline const juce::Colour panel = C (0xff565656);
inline const juce::Colour panelHover = C (0xff5d5d5d);
inline const juce::Colour button = C (0xff575757);
inline const juce::Colour buttonHover = C (0xff636363);
inline const juce::Colour buttonBorder = C (0xff626262);
inline const juce::Colour icon = C (0xff8c8c8c);
inline const juce::Colour iconBright = C (0xffd2d2d2);
inline const juce::Colour text = C (0xffffffff);
inline const juce::Colour textDim = C (0xffababab);
inline const juce::Colour label = C (0xffd4d4d4);
inline const juce::Colour scale = C (0xffbdbdbd);
inline const juce::Colour waveUnplayed = C (0xffb4b4b4);
inline const juce::Colour accent = C (0xffe8a33a);
inline const juce::Colour dialog = C (0xff353535);
inline const juce::Colour deep = C (0xff2a2a2a);
inline const juce::Colour field = C (0xff1f1f1f);

inline juce::String sansName()
{
   #if JUCE_WINDOWS
    return "Segoe UI";
   #else
    return juce::Font::getDefaultSansSerifFontName();
   #endif
}

/** Font sized in CSS-like pixels (the design mockups' sizes). */
inline juce::Font font (float px, bool bold = false)
{
    return juce::Font (juce::FontOptions (sansName(), px * 1.33f, bold ? juce::Font::bold : juce::Font::plain));
}

inline juce::Font mono (float px, bool bold = true)
{
   #if JUCE_WINDOWS
    const juce::String name = "Consolas";
   #elif JUCE_MAC
    const juce::String name = "Menlo";
   #else
    const juce::String name = juce::Font::getDefaultMonospacedFontName();
   #endif
    return juce::Font (juce::FontOptions (name, px * 1.33f, bold ? juce::Font::bold : juce::Font::plain));
}
} // namespace theme

/** Frame-rate independent exponential smoothing. */
struct Smoothed
{
    float current = 0.0f, target = 0.0f;

    /** Returns true while still moving. speed ~ 1/time-constant in seconds. */
    bool tick (float dt, float speed) noexcept
    {
        const float prev = current;
        current += (target - current) * (1.0f - std::exp (-speed * dt));
        if (std::abs (target - current) < 0.0005f)
            current = target;
        return current != prev;
    }

    void snap (float v) noexcept { current = target = v; }
    bool moving() const noexcept { return current != target; }
};

/** Critically-damped-ish spring for needles and anything that should feel physical. */
struct Spring
{
    float position = 0.0f, velocity = 0.0f, target = 0.0f;
    float stiffness = 180.0f, damping = 22.0f;

    bool tick (float dt) noexcept
    {
        dt = juce::jmin (dt, 1.0f / 30.0f);
        const float a = stiffness * (target - position) - damping * velocity;
        velocity += a * dt;
        position += velocity * dt;
        return std::abs (velocity) > 1.0e-4f || std::abs (target - position) > 1.0e-4f;
    }
};

inline float easeOutCubic (float t) noexcept { t = juce::jlimit (0.0f, 1.0f, t); return 1.0f - std::pow (1.0f - t, 3.0f); }
inline float easeInOutCubic (float t) noexcept
{
    t = juce::jlimit (0.0f, 1.0f, t);
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow (-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

/** Everything that animates implements this; MainComponent ticks the tree once per vblank. */
struct Animated
{
    virtual ~Animated() = default;
    virtual void animate (float dt) = 0;
};

/** Small helper for hover/press state on custom-painted widgets. */
struct HoverState
{
    Smoothed hover, press;
    bool tick (float dt)
    {
        const bool a = hover.tick (dt, 18.0f);
        const bool b = press.tick (dt, 30.0f);
        return a || b;
    }
};
} // namespace tp
