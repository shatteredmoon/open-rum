class Widget extends rumWidget
{
    static s_cGraphic = Graphic_widget_rock;
    static s_bBlocksLOS = false;
    static rumDrawOrder = 200;

    constructor()
    {
        base.constructor();
        rumSetGraphic(s_cGraphic);
    }
}


class Widget_Rock extends Widget
{
    static s_strDescription = "widget_rock";
    static s_cGraphic = Graphic_widget_rock;

    constructor()
    {
        base.constructor();
    }
}


class Widget_Bush extends Widget
{
    static s_strDescription = "widget_bush";
    static s_cGraphic = Graphic_widget_bush;

    constructor()
    {
        base.constructor();
    }
}
