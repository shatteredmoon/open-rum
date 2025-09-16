#include <controls/c_control.h>

#include <c_font.h>
#include <controls/c_label.h>
#include <controls/c_listview.h>
#include <controls/c_region.h>
#include <controls/c_slider.h>
#include <controls/c_textbox.h>
#include <controls/c_textview.h>

#include <u_utility.h>

#include <algorithm>

#define FIRST_FREE_HANDLE 1

// Static initializations
rumClientControl::ActiveControlList rumClientControl::s_listActiveControls;
rumClientControl::ControlList rumClientControl::s_hashControls;
NativeHandle rumClientControl::s_iFocusedHandle{ RUM_INVALID_NATIVEHANDLE };


rumClientControl::rumClientControl()
  : m_strDefaultFont( rumFont::GetDefaultFontName() )
  , m_cBackgroundColor( rumColor::s_cBlackTransparent )
{
  // Add the new control to the end of the list
  s_hashControls.insert( { m_iHandle, this } );

  m_pcBackground = rumGraphic::Create();
  m_pcDisplay = rumGraphic::Create();
}


rumClientControl::~rumClientControl()
{
  s_listActiveControls.remove( GetHandle() );

  m_pcBackground->Free();
  m_pcDisplay->Free();
}


void rumClientControl::Align( const AlignType i_eAlignment )
{
  if( i_eAlignment != m_eAlignment )
  {
    m_eAlignment = i_eAlignment;
    Invalidate();
  }
}


// virtual
void rumClientControl::Clear()
{
  Invalidate();
}


bool rumClientControl::ContainsPoint( const rumPoint& i_rcPoint ) const
{
  if( GetWidth() > 0 && GetHeight() > 0 )
  {
    const rumRectangle cBounds( GetPos(), GetWidth(), GetHeight() );
    return cBounds.Contains( i_rcPoint );
  }

  return false;
}


// static
Sqrat::Object rumClientControl::CreateControl( Sqrat::Object i_sqClass )
{
  Sqrat::Table sqParams;
  Sqrat::Object sqInstance{ rumScript::CreateInstance( i_sqClass, sqParams ) };

  if( sqInstance.GetType() == OT_INSTANCE )
  {
    rumClientControl* pcControl{ sqInstance.Cast<rumClientControl*>() };
    if( pcControl )
    {
      pcControl->m_sqInstance = rumScript::GetWeakReference( sqInstance );
      pcControl->OnCreated();
    }
    else
    {
      std::string strError{ "Failed to fetch script instance user pointer for rumClientControl" };
      Logger::LogStandard( strError, Logger::LOG_ERROR );
    }
  }
  else
  {
    std::string strError{ "Failed to create a script instance for rumClientControl" };
    Logger::LogStandard( strError, Logger::LOG_ERROR );
  }

  return sqInstance;
}


uint32_t rumClientControl::CursorAnimationAdvance()
{
  rumGraphic* pcGraphic{ rumGraphic::Fetch( m_eCursorGraphicID ) };
  if( pcGraphic )
  {
    m_uiCursorFrame = pcGraphic->AnimationAdvance( m_uiCursorFrame + 1 );
  }

  return m_uiCursorFrame;
}


// static
void rumClientControl::DisplayActiveControls()
{
  auto r_iter{ s_listActiveControls.rbegin() };
  while( r_iter != s_listActiveControls.rend() )
  {
    const auto iter{ s_hashControls.find( *r_iter ) };
    if( iter != s_hashControls.end() && iter->second != nullptr )
    {
      iter->second->Draw();
    }

    ++r_iter;
  }
}


