#pragma once

#include <u_rum.h>


class rumKeypressBase
{
public:

  virtual ~rumKeypressBase() = default;

  int32_t GetAscii() const
  {
    return m_iAscii;
  }

  int32_t GetKey() const
  {
    return m_iKey;
  }

  int32_t GetScancode() const
  {
    return m_iScancode;
  }

  bool IsPrintable() const
  {
    return ( m_iAscii > 0x1f && m_iAscii < 0x7f );
  }

  bool AltPressed() const
  {
    return m_bAlt;
  }

  bool CtrlPressed() const
  {
    return m_bCtrl;
  }

  bool OSPressed() const
  {
    return m_bOS;
  }

  bool ShiftPressed() const
  {
    return m_bShift;
  }

  virtual std::string ToString();

  const char* ToString_VM()
  {
    return ToString().c_str();
  }

  static void ScriptBind();

protected:

  virtual void SetValue( int32_t i_iKey, int32_t i_iAscii = 0, int32_t i_iScancode = 0, int32_t i_iMod = 0 )
  {
    m_iKey = i_iKey;
    m_iAscii = i_iAscii;
    m_iScancode = i_iScancode;
  }

  int32_t m_iKey{ 0 };
  int32_t m_iAscii{ 0 };
  int32_t m_iScancode{ 0 };

  bool m_bAlt{ false };
  bool m_bCtrl{ false };
  bool m_bOS{ false }; // Windows key, Apple Key, etc.
  bool m_bShift{ false };
};


class rumInputBase
{
public:

  enum class rumMouseButton : uint32_t
  {
    Left,
    Right,
    Center
  };

  virtual rumPoint GetMousePosition() const = 0;

  virtual bool IsKeyPressed( int32_t i_iKey ) const = 0;

  void OnKeyPressed( const rumKeypressBase* i_pcKeypress );
  void OnKeyReleased( const rumKeypressBase* i_pcKeypress );
  void OnKeyRepeated( const rumKeypressBase* i_pcKeypress );

  void OnMouseButtonPressed( rumMouseButton i_eMouseButton, const rumPoint& i_rcPoint ) const;
  void OnMouseButtonReleased( rumMouseButton i_eMouseButton, const rumPoint& i_rcPoint ) const;
  void OnMouseMoved( const rumPoint& i_rcPoint ) const;
  void OnMouseScrolled( const rumPoint& i_rcDirection, const rumPoint& i_rcPoint ) const;

  static rumInputBase* GetInstance()
  {
    return s_pcInstance;
  }

  static int32_t GetLastKeyPressed();

  static rumPoint GetMousePosition_VM()
  {
    return GetInstance()->GetMousePosition();
  }

  static int32_t Init()
  {
    return RESULT_SUCCESS;
  }

  static bool IsKeyPressed_VM( int32_t i_iKey )
  {
    return GetInstance()->IsKeyPressed( i_iKey );
  }

  static void ScriptBind();

  static void Shutdown();

protected:

  // No one should be constructing this object
  rumInputBase() = default;

  // The singleton object
  static rumInputBase* s_pcInstance;

  // The last keypress
  static rumKeypressBase s_cLastKeypress;
};
