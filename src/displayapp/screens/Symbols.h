#pragma once

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace Symbols {
        static constexpr const char* none = "";
        static constexpr const char* batteryHalf = "\xEF\x89\x82";
        static constexpr const char* heartBeat = "\xEF\x88\x9E";
        static constexpr const char* bluetooth = "\xEF\x8A\x94";
        static constexpr const char* shieldAlt = "\xEF\x8F\xAD";
        static constexpr const char* plug = "\xEF\x87\xA6";
        static constexpr const char* shoe = "\xEF\x95\x8B";
        static constexpr const char* clock = "\xEF\x80\x97";
        static constexpr const char* bell = "\xEF\x83\xB3";
        static constexpr const char* info = "\xEF\x84\xA9";
        static constexpr const char* list = "\xEF\x80\xBA";
        static constexpr const char* sun = "\xEF\x86\x85";
        static constexpr const char* check = "\xEF\x95\xA0";
        static constexpr const char* music = "\xEF\x80\x81";
        static constexpr const char* tachometer = "\xEF\x98\xA5";
        static constexpr const char* gallery = "\xEF\x8C\x82";
        static constexpr const char* paintbrush = "\xEF\x87\xBC";
        static constexpr const char* paddle = "\xEF\x91\x9D";
        static constexpr const char* map = "\xEF\x96\xa0";
        static constexpr const char* phone = "\xEF\x82\x95";
        static constexpr const char* phoneSlash = "\xEF\x8F\x9D";
        static constexpr const char* volumMute = "\xEF\x9A\xA9";
        static constexpr const char* volumUp = "\xEF\x80\xA8";
        static constexpr const char* volumDown = "\xEF\x80\xA7";
        static constexpr const char* stepForward = "\xEF\x81\x91";
        static constexpr const char* stepBackward = "\xEF\x81\x88";
        static constexpr const char* play = "\xEF\x81\x8B";
        static constexpr const char* pause = "\xEF\x81\x8C";
        static constexpr const char* stop = "\xEF\x81\x8D";
        static constexpr const char* stopWatch = "\xEF\x8B\xB2";
        static constexpr const char* hourGlass = "\xEF\x89\x92";
        static constexpr const char* lapsFlag = "\xEF\x80\xA4";
        static constexpr const char* drum = "\xEF\x95\xA9";
        static constexpr const char* dice = "\xEF\x94\xA2";
        static constexpr const char* eye = "\xEF\x81\xAE";
        static constexpr const char* home = "\xEE\x8E\xAF";
        static constexpr const char* sleep = "\xEE\xBD\x84";
        static constexpr const char* calculator = "\xEF\x87\xAC";
        static constexpr const char* backspace = "\xEF\x95\x9A";

        // App logos and emoji for Apple notifications (ANCS). The set of apps, emoji and glyphs comes from
        // Titus Kendzorra's InfiniBros fork (lil-watch branches).

        // notification_icons.c
        static constexpr const char* appleLogo = "\xEF\x85\xB9";     // 0xf179
        static constexpr const char* whatsappLogo = "\xEF\x88\xB2";  // 0xf232
        static constexpr const char* instagramLogo = "\xEF\x85\xAD"; // 0xf16d
        static constexpr const char* discordLogo = "\xEF\x8E\x92";   // 0xf392
        static constexpr const char* youtubeLogo = "\xEF\x85\xA7";   // 0xf167
        static constexpr const char* snapchatLogo = "\xEF\x8A\xAB";  // 0xf2ab
        static constexpr const char* githubLogo = "\xEF\x82\x9B";    // 0xf09b
        static constexpr const char* walletLogo = "\xEF\x90\x95";    // 0xf415
        static constexpr const char* podcastsLogo = "\xEF\x8B\x8E";  // 0xf2ce
        static constexpr const char* messagesLogo = "\xEF\x89\xBA";  // 0xf27a
        static constexpr const char* remindersLogo = "\xEF\x83\x8A"; // 0xf0ca
        static constexpr const char* shortcutsLogo = "\xEF\x97\xBD"; // 0xf5fd
        static constexpr const char* mailLogo = "\xEF\x83\xA0";      // 0xf0e0
        static constexpr const char* musicLogo = "\xEF\x80\x81";     // 0xf001
        static constexpr const char* phoneLogo = "\xEF\x82\x95";     // 0xf095

        // Emoji, drawn inline in jetbrains_mono_bold_20
        static constexpr const char* thumbsUp = "\xEF\x85\xA4";      // 0xf164
        static constexpr const char* thumbsDown = "\xEF\x85\xA5";    // 0xf165
        static constexpr const char* smile = "\xEF\x84\x98";         // 0xf118
        static constexpr const char* tired = "\xEF\x97\x88";         // 0xf5c8
        static constexpr const char* openMouth = "\xEF\x97\x82";     // 0xf5c2
        static constexpr const char* wink = "\xEF\x93\x9A";          // 0xf4da
        static constexpr const char* grin = "\xEF\x96\xB8";          // 0xf5b8
        static constexpr const char* sad = "\xEF\x96\xB4";           // 0xf5b4
        static constexpr const char* cry = "\xEF\x96\xB3";           // 0xf5b3
        static constexpr const char* rollingEyes = "\xEF\x96\xA5";   // 0xf5a5
        static constexpr const char* noMouth = "\xEF\x96\xA4";       // 0xf5a4
        static constexpr const char* neutralFace = "\xEF\x84\x9A";   // 0xf11a
        static constexpr const char* grinSquint = "\xEF\x96\x9B";    // 0xf59b
        static constexpr const char* grinSmile = "\xEF\x96\x9A";     // 0xf59a
        static constexpr const char* grinOpenMouth = "\xEF\x96\x99"; // 0xf599
        static constexpr const char* blowingKiss = "\xEF\x96\x98";   // 0xf598
        static constexpr const char* kissGrin = "\xEF\x96\x97";      // 0xf597
        static constexpr const char* kiss = "\xEF\x96\x96";          // 0xf596
        static constexpr const char* grinWide = "\xEF\x96\x81";      // 0xf581
        static constexpr const char* winkTongue = "\xEF\x96\x8B";    // 0xf58b
        static constexpr const char* squintTongue = "\xEF\x96\x8A";  // 0xf58a
        static constexpr const char* grinTongue = "\xEF\x96\x89";    // 0xf589
        static constexpr const char* laugh = "\xEF\x96\x88";         // 0xf588
        static constexpr const char* grinStars = "\xEF\x96\x87";     // 0xf587
        static constexpr const char* rofl = "\xEF\x96\x86";          // 0xf586
        static constexpr const char* heartEyes = "\xEF\x96\x84";     // 0xf584
        static constexpr const char* sweatSmile = "\xEF\x96\x83";    // 0xf583
        static constexpr const char* grimace = "\xEF\x95\xBF";       // 0xf57f
        static constexpr const char* slightFrown = "\xEF\x95\xBA";   // 0xf57a
        static constexpr const char* frown = "\xEF\x84\x99";         // 0xf119
        static constexpr const char* flushed = "\xEF\x95\xB9";       // 0xf579
        static constexpr const char* deadEyes = "\xEF\x95\xA7";      // 0xf567
        static constexpr const char* angry = "\xEF\x95\x96";         // 0xf556
        static constexpr const char* heartEmoji = "\xEF\x80\x84";    // 0xf004
        static constexpr const char* prayingHands = "\xEF\x9A\x84";  // 0xf684
        static constexpr const char* handshake = "\xEF\x8A\xB5";     // 0xf2b5
        static constexpr const char* peaceSign = "\xEF\x89\x9B";     // 0xf25b
        static constexpr const char* middleFinger = "\xEF\xA0\x86";  // 0xf806
        static constexpr const char* camera = "\xEF\x80\xB0";        // 0xf030
        static constexpr const char* videoCam = "\xEF\x80\xBD";      // 0xf03d
        static constexpr const char* microphone = "\xEF\x84\xB0";    // 0xf130
        static constexpr const char* flame = "\xEF\x81\xAD";         // 0xf06d

        // fontawesome_weathericons.c
        // static constexpr const char* sun = "\xEF\x86\x85";
        static constexpr const char* moon = "\xEF\x86\x86"; // 0xf186
        static constexpr const char* cloudSun = "\xEF\x9B\x84";
        static constexpr const char* cloudMoon = "\xEF\x9B\x83"; // 0xf6c3
        static constexpr const char* cloudSunRain = "\xEF\x9D\x83";
        static constexpr const char* cloudMoonRain = "\xEF\x9C\xBC"; // 0xf73c
        static constexpr const char* cloudShowersHeavy = "\xEF\x9D\x80";
        static constexpr const char* smog = "\xEF\x9D\x9F";
        static constexpr const char* cloud = "\xEF\x83\x82";
        static constexpr const char* cloudMeatball = "\xEF\x9C\xBB";
        static constexpr const char* bolt = "\xEF\x83\xA7";
        static constexpr const char* snowflake = "\xEF\x8B\x9C";
        static constexpr const char* ban = "\xEF\x81\x9E";

        // lv_font_sys_48.c
        static constexpr const char* settings = "\xEE\xA2\xB8";

        static constexpr const char* brightnessLow = "\xEE\x8E\xAA";
        static constexpr const char* brightnessMedium = "\xEE\x8E\xAB";
        static constexpr const char* brightnessHigh = "\xEE\x8E\xAC";

        static constexpr const char* notificationsOff = "\xEE\x9F\xB6";
        static constexpr const char* notificationsOn = "\xEE\x9F\xB7";

        static constexpr const char* flashlight = "\xEF\x80\x8B";
        static constexpr const char* bluetoothOn = "\xEE\x86\xA7";
        static constexpr const char* bluetoothOff = "\xEE\x86\xA9";
        static constexpr const char* paintbrushLg = "\xEE\x90\x8A";
      }
    }
  }
}
