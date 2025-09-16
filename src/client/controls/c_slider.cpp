#include <controls/c_slider.h>

#include <u_utility.h>


rumClientSlider::~rumClientSlider()
{
  Clear();
}


void rumClientSlider::Clear()
{
  super::Clear();
  OnValueChanged();
}


void rumClientSlider::DecrementValue()
{
  m_iValue -= m_uiStepAmount;
  rumNumberUtils::Clamp<int32_t>( m_iValue, m_iMinValue, m_iMaxValue );
  SetText( rumStringUtils::ToString( m_iValue ) );
  OnValueChanged();
}


void rumClientSlider::IncrementValue()
{
  m_iValue += m_uiStepAmount;
  rumNumberUtils::Clamp<int32_t>( m_iValue, m_iMinValue, m_iMaxValue );
  SetText( rumStringUtils::ToString( m_iValue ) );
  OnValueChanged();
}


void rumClientSlider::OnValueChanged()
{
  Sqrat::Object sqInstance{ GetScriptInstance() };
  if( sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( sqInstance, "OnValueChanged", m_iValue );
  }
}


// static
void rumClientSlider::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientSlider, rumClientLabel> cClientSlider( pcVM, "rumClientSlider" );
  cClientSlider
    .Func( "DecrementValue", &DecrementValue )
    .Func( "IncrementValue", &IncrementValue )
    .Func( "GetMaxValue", &GetMaxValue )
    .Func( "GetMinValue", &GetMinValue )
    .Func( "GetValue", &GetValue )
    .Func( "SetValue", &SetValue )
    .Func( "SetValueRange", &SetValueRange );
  Sqrat::RootTable( pcVM ).Bind( "rumSlider", cClientSlider );

  rumScript::CreateClassScript( "Slider", "rumSlider" );
}


void rumClientSlider::SetValue( int32_t i_iValue )
{
  rumNumberUtils::Clamp<int32_t>( i_iValue, m_iMinValue, m_iMaxValue );
  m_iValue = i_iValue;
  SetText( rumStringUtils::ToString( m_iValue ) );
  OnValueChanged();
}


void rumClientSlider::SetValueRange( int32_t i_iMinValue, int32_t i_iMaxValue )
{
  if( i_iMinValue > i_iMaxValue )
  {
    std::swap( i_iMinValue, i_iMaxValue );
  }

  m_iMinValue = i_iMinValue;
  m_iMaxValue = i_iMaxValue;
  rumNumberUtils::Clamp<int32_t>( m_iValue, m_iMinValue, m_iMaxValue );
  SetText( rumStringUtils::ToString( m_iValue ) );
}
