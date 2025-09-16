class Tile extends rumTile
{
    constructor()
    {
        base.constructor();
    }

    function isCollision(eMoveType)
    {
        return false;
    }
}

class Tile_Floor_Brick extends Tile
{
}

class Tile_Floor_Rock extends Tile
{
}

class Tile_Floor_Tile_A extends Tile
{
}

class Tile_Floor_Tile_B extends Tile
{
}
