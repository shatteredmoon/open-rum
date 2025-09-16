// Defined in the main project CMakeLists.txt
#ifdef USE_GLFW

#include <c_input_glfw.h>

#include <c_graphics.h>

#include <u_assert.h>
#include <u_log.h>
#include <u_utility.h>

#ifdef WIN32
#include <Windows.h>
#endif // WIN32

#include <GLFW/glfw3.h>

GLFWwindow* rumInput::s_pcWindow{ nullptr };
GLFWcursorposfun rumInput::s_funcPrevUserCallbackCursorPos{ nullptr };
GLFWcursorenterfun rumInput::s_funcPrevUserCallbackCursorEnter{ nullptr };
GLFWmousebuttonfun rumInput::s_funcPrevUserCallbackMousebutton{ nullptr };
GLFWscrollfun rumInput::s_funcPrevUserCallbackScroll{ nullptr };
GLFWkeyfun rumInput::s_funcPrevUserCallbackKey{ nullptr };
GLFWcharfun rumInput::s_funcPrevUserCallbackChar{ nullptr };


// override
rumPoint rumInput::GetMousePosition() const
{
  double fXPos, fYPos;
  glfwGetCursorPos( s_pcWindow, &fXPos, &fYPos );

  if( rumClientGraphicBase::IsFullscreen() )
  {
    return { static_cast<int32_t>( fXPos * ( DEFAULT_SCREEN_WIDTH / static_cast<double>( rumClientGraphicBase::GetFullScreenWidth() ) ) ),
             static_cast<int32_t>( fYPos * ( DEFAULT_SCREEN_HEIGHT / static_cast<double>( rumClientGraphicBase::GetFullScreenHeight() )) ) };
  }

  return { static_cast<int32_t>( fXPos * ( DEFAULT_SCREEN_WIDTH / static_cast<double>( rumClientGraphicBase::GetWindowedScreenWidth() )) ),
           static_cast<int32_t>( fYPos * ( DEFAULT_SCREEN_HEIGHT / static_cast<double>( rumClientGraphicBase::GetWindowedScreenHeight() )) ) };
}


// override
bool rumInput::IsKeyPressed( int32_t i_iKey ) const
{
  const int32_t eState{ glfwGetKey( s_pcWindow, i_iKey ) };
  return ( GLFW_PRESS == eState );
}


// static
void rumInput::KeyPressCallback( GLFWwindow* i_pcWindow, int32_t i_iKey, int32_t i_iScancode, int32_t i_iAction,
                                 int32_t i_iMods )
{
  if( s_funcPrevUserCallbackKey )
  {
    s_funcPrevUserCallbackKey( i_pcWindow, i_iKey, i_iScancode, i_iAction, i_iMods );
  }

  if( GLFW_RELEASE == i_iAction )
  {
    // Ignore release events
    return;
  }

  if( s_pcInstance )
  {
    RUM_COUT_IFDEF( INPUT_DEBUG, "Key pressed: " << i_iKey << "|" << i_iScancode << "|" << i_iAction << "|" << i_iMods << '\n' );

    rumKeypress cKeypress;
    cKeypress.SetValue( i_iKey, i_iKey, i_iScancode, i_iMods );

    s_cLastKeypress = cKeypress;

    if( GLFW_PRESS == i_iAction )
    {
      s_pcInstance->OnKeyPressed( &cKeypress );
    }
    else if( GLFW_RELEASE == i_iAction )
    {
      s_pcInstance->OnKeyReleased( &cKeypress );
    }
    else if( GLFW_REPEAT == i_iAction )
    {
      s_pcInstance->OnKeyRepeated( &cKeypress );
    }
  }
}


// static
void rumInput::MouseButtonCallback( GLFWwindow* i_pcWindow, int32_t i_iButton, int32_t i_iAction, int32_t i_iMods )
{
  if( s_funcPrevUserCallbackMousebutton )
  {
    s_funcPrevUserCallbackMousebutton( i_pcWindow, i_iButton, i_iAction, i_iMods );
  }

  if( s_pcInstance )
  {
    // Mods (GLFW_MOD_SHIFT, etc.) aren't supported
    if( GLFW_RELEASE == i_iAction )
    {
      s_pcInstance->OnMouseButtonReleased( static_cast<rumInputBase::rumMouseButton>( i_iButton ),
                                           s_pcInstance->GetMousePosition() );
    }
    else if( GLFW_PRESS == i_iAction )
    {
      s_pcInstance->OnMouseButtonPressed( static_cast<rumInputBase::rumMouseButton>( i_iButton ),
                                          s_pcInstance->GetMousePosition() );
    }
  }
}


