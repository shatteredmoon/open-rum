class Portal extends rumPortal
{
    //static s_cGraphic = Graphic_portal;
    static s_bBlocksLOS = false;
    static rumDrawOrder = 100;

    constructor()
    {
        base.constructor();
        rumSetGraphic(s_cGraphic);
    }
}
