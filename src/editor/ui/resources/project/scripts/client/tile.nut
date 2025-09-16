class Tile extends rumTile
{
    static s_bBlocksLOS = false;

    function isCollision(eMoveType)
    {
        return false;
    }
}

class Tile_Floor_Brick extends Tile
{
    static s_strDescription = "tile_floor_brick";
    static s_cGraphic = Graphic_floor_brick;
}

class Tile_Floor_Rock extends Tile
{
    static s_strDescription = "tile_floor_rock";
    static s_cGraphic = Graphic_floor_rock;
}

class Tile_Floor_Tile_A extends Tile
{
    static s_strDescription = "tile_floor_tile_a";
    static s_cGraphic = Graphic_floor_tile_a;
}

class Tile_Floor_Tile_B extends Tile
{
    static s_strDescription = "tile_floor_tile_b";
    static s_cGraphic = Graphic_floor_tile_b;
}