// static
void rumInput::MousePositionCallback( GLFWwindow* i_pcWindow, double i_fXPos, double i_fYPos )
{
  if( s_funcPrevUserCallbackCursorPos )
  {
    s_funcPrevUserCallbackCursorPos( i_pcWindow, i_fXPos, i_fYPos );
  }

  if( s_pcInstance )
  {
    if( rumClientGraphicBase::IsFullscreen() )
    {
      s_pcInstance->OnMouseMoved( { static_cast<int32_t>( i_fXPos * ( DEFAULT_SCREEN_WIDTH /
                                                                      static_cast<double>( rumClientGraphicBase::GetFullScreenWidth() ) ) ),
                                    static_cast<int32_t>( i_fYPos * ( DEFAULT_SCREEN_HEIGHT /
                                                                      static_cast<double>( rumClientGraphicBase::GetFullScreenHeight() ) ) ) } );
    }
    else
    {
      s_pcInstance->OnMouseMoved( { static_cast<int32_t>( i_fXPos * ( DEFAULT_SCREEN_WIDTH /
                                                                      static_cast<double>( rumClientGraphicBase::GetWindowedScreenWidth() ) ) ),
                                    static_cast<int32_t>( i_fYPos * ( DEFAULT_SCREEN_HEIGHT /
                                                                      static_cast<double>( rumClientGraphicBase::GetWindowedScreenHeight() ) ) ) } );
    }
  }
}


// static
void rumInput::MouseScrollCallback( GLFWwindow* i_pcWindow, double i_fXOffset, double i_fYOffset )
{
  if( s_funcPrevUserCallbackScroll )
  {
    s_funcPrevUserCallbackScroll( i_pcWindow, i_fXOffset, i_fYOffset );
  }

  if( s_pcInstance )
  {
    s_pcInstance->OnMouseScrolled( { static_cast<int32_t>( i_fXOffset ), static_cast<int32_t>( i_fYOffset ) },
                                   s_pcInstance->GetMousePosition() );
  }
}


// static
int32_t rumInput::Init()
{
  // Create the singleton object
  rumInput* pcInput{ new rumInput };
  s_pcInstance = pcInput;

  // GLFW input requires a pointer to the window. This should have already been set by the graphic system.
  rumAssert( s_pcWindow );

  if( s_pcWindow )
  {
    // Register new callbacks, but remember the previously set callbacks so that they can also be called
    s_funcPrevUserCallbackCursorPos = glfwSetCursorPosCallback( s_pcWindow, MousePositionCallback );
    s_funcPrevUserCallbackMousebutton = glfwSetMouseButtonCallback( s_pcWindow, MouseButtonCallback );
    s_funcPrevUserCallbackScroll = glfwSetScrollCallback( s_pcWindow, MouseScrollCallback );
    s_funcPrevUserCallbackKey = glfwSetKeyCallback( s_pcWindow, KeyPressCallback );

    return RESULT_SUCCESS;
  }

  return RESULT_FAILED;
}


// static
void rumInput::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumInput, rumInputBase> cInput( pcVM, "rumInput" );
  Sqrat::RootTable( pcVM ).Bind( "rumInput", cInput );

  rumKeypress::ScriptBind();
}


// static
void rumInput::SetWindow( GLFWwindow* i_pcWindow )
{
  s_pcWindow = i_pcWindow;
}


// static
void rumInput::Shutdown()
{
  RUM_COUT( "Shutting down GLFW Input\n" );
  s_pcWindow = nullptr;
}


// static
void rumInput::Update()
{
  glfwPollEvents();
}


