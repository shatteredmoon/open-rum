class Graphic extends rumGraphic
{
    // The number of animation frames per set (vertical tiles)
    static s_iNumFrames = 1;

    // The number of sets in an animation (horizontal tiles)
    static s_iNumSets = 1;

    constructor()
    {
        base.constructor();
    }
}

class Graphic_floor_brick extends Graphic
{
    static s_strFilename = "floor_brick.png";
}

class Graphic_floor_rock extends Graphic
{
    static s_strFilename = "floor_rock.png";
}

class Graphic_floor_tile_a extends Graphic
{
    static s_strFilename = "floor_tile_a.png";
}

class Graphic_floor_tile_b extends Graphic
{
    static s_strFilename = "floor_tile_b.png";
}

class Graphic_widget_bush extends Graphic
{
    static s_strFilename = "widget_bush.png";
}

class Graphic_widget_rock extends Graphic
{
    static s_strFilename = "widget_rock.png";
}
