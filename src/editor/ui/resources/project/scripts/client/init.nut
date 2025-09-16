rumLoadScript("shared.nut");
rumLoadScript("utility.nut");
rumLoadScript("property.nut");
rumLoadScript("inventory.nut");
rumLoadScript("graphic.nut");
rumLoadScript("tile.nut");
rumLoadScript("widget.nut");
rumLoadScript("portal.nut");
rumLoadScript("creature.nut");
rumLoadScript("player.nut");
rumLoadScript("map.nut");
rumLoadScript("sound.nut");
rumLoadScript("broadcast.nut");

g_fMainTimer <- 0.0;

function onInit(clientTime)
{
    print ("Client initialization\n");

    // Seed the random number generator
    srand(time());

    g_fMainTimer = clientTime;

    PushKeyboardMode(keyboardMode.Game);

    // Some things to remember about fonts
    // 1. If you want dynamic color blitting, the face should be white
    // 2. Outlines in any color will always change to black during dynamic
    //    color changes
    // 3. Use attrib.blendFace = false for very cheap pseudo-outline
    // 4. If you use a single color most of the time and it is not white,
    //    consider making that color its own separate font. Use the white
    //    version for color changes.

    local fp = rumFontProps();
    fp.PixelHeight = 13;
    fp.FaceColor = rumColorWhite;
    fp.BlendFace = false;
    rumCreateFont("fonts/tahoma.ttf", "default", fp);
    fp.PixelHeight = 14;
    rumCreateFont("fonts/runes.ttf", "runes", fp);

    //rumExtractArchive(rumGamePath + "/graphics/vga.z");

    print("Creating tiles...\n");

    // Build the default tileset (vga)
    g_ciVGATileSet = rumCreateTileSet();

    // These are temporarily filed under g_ciVGATileSet
    rumLoadGraphic(Graphic_border);
    rumLoadGraphic(Graphic_moonphases);
    rumLoadGraphic(Graphic_prompt);
    rumLoadGraphic(Graphic_cursor);

    rumLoadGraphicArchive("vga_g.z");

    // Build the commodore 64 tileset
    g_ciC64TileSet = rumCreateTileSet();
    rumLoadGraphicArchive("c64_g.z");

    // Switch back to the vga tilest
    rumUseTileSet(g_ciVGATileSet);

    rumAddMapArchive("maps.z");

    // Build the default sound group
    g_ciVGASoundSet = rumGetCurrentSampleGroup();
    rumLoadSoundArchiveStubs("vga_s.z");

    // Build the c64 sound group
    g_ciC64SoundSet = rumCreateSampleGroup();
    rumLoadSoundArchiveStubs("c64_s.z");

    // Switch back to the default sound group
    rumSetCurrentSampleGroup(g_ciVGASoundSet);

    MusicChannel = rumFetchSound(Sound_u4_towns).rumPlay();

    return 0;
}


function onEditorInit(editorTime)
{
    return 0;
}