void rumClientControl::Draw( const rumPoint& i_rcPos )
{
  if( HasPrompt() &&
      HasInputFocus() &&
      m_fPromptAnimationInterval > 0.0f &&
      m_cPromptAnimationTimer.GetElapsedSeconds() > m_fPromptAnimationInterval )
  {
    m_cPromptAnimationTimer.Restart();
    PromptAnimationAdvance();
  }

  if( m_bInvalidated )
  {
    Validate();
  }

  if( m_cBackgroundColor.GetAlpha() != RUM_ALPHA_OPAQUE )
  {
    rumClientGraphicBase::GetBackBuffer().BlitTransparent( *m_pcBackground, i_rcPos, m_cBackgroundColor.GetAlpha() );
  }
  else
  {
    rumClientGraphicBase::GetBackBuffer().Blit( *m_pcBackground, rumPoint(), i_rcPos, m_pcBackground->GetWidth(),
                                                m_pcBackground->GetHeight() );
  }

  rumClientGraphicBase::GetBackBuffer().BlitAlpha( *m_pcDisplay, rumPoint(), i_rcPos, m_pcDisplay->GetWidth(),
                                                   m_pcDisplay->GetHeight(), false /* src alpha */ );

  if( HasCursor() && ( HasInputFocus() || IsCursorPersistent() ) )
  {
    if( m_fCursorAnimationInterval > 0.0f && m_cCursorAnimationTimer.GetElapsedSeconds() > m_fCursorAnimationInterval )
    {
      m_cCursorAnimationTimer.Restart();
      CursorAnimationAdvance();
    }

    const rumGraphic* pcCursor{FetchCursor()};
    if( pcCursor )
    {
      pcCursor->DrawAnimation( rumPoint( m_cCursorOffset.m_iX + i_rcPos.m_iX, m_cCursorOffset.m_iY + i_rcPos.m_iY ),
                               0, m_uiCursorFrame );
    }
  }
}


const rumGraphic* rumClientControl::FetchCursor() const
{
  return rumGraphic::Fetch( m_eCursorGraphicID );
}


const rumGraphic* rumClientControl::FetchPrompt() const
{
  return rumGraphic::Fetch( m_ePromptGraphicID );
}


void rumClientControl::Focus()
{
  if( GetHandle() == s_iFocusedHandle )
  {
    return;
  }

  if( !HandlesInput() )
  {
    return;
  }

  // Remove focus from the current control
  rumClientControl* pcControl{ GetFocusedControl() };
  if( pcControl )
  {
    pcControl->Invalidate();
  }

  s_iFocusedHandle = GetHandle();

  RUM_COUT_IFDEF_DBG( CONTROL_DEBUG, "Focused control: " << s_iFocusedHandle << '\n' );

  Invalidate();
}


// static
void rumClientControl::FocusNextControl()
{
  if( s_listActiveControls.size() <= 1 )
  {
    return;
  }

  bool bFullSearch{ false };

  auto iter{ std::find( s_listActiveControls.begin(), s_listActiveControls.end(), s_iFocusedHandle ) };
  if( s_listActiveControls.end() == iter )
  {
    // Start over at the front of the list
    bFullSearch = true;
    iter = s_listActiveControls.begin();
  }
  else
  {
    // Advance to next
    ++iter;
  }

  s_iFocusedHandle = RUM_INVALID_NATIVEHANDLE;

  const auto nextIter{ std::find_if( iter, s_listActiveControls.end(),
                                     []( auto iHandle )
                                     {
                                       // Return true if the control matching the handle can take input
                                       const auto foundIter{ s_hashControls.find( iHandle ) };
                                       if( foundIter != s_hashControls.end() )
                                       {
                                         const auto* pcControl{ foundIter->second };
                                         if( pcControl && pcControl->HandlesInput() )
                                         {
                                           return true;
                                         }
                                       }

                                       return false;
                                     } ) };
  if( nextIter != s_listActiveControls.end() )
  {
    s_iFocusedHandle = *nextIter;
    RUM_COUT_IFDEF_DBG( CONTROL_DEBUG, "Focused control: " << s_iFocusedHandle << '\n' );
  }

  if( !bFullSearch && ( RUM_INVALID_NATIVEHANDLE == s_iFocusedHandle ) )
  {
    // Try again for a full search
    FocusNextControl();
  }
}


// static
rumClientControl* rumClientControl::GetFocusedControl()
{
  if( s_listActiveControls.empty() )
  {
    return nullptr;
  }

  const auto iter{ s_hashControls.find( s_iFocusedHandle ) };
  return ( iter != s_hashControls.end() ? iter->second : nullptr );
}


// static
Sqrat::Object rumClientControl::GetFocusedControlVM()
{
  auto* pcControl{ GetFocusedControl() };
  if( pcControl )
  {
    return pcControl->GetScriptInstance();
  }

  return Sqrat::Object();
}


rumFont* rumClientControl::GetFont() const
{
  rumFont* pcFont{ rumFont::Get( m_strDefaultFont ) };
  return ( !pcFont ? rumFont::GetDefault() : pcFont );
}