// staic
void rumKeypress::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumKeypress, rumKeypressBase> cKeypress( pcVM, "rumKeypress" );
  cKeypress
    .StaticFunc( "KeyA", &KeyA )
    .StaticFunc( "KeyB", &KeyB )
    .StaticFunc( "KeyC", &KeyC )
    .StaticFunc( "KeyD", &KeyD )
    .StaticFunc( "KeyE", &KeyE )
    .StaticFunc( "KeyF", &KeyF )
    .StaticFunc( "KeyG", &KeyG )
    .StaticFunc( "KeyH", &KeyH )
    .StaticFunc( "KeyI", &KeyI )
    .StaticFunc( "KeyJ", &KeyJ )
    .StaticFunc( "KeyK", &KeyK )
    .StaticFunc( "KeyL", &KeyL )
    .StaticFunc( "KeyM", &KeyM )
    .StaticFunc( "KeyN", &KeyN )
    .StaticFunc( "KeyO", &KeyO )
    .StaticFunc( "KeyP", &KeyP )
    .StaticFunc( "KeyQ", &KeyQ )
    .StaticFunc( "KeyR", &KeyR )
    .StaticFunc( "KeyS", &KeyS )
    .StaticFunc( "KeyT", &KeyT )
    .StaticFunc( "KeyU", &KeyU )
    .StaticFunc( "KeyV", &KeyV )
    .StaticFunc( "KeyW", &KeyW )
    .StaticFunc( "KeyX", &KeyX )
    .StaticFunc( "KeyY", &KeyY )
    .StaticFunc( "KeyZ", &KeyZ )

    .StaticFunc( "Key0", &Key0 )
    .StaticFunc( "Key1", &Key1 )
    .StaticFunc( "Key2", &Key2 )
    .StaticFunc( "Key3", &Key3 )
    .StaticFunc( "Key4", &Key4 )
    .StaticFunc( "Key5", &Key5 )
    .StaticFunc( "Key6", &Key6 )
    .StaticFunc( "Key7", &Key7 )
    .StaticFunc( "Key8", &Key8 )
    .StaticFunc( "Key9", &Key9 )

    .StaticFunc( "KeyPad0", &KeyPad0 )
    .StaticFunc( "KeyPad1", &KeyPad1 )
    .StaticFunc( "KeyPad2", &KeyPad2 )
    .StaticFunc( "KeyPad3", &KeyPad3 )
    .StaticFunc( "KeyPad4", &KeyPad4 )
    .StaticFunc( "KeyPad5", &KeyPad5 )
    .StaticFunc( "KeyPad6", &KeyPad6 )
    .StaticFunc( "KeyPad7", &KeyPad7 )
    .StaticFunc( "KeyPad8", &KeyPad8 )
    .StaticFunc( "KeyPad9", &KeyPad9 )

    .StaticFunc( "KeyF1",  &KeyF1 )
    .StaticFunc( "KeyF2",  &KeyF2 )
    .StaticFunc( "KeyF3",  &KeyF3 )
    .StaticFunc( "KeyF4",  &KeyF4 )
    .StaticFunc( "KeyF5",  &KeyF5 )
    .StaticFunc( "KeyF6",  &KeyF6 )
    .StaticFunc( "KeyF7",  &KeyF7 )
    .StaticFunc( "KeyF8",  &KeyF8 )
    .StaticFunc( "KeyF9",  &KeyF9 )
    .StaticFunc( "KeyF10", &KeyF10 )
    .StaticFunc( "KeyF11", &KeyF11 )
    .StaticFunc( "KeyF12", &KeyF12 )

    .StaticFunc( "KeyLeftAlt",      &KeyLeftAlt      )
    .StaticFunc( "KeyRightAlt",     &KeyRightAlt     )
    .StaticFunc( "KeyLeftControl",  &KeyLeftControl  )
    .StaticFunc( "KeyRightControl", &KeyRightControl )
    .StaticFunc( "KeyLeftShift",    &KeyLeftShift    )
    .StaticFunc( "KeyRightShift",   &KeyRightShift   )
    .StaticFunc( "KeyEscape",       &KeyEscape       )
    .StaticFunc( "KeyDash",         &KeyDash         )
    .StaticFunc( "KeyEquals",       &KeyEquals       )
    .StaticFunc( "KeyBackspace",    &KeyBackspace    )
    .StaticFunc( "KeyTab",          &KeyTab          )
    .StaticFunc( "KeyOpenBracket",  &KeyOpenBracket  )
    .StaticFunc( "KeyCloseBracket", &KeyCloseBracket )
    .StaticFunc( "KeyEnter",        &KeyEnter        )
    .StaticFunc( "KeyBackslash",    &KeyBackslash    )
    .StaticFunc( "KeyComma",        &KeyComma        )
    .StaticFunc( "KeyPeriod",       &KeyPeriod       )
    .StaticFunc( "KeySlash",        &KeySlash        )
    .StaticFunc( "KeySpace",        &KeySpace        )
    .StaticFunc( "KeyInsert",       &KeyInsert       )
    .StaticFunc( "KeyDelete",       &KeyDelete       )
    .StaticFunc( "KeyHome",         &KeyHome         )
    .StaticFunc( "KeyEnd",          &KeyEnd          )
    .StaticFunc( "KeyPageUp",       &KeyPageUp       )
    .StaticFunc( "KeyPageDown",     &KeyPageDown     )
    .StaticFunc( "KeyLeft",         &KeyLeft         )
    .StaticFunc( "KeyRight",        &KeyRight        )
    .StaticFunc( "KeyUp",           &KeyUp           )
    .StaticFunc( "KeyDown",         &KeyDown         )
    .StaticFunc( "KeyPadDivide",    &KeyPadDivide    )
    .StaticFunc( "KeyPadMultiply",  &KeyPadMultiply  )
    .StaticFunc( "KeyPadSubtract",  &KeyPadSubtract  )
    .StaticFunc( "KeyPadAdd",       &KeyPadAdd       )
    .StaticFunc( "KeyPadDecimal",   &KeyPadDecimal   )
    .StaticFunc( "KeyPadEnter",     &KeyPadEnter     )
    .StaticFunc( "KeyPrintScreen",  &KeyPrintScreen  )
    .StaticFunc( "KeyPause",        &KeyPause        )
    .StaticFunc( "KeyPadEquals",    &KeyPadEquals    )
    .StaticFunc( "KeySemicolon",    &KeySemicolon    )
    .StaticFunc( "KeyApostrophe",   &KeyApostrophe   )
    .StaticFunc( "KeyNumLock",      &KeyNumLock      )
    .StaticFunc( "KeyBacktick",     &KeyBacktick     );

  Sqrat::RootTable( pcVM )
    .Bind( "rumKeypress", cKeypress );
}


