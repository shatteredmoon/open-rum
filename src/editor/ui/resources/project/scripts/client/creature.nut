class Creature extends rumCreature
{
    //static s_cGraphic = Graphic_creature;
    static s_bBlocksLOS = false;
    static rumDrawOrder = 300;

    hitpoints = 0;

    constructor()
    {
        base.constructor();
        rumSetGraphic(s_cGraphic);
    }
}