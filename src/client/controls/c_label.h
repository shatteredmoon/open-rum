#pragma once

#include <u_structs.h>
#include <controls/c_control.h>


class rumClientLabel : public rumClientControl
{
public:

  rumClientLabel() = default;
  rumClientLabel( const rumClientLabel& i_rcRHS ) = default;
  ~rumClientLabel() override;

  void Clear() override;

  std::string GetText() const
  {
    return m_strText;
  }

  bool IsEmpty() const
  {
    return m_strText.empty();
  }

  void SetText( const std::string& i_strText );

  static void ScriptBind();

private:

  void UpdateDisplay();
  bool Validate() override;

  std::string m_strText;

  typedef rumClientControl super;
};