void rumKeypress::SetValue( int32_t i_iKey, int32_t i_iAscii, int32_t i_iScancode, int32_t i_iMod )
{
  // The distance between uppercase and lowercase letters;
  static constexpr int32_t iUppderDistance{ 'a' - 'A' };

  m_iKey = i_iKey;
  m_iAscii = i_iAscii;
  m_iScancode = i_iScancode;

  bool bCapsLock{ ( i_iMod & GLFW_MOD_CAPS_LOCK ) != 0 };

  if( i_iMod & GLFW_MOD_SHIFT )
  {
    m_bShift = true;
  }
  else if( i_iMod & GLFW_MOD_CONTROL )
  {
    m_bCtrl = true;
  }
  else if( i_iMod & GLFW_MOD_ALT )
  {
    m_bAlt = true;
  }
  else if( i_iMod & GLFW_MOD_SUPER ) // Windows key
  {
    m_bOS = true;
  }

  if( !bCapsLock && m_iAscii >= GLFW_KEY_A && m_iAscii <= GLFW_KEY_Z )
  {
    if( !m_bShift )
    {
      m_iAscii += iUppderDistance;
    }
  }
  else if( m_bShift )
  {
    switch( m_iAscii )
    {
      case GLFW_KEY_GRAVE_ACCENT: m_iAscii = '~'; break;

      case GLFW_KEY_1: m_iAscii = '!'; break;
      case GLFW_KEY_2: m_iAscii = '@'; break;
      case GLFW_KEY_3: m_iAscii = '#'; break;
      case GLFW_KEY_4: m_iAscii = '$'; break;
      case GLFW_KEY_5: m_iAscii = '%'; break;
      case GLFW_KEY_6: m_iAscii = '^'; break;
      case GLFW_KEY_7: m_iAscii = '&'; break;
      case GLFW_KEY_8: m_iAscii = '*'; break;
      case GLFW_KEY_9: m_iAscii = '('; break;
      case GLFW_KEY_0: m_iAscii = ')'; break;

      case GLFW_KEY_MINUS: m_iAscii = '_'; break;
      case GLFW_KEY_EQUAL: m_iAscii = '+'; break;

      case GLFW_KEY_LEFT_BRACKET:  m_iAscii = '{'; break;
      case GLFW_KEY_RIGHT_BRACKET: m_iAscii = '}'; break;
      case GLFW_KEY_BACKSLASH:     m_iAscii = '|'; break;

      case GLFW_KEY_SEMICOLON:  m_iAscii = ':'; break;
      case GLFW_KEY_APOSTROPHE: m_iAscii = '\''; break;

      case GLFW_KEY_COMMA:  m_iAscii = '<'; break;
      case GLFW_KEY_PERIOD: m_iAscii = '>'; break;
      case GLFW_KEY_SLASH:  m_iAscii = '?'; break;
    }
  }
}

#endif // USE_GLFW