uint32_t rumClientControl::GetFontHeight() const
{
  const rumFont* pcFont{ GetFont() };
  return ( pcFont ? pcFont->GetPixelHeight() : 0 );
}


// static
NativeHandle rumClientControl::GetFreeHandle()
{
  static NativeHandle iFreeHandle{ FIRST_FREE_HANDLE };
  return iFreeHandle++;
}


Sqrat::Object rumClientControl::GetScriptInstance()
{
  if( m_sqInstance.GetType() == OT_INSTANCE )
  {
    return m_sqInstance;
  }
  else if( m_sqInstance.GetType() == OT_WEAKREF )
  {
    return rumScript::GetWeakReferenceValue( m_sqInstance );
  }

  return Sqrat::Object();
}


bool rumClientControl::HasInputFocus() const
{
  return ( GetHandle() == s_iFocusedHandle );
}


// virtual
void rumClientControl::OnCreated()
{
  if( m_sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( GetScriptInstance(), "OnControlCreated" );
  }
}


// static
void rumClientControl::OnInputSystemKeyPressed( const rumKeypressBase* i_pcKeypress )
{
  auto* pcControl{ GetFocusedControl() };
  if( pcControl )
  {
    pcControl->OnKeyPressed( i_pcKeypress );
  }
}


// static
void rumClientControl::OnInputSystemKeyReleased( const rumKeypressBase* i_pcKeypress )
{
  auto* pcControl{ GetFocusedControl() };
  if( pcControl )
  {
    pcControl->OnKeyReleased( i_pcKeypress );
  }
}


// static
void rumClientControl::OnInputSystemKeyRepeated( const rumKeypressBase* i_pcKeypress )
{
  auto* pcControl{ GetFocusedControl() };
  if( pcControl )
  {
    pcControl->OnKeyRepeated( i_pcKeypress );
  }
}


// static
void rumClientControl::OnInputSystemMouseButtonPressed( rumInputBase::rumMouseButton i_eMouseButton,
                                                        const rumPoint& i_rcPoint )
{
  // TODO - bring the control into focus and handle the button press
  for( auto iter : s_hashControls )
  {
    auto* pcControl{ iter.second };
    if( pcControl && pcControl->IsActive() && pcControl->ContainsPoint( i_rcPoint ) )
    {
      pcControl->OnMouseButtonPressed( i_eMouseButton, i_rcPoint );
    }
  }
}


// static
void rumClientControl::OnInputSystemMouseButtonReleased( rumInputBase::rumMouseButton i_eMouseButton,
                                                         const rumPoint& i_rcPoint )
{
  // TODO - bring the control into focus and handle the button press
  for( auto iter : s_hashControls )
  {
    auto* pcControl{ iter.second };
    if( pcControl && pcControl->IsActive() && pcControl->ContainsPoint( i_rcPoint ) )
    {
      pcControl->OnMouseButtonReleased( i_eMouseButton, i_rcPoint );
    }
  }
}


// static
void rumClientControl::OnInputSystemMouseMoved( const rumPoint& i_rcPoint )
{
  for( auto iter : s_hashControls )
  {
    auto* pcControl{ iter.second };
    if( pcControl && pcControl->IsActive() && pcControl->ContainsPoint( i_rcPoint ) )
    {
      pcControl->OnMouseMoved( i_rcPoint );
    }
  }
}


// static
void rumClientControl::OnInputSystemMouseScrolled( const rumPoint& i_rcDirection, const rumPoint& i_rcPoint )
{
  for( auto iter : s_hashControls )
  {
    auto* pcControl{ iter.second };
    if( pcControl && pcControl->IsActive() && pcControl->ContainsPoint( i_rcPoint ) )
    {
      pcControl->OnMouseScrolled( i_rcDirection, i_rcPoint );
    }
  }
}


void rumClientControl::OnKeyPressed( const rumKeypressBase* i_pcKeypress )
{
  if( m_sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( GetScriptInstance(), "OnKeyPressed", i_pcKeypress );
  }
}


void rumClientControl::OnKeyReleased( const rumKeypressBase* i_pcKeypress )
{
  if( m_sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( GetScriptInstance(), "OnKeyReleased", i_pcKeypress );
  }
}


void rumClientControl::OnKeyRepeated( const rumKeypressBase* i_pcKeypress )
{
  if( m_sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( GetScriptInstance(), "OnKeyRepeated", i_pcKeypress );
  }
}


