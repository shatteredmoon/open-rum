#pragma once

#include <controls/c_label.h>


// A slider control for setting an integer value within a min/max range
class rumClientSlider : public rumClientLabel
{
public:

  virtual ~rumClientSlider();

  void Clear() override;

  void DecrementValue();
  void IncrementValue();

  int32_t GetMaxValue() const
  {
    return m_iMaxValue;
  }

  int32_t GetMinValue() const
  {
    return m_iMinValue;
  }

  uint32_t GetStepAmount() const
  {
    return m_uiStepAmount;
  }

  void SetStepAmount( uint32_t i_uiStepAmount )
  {
    m_uiStepAmount = std::max<uint32_t>( 1, i_uiStepAmount );
  }

  int32_t GetValue() const
  {
    return m_iValue;
  }

  void SetValue( int32_t i_iValue );
  void SetValueRange( int32_t i_iMinValue, int32_t i_iMaxValue );

  static void ScriptBind();

private:

  void OnValueChanged();

  uint32_t m_uiStepAmount{ 1 };

  int32_t m_iMinValue{ 0 };
  int32_t m_iMaxValue{ 100 };

  int32_t m_iValue{ 100 };

  typedef rumClientLabel super;
};
