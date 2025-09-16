#ifndef _U_GRAPHIC_ATTRIBUTES_H_
#define _U_GRAPHIC_ATTRIBUTES_H_

#include <u_script.h>
#include <u_structs.h>


class rumGraphicAttributes
{
public:

  enum BlendType
  {
    BlendType_None,
    BlendType_Translucent,
    //BlendType_Screen,
    //BlendType_Add,
    //BlendType_Burn,
    //BlendType_Color,
    //BlendType_Difference,
    //BlendType_Dissolve,
    //BlendType_Dodge,
    //BlendType_Hue,
    //BlendType_Invert,
    //BlendType_Luminance,
    BlendType_Multiply,
    //BlendType_Saturation
  };

  static void ScriptBind();

  uint32_t m_uiAnimationFrame{ 0 };
  uint32_t m_uiAnimationSet{ 0 };

  uint32_t m_uiTransparentLevel{ RUM_ALPHA_OPAQUE };

  float m_fLightLevel{ 1.f };

  float m_fHorizontalScale{ 1.f };
  float m_fVerticalScale{ 1.f };

  // The blend type - add, mulitply, transparent, etc.
  BlendType m_eBlendType{ BlendType_None };

  // The color that temporary buffers are cleared to for blending operations
  rumColor m_cBufferColor{ rumColor::s_cBlackOpaque };

  // The color setting used by blend actions
  rumColor m_cBlendColor{ rumColor::s_cWhiteOpaque };

  // Restores the original images alpha values after a blend operation (only if DrawMasked = true)
  bool m_bRestoreAlphaPostBlend{ false };

  bool m_bDrawMasked{ true };
  bool m_bDrawScaled{ false };
  bool m_bDrawLit{ false };
  bool m_bHidden{ false };
};

#endif // _U_GRAPHIC_ATTRIBUTES_H_
