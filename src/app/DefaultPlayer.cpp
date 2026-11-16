#include "DefaultPlayer.h"
#include "DocIcon.h"
#include "media/Media.h"
#if JUCE_WINDOWS
 #include "DocIcons.h"
#endif

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #include <shlobj.h>
 #include <shellapi.h>
#elif JUCE_MAC
 #include <CoreServices/CoreServices.h>
#endif

namespace tp::defaultplayer
{
juce::Colour resolveIconColour (const juce::String& setting)
{
    for (auto& s : docicon::swatches)
        if (setting == s.id)
            return juce::Colour (s.argb);
    if (setting.startsWithChar ('#') && setting.length() == 7)
        return juce::Colour::fromString ("ff" + setting.substring (1));
   #if JUCE_WINDOWS
    // "auto": light icons when Windows apps use the light theme
    const auto lightTheme = juce::WindowsRegistry::getValue (
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize\\AppsUseLightTheme", "1");
    if (lightTheme.getIntValue() != 0)
        return juce::Colour (docicon::swatches[2].argb);
   #endif
    return docicon::defaultPage;
}

#if JUCE_WINDOWS

bool isSupported() { return true; }

bool isDefault()
{
    const auto choice = juce::WindowsRegistry::getValue (
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.mp3\\UserChoice\\ProgId");
    return choice.startsWith ("Transport.");
}

namespace
{
/** Registers one ProgID per file type ("Transport.mp3"), each with its own icon from the .exe
    (big type label + small Transport logo), plus the Capabilities that list Transport in Default Apps.
    Only per user (HKCU); Windows never lets an app take the default without the user confirming. */
juce::File iconFolder()
{
    return juce::File::getSpecialLocation (juce::File::windowsLocalAppData).getChildFile ("Transport").getChildFile ("FileIcons");
}

/** Where each type's icon comes from: the .exe for the default colour, otherwise .ico files made for
    that colour (a new name per colour, so Explorer's icon cache can't show the old ones). */
juce::String iconFor (const docicons::Entry& entry, juce::Colour page, const juce::String& exe)
{
    const juce::String ext (entry.ext);
    if (page == docicon::defaultPage)
        return "\"" + exe + "\",-" + juce::String (entry.id);

    const auto file = iconFolder().getChildFile (ext + "-" + page.toDisplayString (false).toLowerCase() + ".ico");
    if (! file.existsAsFile())
    {
        file.getParentDirectory().createDirectory();
        const auto data = docicon::makeIco (ext, entry.video, page);
        if (! file.replaceWithData (data.getData(), data.getSize()))
            return "\"" + exe + "\",-" + juce::String (entry.id);
    }
    return "\"" + file.getFullPathName() + "\",0";
}

void removeOtherIcons (juce::Colour keep)
{
    const auto suffix = "-" + keep.toDisplayString (false).toLowerCase() + ".ico";
    for (auto& f : iconFolder().findChildFiles (juce::File::findFiles, false, "*.ico"))
        if (keep == docicon::defaultPage || ! f.getFileName().endsWith (suffix))
            f.deleteFile();
}

void registerTypes (juce::Colour page)
{
    using R = juce::WindowsRegistry;
    const auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getFullPathName();
    const juce::String classes = "HKEY_CURRENT_USER\\Software\\Classes\\";
    const juce::String caps = "HKEY_CURRENT_USER\\Software\\Transport\\Capabilities\\";
    const auto videos = media::videoExtensions();
    const juce::StringArray audio { "mp3", "wav", "flac", "aif", "aiff", "ogg", "m4a", "aac", "wma" };

    R::setValue (caps + "ApplicationName", "Transport");
    R::setValue (caps + "ApplicationDescription", "A music player that shows you what you hear.");

    for (const auto& entry : docicons::entries)
    {
        const juce::String ext (entry.ext);
        if (! (entry.video ? videos.contains (ext) : audio.contains (ext)))
            continue;   // a type this platform can't play

        const auto progId = "Transport." + ext;
        R::setValue (classes + progId + "\\", ext.toUpperCase() + (entry.video ? " video" : " audio"));
        R::setValue (classes + progId + "\\DefaultIcon\\", iconFor (entry, page, exe));
        R::setValue (classes + progId + "\\shell\\open\\command\\", "\"" + exe + "\" \"%1\"");
        R::setValue (caps + "FileAssociations\\." + ext, progId);
        R::setValue (classes + "." + ext + "\\OpenWithProgids\\" + progId, juce::String());
    }
    R::setValue ("HKEY_CURRENT_USER\\Software\\RegisteredApplications\\Transport", "Software\\Transport\\Capabilities");

    // older versions used one ProgID for all audio; keep it working, with the generic music icon
    if (R::keyExists (classes + "Transport.AudioFile"))
        R::setValue (classes + "Transport.AudioFile\\shell\\open\\command\\", "\"" + exe + "\" \"%1\"");

    removeOtherIcons (page);
    SHChangeNotify (SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
}

juce::Colour lastIconColour = docicon::defaultPage;
} // namespace

juce::String makeDefault()
{
    registerTypes (lastIconColour);
    // Windows only lets the user confirm the default, so open the settings page for this app
    ShellExecuteW (nullptr, L"open", L"ms-settings:defaultapps?registeredAppUser=Transport", nullptr, nullptr, SW_SHOWNORMAL);
    return "In Settings, choose Transport and click \"Set default\"";
}

void refresh (const juce::String& iconColour)
{
    lastIconColour = resolveIconColour (iconColour);
    // keep the registration pointing at this .exe (and its icons) when Transport is already registered
    if (juce::WindowsRegistry::keyExists ("HKEY_CURRENT_USER\\Software\\Transport\\Capabilities"))
        registerTypes (lastIconColour);
}

bool canColourIcons() { return true; }

juce::String setIconColour (const juce::String& iconColour)
{
    lastIconColour = resolveIconColour (iconColour);
    registerTypes (lastIconColour);
    return "File icons updated (Explorer may take a moment)";
}

#elif JUCE_MAC

bool isSupported() { return true; }

namespace
{
juce::String bundleId()
{
    if (auto id = CFBundleGetIdentifier (CFBundleGetMainBundle()))
        return juce::String::fromCFString (id);
    return "app.transport.player";
}
const char* kTypes[] = { "public.mp3", "com.microsoft.waveform-audio", "org.xiph.flac", "public.aiff-audio",
                         "public.aifc-audio", "public.mpeg-4-audio", "com.apple.m4a-audio", "org.xiph.ogg-audio" };
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

bool isDefault()
{
    CFStringRef handler = LSCopyDefaultRoleHandlerForContentType (CFSTR ("public.mp3"), kLSRolesAll);
    if (handler == nullptr) return false;
    const bool result = juce::String::fromCFString (handler).equalsIgnoreCase (bundleId());
    CFRelease (handler);
    return result;
}

juce::String makeDefault()
{
    const auto id = bundleId().toCFString();
    int ok = 0;
    for (auto* type : kTypes)
    {
        CFStringRef t = CFStringCreateWithCString (nullptr, type, kCFStringEncodingUTF8);
        if (LSSetDefaultRoleHandlerForContentType (t, kLSRolesViewer | kLSRolesEditor, id) == noErr) ++ok;
        CFRelease (t);
    }
    CFRelease (id);
    return ok > 0 ? "Transport now opens your audio files" : "macOS didn't allow the change";
}

#pragma clang diagnostic pop

void refresh (const juce::String&) {}   // macOS reads the icons from the app bundle
bool canColourIcons() { return false; }
juce::String setIconColour (const juce::String&) { return {}; }

#else

void refresh (const juce::String&) {}
bool canColourIcons() { return false; }
juce::String setIconColour (const juce::String&) { return {}; }
bool isSupported() { return false; }
bool isDefault() { return false; }
juce::String makeDefault() { return {}; }

#endif
} // namespace tp::defaultplayer