void rumClientControl::OnMouseButtonPressed( rumInputBase::rumMouseButton i_eMouseButton, const rumPoint& i_rcPoint )
{
  if( m_sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( GetScriptInstance(),
                                 "OnMouseButtonPressed",
                                 rumUtility::ToUnderlyingType( i_eMouseButton ),
                                 i_rcPoint );
  }
}


void rumClientControl::OnMouseButtonReleased( rumInputBase::rumMouseButton i_eMouseButton, const rumPoint& i_rcPoint )
{
  if( m_sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( GetScriptInstance(),
                                 "OnMouseButtonReleased",
                                 rumUtility::ToUnderlyingType( i_eMouseButton ),
                                 i_rcPoint );
  }
}


void rumClientControl::OnMouseMoved( const rumPoint& i_rcPoint )
{
  if( m_sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( GetScriptInstance(), "OnMouseMoved", i_rcPoint );
  }
}


void rumClientControl::OnMouseScrolled( const rumPoint& i_rcDirection, const rumPoint& i_rcPoint )
{
  if( m_sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( GetScriptInstance(), "OnMouseScrolled", i_rcDirection, i_rcPoint );
  }
}


/*
// override
void rumClientControl::OnPropertyRemoved( rumAssetID i_ePropertyID )
{
  Sqrat::Object sqInstance{ GetScriptInstance() };
  if( sqInstance.GetType() == OT_INSTANCE )
  {
    rumScript::ExecOptionalFunc( sqInstance, "OnPropertyRemoved", i_ePropertyID );
  }
}


// override
void rumClientControl::OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded )
{
  Sqrat::Object sqInstance{ GetScriptInstance() };
  if( sqInstance.GetType() == OT_INSTANCE )
  {
    rumScript::ExecOptionalFunc( sqInstance, "OnPropertyUpdated", i_ePropertyID, i_sqValue );
  }
}
*/


uint32_t rumClientControl::PromptAnimationAdvance()
{
  rumGraphic* pcGraphic{ rumGraphic::Fetch( m_ePromptGraphicID ) };
  if( pcGraphic )
  {
    m_uiPromptFrame = pcGraphic->AnimationAdvance( m_uiPromptFrame + 1 );
  }

  return m_uiPromptFrame;
}


// static
void rumClientControl::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumCreateControl", CreateControl )
    .Func( "rumDisplayActiveControls", DisplayActiveControls )
    .Func( "rumFocusNextControl", FocusNextControl )
    .Func( "rumGetFocusedControl", &GetFocusedControlVM );

  //Sqrat::DerivedClass< rumClientControl, rumPropertyContainer, Sqrat::NoConstructor<rumClientControl> >
  Sqrat::Class< rumClientControl, Sqrat::NoConstructor<rumClientControl> >
    cClientControl( pcVM, "rumClientControl" );
  cClientControl
    .Func( "AlignCenter", &AlignCenter )
    .Func( "AlignLeft", &AlignLeft )
    .Func( "AlignRight", &AlignRight )
    .Func( "Clear", &Clear )
    .Func( "ContainsPoint", &ContainsPoint )
    .Func( "CursorAnimationAdvance", &CursorAnimationAdvance )
    .Func( "Focus", &Focus )
    .Func( "FocusNext", &FocusNext )
    .Func( "GetHeight", &GetHeight )
    .Func( "GetWidth", &GetWidth )
    .Func( "GetPosX", &GetPosX )
    .Func( "GetPosY", &GetPosY )
    .Func( "GetPos", &GetPos )
    .Func( "HasInputFocus", &HasInputFocus )
    .Func( "IsActive", &IsActive )
    .Func( "PromptAnimationAdvance", &PromptAnimationAdvance )
    .Func( "SetCursor", &SetCursor )
    .Func( "SetPrompt", &SetPrompt )
    .Func( "SetActive", &SetActive )
    .Func( "SetBackgroundColor", &SetBackgroundColor )
    .Func( "SetHandlesInput", &SetHandlesInput )
    .Func( "SetHeight", &SetHeight )
    .Func( "SetPersistentCursor", &SetPersistentCursor )
    .Func( "SetPos", &SetPos )
    .Func( "SetWidth", &SetWidth )
    .Func( "ShowCursor", &ShowCursor )
    .Func( "ShowPrompt", &ShowPrompt )
    .Overload<void( rumClientControl::* )( void )>( "Display", &Draw )
    .Overload<void( rumClientControl::* )( const rumPoint& )>( "Display", &Draw );
  Sqrat::RootTable( pcVM ).Bind( "rumControl", cClientControl );

  rumClientScrollBuffer::ScriptBind();
  rumClientListView::ScriptBind();
  rumClientTextBox::ScriptBind();
  rumClientTextView::ScriptBind();
  rumClientLabel::ScriptBind();
  rumClientRegion::ScriptBind();
  rumClientSlider::ScriptBind();
}


