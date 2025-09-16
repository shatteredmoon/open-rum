#include <c_input.h>

#include <controls/c_control.h>
#include <u_utility.h>

rumInputBase* rumInputBase::s_pcInstance{ nullptr };
rumKeypressBase rumInputBase::s_cLastKeypress;


// static
void rumKeypressBase::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumKeypressBase, Sqrat::NoConstructor<rumKeypressBase>> cKeypressBase( pcVM, "rumKeypressBase" );
  cKeypressBase
    .Func( "GetKey", &GetKey )
    .Func( "GetAscii", &GetAscii )
    .Func( "GetScancode", &GetScancode )
    .Func( "AltPressed", &AltPressed )
    .Func( "CtrlPressed", &CtrlPressed )
    .Func( "OSKeyPressed", &OSPressed )
    .Func( "ShiftPressed", &ShiftPressed )
    .Func( "IsPrintable", &IsPrintable )
    .Func( "ToString", &ToString_VM );
  Sqrat::RootTable( pcVM ).Bind( "rumKeypressBase", cKeypressBase );
}


std::string rumKeypressBase::ToString()
{
  std::string str{ "m_iKey: " };
  str += rumStringUtils::ToString( m_iKey, 10 );
  str += " m_iAscii: ";
  str += rumStringUtils::ToString( m_iAscii, 10 );
  str += " m_iScancode: ";
  str += rumStringUtils::ToString( m_iScancode, 10 );
  return str;
}


int32_t rumInputBase::GetLastKeyPressed()
{
  return s_cLastKeypress.GetKey();
}


void rumInputBase::OnKeyPressed( const rumKeypressBase* i_pcKeypress )
{
  rumClientControl::OnInputSystemKeyPressed( i_pcKeypress );
}


void rumInputBase::OnKeyReleased( const rumKeypressBase* i_pcKeypress )
{
  rumClientControl::OnInputSystemKeyReleased( i_pcKeypress );
}


void rumInputBase::OnKeyRepeated( const rumKeypressBase* i_pcKeypress )
{
  rumClientControl::OnInputSystemKeyRepeated( i_pcKeypress );
}


void rumInputBase::OnMouseButtonPressed( rumMouseButton i_eMouseButton, const rumPoint& i_rcPoint ) const
{
  rumClientControl::OnInputSystemMouseButtonPressed( i_eMouseButton, i_rcPoint );
}



void rumInputBase::OnMouseButtonReleased( rumMouseButton i_eMouseButton, const rumPoint& i_rcPoint ) const
{
  rumClientControl::OnInputSystemMouseButtonReleased( i_eMouseButton, i_rcPoint );
}


void rumInputBase::OnMouseMoved( const rumPoint& i_rcPoint ) const
{
  rumClientControl::OnInputSystemMouseMoved( i_rcPoint );
}


void rumInputBase::OnMouseScrolled( const rumPoint& i_rcDirection, const rumPoint& i_rcPoint ) const
{
  rumClientControl::OnInputSystemMouseScrolled( i_rcDirection, i_rcPoint );
}


// static
void rumInputBase::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // Doesn't work in compiled scripts for some reason
  //Sqrat::ConstTable( pcVM )
  //  .Const( "rumLeftMouseButton", rumUtility::ToUnderlyingType( rumMouseButton::Left ) )
  //  .Const( "rumRightMouseButton", rumUtility::ToUnderlyingType( rumMouseButton::Right ) )
  //  .Const( "rumCenterMouseButton", rumUtility::ToUnderlyingType( rumMouseButton::Center ) );

  Sqrat::RootTable( pcVM )
    .Func( "rumGetLastKeyPressed", GetLastKeyPressed )
    .Func( "rumGetMousePosition", GetMousePosition_VM )
    .Func( "rumIsKeyPressed", IsKeyPressed_VM )
    .SetValue( "rumLeftMouseButton", rumUtility::ToUnderlyingType( rumMouseButton::Left ) )
    .SetValue( "rumRightMouseButton", rumUtility::ToUnderlyingType( rumMouseButton::Right ) )
    .SetValue( "rumCenterMouseButton", rumUtility::ToUnderlyingType( rumMouseButton::Center ) );

  Sqrat::Class<rumInputBase, Sqrat::NoConstructor<rumInputBase>> cInputBase( pcVM, "rumInputBase" );
  Sqrat::RootTable( pcVM ).Bind( "rumInputBase", cInputBase );
}


// static
void rumInputBase::Shutdown()
{
  SAFE_DELETE( s_pcInstance );
}
