// [CHOSEN, not recovered] HiDPI menu text bridge (kr4uzi/OpenBF2#42).
//
// The Flash menus draw their dynamic text through the renderer's FontSystem: SwiffPlayer asks for
// "<exported font name>_<pixel height>" and the renderer loads that pre-rasterised .dif/.dds pair
// (Fonts/<language>/StandardText_13 and the like, authored for a 1024x768 reference) and scales it
// by width/1024 - 3.75x at 3840x2160, so 13 px bitmaps end up as blurred 49 px glyphs. The SWFs
// that export those names embed the same typefaces as vector outlines (StandardText = Helvetica
// Condensed, StandardTextBold = TradeGothic Bold Condensed, DynamicText = Helvetica in
// mainMenu.swf). gameswf registers every exported font under its export name and these two calls
// rasterise one glyph's outline, so the FontSystem can rebuild a font's atlas at a multiple of its
// resolution while keeping the .dif metrics, layout and texel rectangles untouched.
// On macOS static-plugin clients, BF2_MENU_HIDPI=1 enables the higher-resolution atlases.
#ifndef SWIFFPLAYERABI_HIDPIGLYPHS_H
#define SWIFFPLAYERABI_HIDPIGLYPHS_H

#include <cstdlib>

inline bool openbf2HiDpiMenuEnabled()
{
#if defined(__APPLE__) && defined(BF2_STATIC_PLUGINS) && BF2_STATIC_PLUGINS
    static const bool enabled = [] {
        const char* setting = std::getenv("BF2_MENU_HIDPI");
        return setting && setting[0] == '1' && setting[1] == '\0';
    }();
    return enabled;
#else
    return false;
#endif
}

extern "C" {

// Bumped whenever a movie registers a new exported font; a font that found no matching outlines
// tries again once this changes.
unsigned int openbf2HiDpiFontGeneration();

// The renderer's hook for fonts waiting on outlines; SwiffPlayer calls the notifier after every
// movie it loads (all of that movie's fonts are registered by then).
void openbf2HiDpiSetFontListener(void (*listener)());
void openbf2HiDpiNotifyMovieLoaded();

// The font name (DefineFont2) of a candidate, for logs; "" when out of range.
const char* openbf2HiDpiFontName(const char* exportName, int index);

// How many registered SWF fonts carry this export name (case-insensitive); 0 = none. The
// pseudo name "*" lists every outline font, one per font name.
int openbf2HiDpiFontCandidates(const char* exportName);

// Coverage (0..1) of character `code` of candidate `index`, with the font's EM square scaled to
// `emPx` pixels and its pen origin on the baseline at pixel (ox, oy) of a w x h buffer, each pixel
// sampled ss x ss. Writes all of out[0..w*h); returns false (out zeroed) when the font has no
// outline for the character.
bool openbf2HiDpiRasterGlyph(const char* exportName, int index, unsigned short code, float emPx,
                             float ox, float oy, float* out, int w, int h, int ss);

}

#endif // SWIFFPLAYERABI_HIDPIGLYPHS_H
