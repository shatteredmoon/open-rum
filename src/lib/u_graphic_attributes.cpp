#include <u_graphic_attributes.h>


// static
void rumGraphicAttributes::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumGraphicAttributes> cGraphicAttributes( pcVM, "rumGraphicAttributes" );
  cGraphicAttributes
    .Var( "DrawMasked", &rumGraphicAttributes::m_bDrawMasked )
    .Var( "DrawScaled", &rumGraphicAttributes::m_bDrawScaled )
    .Var( "AnimationFrame", &rumGraphicAttributes::m_uiAnimationFrame )
    .Var( "AnimationSet", &rumGraphicAttributes::m_uiAnimationSet )
    .Var( "TransparentLevel", &rumGraphicAttributes::m_uiTransparentLevel )
    .Var( "HorizontalScale", &rumGraphicAttributes::m_fHorizontalScale )
    .Var( "VerticalScale", &rumGraphicAttributes::m_fVerticalScale );
  Sqrat::RootTable( pcVM ).Bind( "rumGraphicAttributes", cGraphicAttributes );

  Sqrat::ConstTable( pcVM )
    .Const( "rumBlendType_None", BlendType_None )
    .Const( "rumBlendType_Translucent", BlendType_Translucent )
    //.Const( "rumBlendType_Screen", BlendType_Screen )
    //.Const( "rumBlendType_Add", BlendType_Add )
    //.Const( "rumBlendType_Burn", BlendType_Burn )
    //.Const( "rumBlendType_Color", BlendType_Color )
    //.Const( "rumBlendType_Difference", BlendType_Difference )
    //.Const( "rumBlendType_Dissolve", BlendType_Dissolve )
    //.Const( "rumBlendType_Dodge", BlendType_Dodge )
    //.Const( "rumBlendType_Hue", BlendType_Hue )
    //.Const( "rumBlendType_Invert", BlendType_Invert )
    //.Const( "rumBlendType_Luminance", BlendType_Luminance )
    .Const( "rumBlendType_Multiply", BlendType_Multiply );
    //.Const( "rumBlendType_Saturation", BlendType_Saturation );
}