void rumClientControl::SetActive( bool i_bActive )
{
  if( i_bActive == m_bActive )
  {
    return;
  }

  m_bActive = i_bActive;

  if( i_bActive )
  {
    s_listActiveControls.push_front( GetHandle() );
  }
  else
  {
    s_listActiveControls.remove( GetHandle() );
  }

#if CONTROL_DEBUG
  RUM_COUT( "Active controls: " );
  for( const auto iHandle : s_listActiveControls )
  {
    RUM_COUT( iHandle << " " );
  }
  RUM_COUT( '\n' );
#endif // CONTROL_DEBUG
}


void rumClientControl::SetBackgroundColor( const rumColor& i_rcColor )
{
  if( m_cBackgroundColor.GetRGBA() != i_rcColor.GetRGBA() )
  {
    m_cBackgroundColor = i_rcColor;

    // Clear now, no reason to invalidate the control
    m_pcBackground->Clear( m_cBackgroundColor );
  }
}


void rumClientControl::SetCursor( rumAssetID i_eGraphicID )
{
  rumGraphic* pcGraphic{ rumGraphic::Fetch( i_eGraphicID ) };
  if( pcGraphic )
  {
    m_eCursorGraphicID = i_eGraphicID;
    if( !pcGraphic->HasAlpha() )
    {
      pcGraphic->GenerateAlpha();
    }

    CursorAnimationReset();
  }
}


void rumClientControl::SetFont( const std::string& i_strFont )
{
  if( strcasecmp( m_strDefaultFont.c_str(), i_strFont.c_str() ) != 0 )
  {
    m_strDefaultFont = i_strFont;
    m_bInvalidated = true;
  }
}


void rumClientControl::SetHeight( uint32_t i_uiHeight )
{
  if( i_uiHeight > 0 )
  {
    if( m_uiPixelHeight != i_uiHeight )
    {
      m_uiPixelHeight = i_uiHeight;
      m_bInvalidated = true;
    }
  }
}


void rumClientControl::SetPrompt( rumAssetID i_eGraphicID )
{
  rumGraphic* pcGraphic{ rumGraphic::Fetch( i_eGraphicID ) };
  if( pcGraphic )
  {
    m_ePromptGraphicID = i_eGraphicID;
    if( !pcGraphic->HasAlpha() )
    {
      pcGraphic->GenerateAlpha();
    }

    PromptAnimationReset();
  }
}


void rumClientControl::SetWidth( uint32_t i_uiWidth )
{
  if( i_uiWidth > 0 )
  {
    if( m_uiPixelWidth != i_uiWidth )
    {
      m_uiPixelWidth = i_uiWidth;
      m_bInvalidated = true;
    }
  }
}


void rumClientControl::ShowCursor( bool i_bShow )
{
  if( m_bCursor != i_bShow )
  {
    m_bCursor = i_bShow;
    Invalidate();
  }
}


void rumClientControl::ShowPrompt( bool i_bShow )
{
  if( m_bPrompt != i_bShow )
  {
    m_bPrompt = i_bShow;
    Invalidate();
  }
}


// static
void rumClientControl::Shutdown()
{
  for( auto iter : s_hashControls )
  {
    delete iter.second;
    iter.second = nullptr;
  }
}


bool rumClientControl::Validate()
{
  bool bResult{ m_pcBackground->InitData( m_uiPixelWidth, m_uiPixelHeight ) };
  m_pcBackground->Clear( m_cBackgroundColor );

  // Recreate the surfaces
  if( m_pcDisplay->InitData( m_uiPixelWidth, m_uiPixelHeight ) )
  {
    m_pcDisplay->Clear( rumColor::s_cBlackTransparent );
    bResult &= true;
  }

  m_bInvalidated = false;

  return bResult;
}
