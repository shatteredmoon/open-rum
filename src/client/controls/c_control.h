#pragma once

#include <c_input.h>
#include <u_structs.h>
#include <u_property_container.h>

class rumFont;
class rumGraphic;

enum AlignType
{
  ALIGN_LEFT,
  ALIGN_CENTER,
  ALIGN_RIGHT
};


class rumClientControl //: public rumPropertyContainer
{
public:

  void AlignCenter()
  {
    Align( ALIGN_CENTER );
  }

  void AlignLeft()
  {
    Align( ALIGN_LEFT );
  }

  void AlignRight()
  {
    Align( ALIGN_RIGHT );
  }

  virtual void Clear();

  bool ContainsPoint( const rumPoint& i_rcPoint ) const;

  uint32_t CursorAnimationAdvance();
  void CursorAnimationReset()
  {
    m_uiCursorFrame = 0;
  }

  // The definitive draw function - draws at the provided position
  virtual void Draw( const rumPoint& i_rcPos );

  // Draw using the stored position
  void Draw()
  {
    Draw( m_cPos );
  }

  const rumGraphic* FetchCursor() const;
  const rumGraphic* FetchPrompt() const;

  virtual void Focus();
  void FocusNext()
  {
    FocusNextControl();
  }

  bool HasInputFocus() const;

  rumFont* GetFont() const;
  uint32_t GetFontHeight() const;

  NativeHandle GetHandle() const
  {
    return m_iHandle;
  }

  uint32_t GetHeight() const
  {
    return m_uiPixelHeight;
  }

  uint32_t GetWidth() const
  {
    return m_uiPixelWidth;
  }

  // Gets the stored draw position
  int32_t GetPosX() const
  {
    return m_cPos.m_iX;
  }

  int32_t GetPosY() const
  {
    return m_cPos.m_iY;
  }

  const rumPoint& GetPos() const
  {
    return m_cPos;
  }

  // Sets the stored draw position
  void SetPos( const rumPoint& i_rcPos )
  {
    m_cPos = i_rcPos;
  }

  bool HandlesInput() const
  {
    return m_bHandlesInput;
  }

  bool HasCursor() const
  {
    return m_bCursor;
  }

  bool HasPrompt() const
  {
    return m_bPrompt;
  }

  Sqrat::Object GetScriptInstance();

  void Invalidate()
  {
    m_bInvalidated = true;
  }

  bool IsActive() const
  {
    return m_bActive;
  }

  // A persistent cursor will show even when the control doesn't have input focus
  bool IsCursorPersistent() const
  {
    return m_bPersistentCursor;
  }

  bool IsInvalidated() const
  {
    return m_bInvalidated;
  }

  uint32_t PromptAnimationAdvance();
  void PromptAnimationReset()
  {
    m_uiPromptFrame = 0;
  }

  void SetActive( bool i_bActive );

  void SetBackgroundColor( const rumColor& i_rcColor );
  void SetCursor( rumAssetID i_eGraphicID );
  void SetFont( const std::string& i_strFont ); // Invalidates

  void SetHeight( uint32_t i_uiHeight ); // Invalidates
  void SetWidth( uint32_t i_uiWidth ); // Invalidates

  void SetPrompt( rumAssetID i_eGraphicID );

  void SetHandlesInput( bool i_bHandlesInput )
  {
    m_bHandlesInput = i_bHandlesInput;
  }

  void SetPersistentCursor( bool i_bPersistent )
  {
    m_bPersistentCursor = i_bPersistent;
  }

  void ShowCursor( bool i_bShow );
  void ShowPrompt( bool i_bShow );

  static Sqrat::Object CreateControl( Sqrat::Object i_sqClass );

  static int32_t Init()
  {
    return RESULT_SUCCESS;
  }

  static void DisplayActiveControls();

  static rumClientControl* GetFocusedControl();
  static Sqrat::Object GetFocusedControlVM();

  static void FocusNextControl();

  static void OnInputSystemKeyPressed( const rumKeypressBase* i_pcKeypress );
  static void OnInputSystemKeyReleased( const rumKeypressBase* i_pcKeypress );
  static void OnInputSystemKeyRepeated( const rumKeypressBase* i_pcKeypress );

  static void OnInputSystemMouseButtonPressed( rumInputBase::rumMouseButton i_eMouseButton,
                                               const rumPoint& i_rcPoint );
  static void OnInputSystemMouseButtonReleased( rumInputBase::rumMouseButton i_eMouseButton,
                                                const rumPoint& i_rcPoint );
  static void OnInputSystemMouseMoved( const rumPoint& i_rcPoint );
  static void OnInputSystemMouseScrolled( const rumPoint& i_rcDirection, const rumPoint& i_rcPoint );

  static void ScriptBind();
  static void Shutdown();

  typedef std::list<NativeHandle> ActiveControlList;
  typedef std::unordered_map<NativeHandle, rumClientControl*> ControlList;

protected:

  rumClientControl();
  virtual ~rumClientControl();

  void Align( const AlignType i_eAlignment );

  template<typename... TArgs>
  void CallScriptFunc( Sqrat::Object& i_sqFunc, TArgs... i_TArgs );

  virtual void OnCreated();

  void OnKeyPressed( const rumKeypressBase* i_pcKeypress );
  void OnKeyReleased( const rumKeypressBase* i_pcKeypress );
  void OnKeyRepeated( const rumKeypressBase* i_pcKeypress );

  void OnMouseButtonPressed( rumInputBase::rumMouseButton i_eMouseButton, const rumPoint& i_rcPoint );
  void OnMouseButtonReleased( rumInputBase::rumMouseButton i_eMouseButton, const rumPoint& i_rcPoint );
  void OnMouseMoved( const rumPoint& i_rcPoint );
  void OnMouseScrolled( const rumPoint& i_rcDirection, const rumPoint& i_rcPoint );

  //void OnPropertyRemoved( rumAssetID i_ePropertyID ) override;
  //void OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded ) override;

  virtual bool Validate();

  static NativeHandle GetFreeHandle();

  NativeHandle m_iHandle{ GetFreeHandle() };

  rumAssetID m_eCursorGraphicID{ INVALID_ASSET_ID };
  rumAssetID m_ePromptGraphicID{ INVALID_ASSET_ID };

  std::string m_strDefaultFont;

  rumColor m_cBackgroundColor;

  rumGraphic* m_pcBackground{ nullptr };
  rumGraphic* m_pcDisplay{ nullptr };

  rumPoint m_cCursorOffset{ 0, 0 };

  uint32_t m_uiCursorFrame{ 0 };
  uint32_t m_uiPromptFrame{ 0 };

  AlignType m_eAlignment{ ALIGN_LEFT };

  bool m_bCursor{ false };
  bool m_bPrompt{ false };

private:

  uint32_t m_uiPixelWidth{ 0 };
  uint32_t m_uiPixelHeight{ 0 };

  rumPoint m_cPos{ 0, 0 };

  Sqrat::Object m_sqInstance;

  rumTimer m_cCursorAnimationTimer;
  float m_fCursorAnimationInterval{ 0.2f };

  rumTimer m_cPromptAnimationTimer;
  float m_fPromptAnimationInterval{ 0.2f };

  bool m_bActive{ false };
  bool m_bHandlesInput{ true };
  bool m_bInvalidated{ true };
  bool m_bPersistentCursor{ false };

  static ActiveControlList s_listActiveControls;
  static ControlList s_hashControls;

  static NativeHandle s_iFocusedHandle;
};

#include <controls/c_control.inl>
