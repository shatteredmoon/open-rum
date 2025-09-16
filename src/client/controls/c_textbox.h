#ifndef _C_TEXTBOX_H_
#define _C_TEXTBOX_H_

#include <u_structs.h>
#include <controls/c_control.h>

#include <regex>


class rumClientTextBox : public rumClientControl
{
public:
  rumClientTextBox();
  virtual ~rumClientTextBox();

  bool CharacterAdd( char i_strChar );
  bool CharacterBackspace();
  bool CharacterDelete();

  void Clear()
  {
    m_strText.clear();
    super::Clear();
    m_uiCursorIndex = 0;
  }

  void CursorLeft( bool i_bSelect );
  void CursorRight( bool i_bSelect );
  void CursorHome( bool i_bSelect );
  void CursorEnd( bool i_bSelect );

  std::string GetText() const
  {
    return m_strText;
  }
  std::string GetSelectedText() const;

  bool HasSelectedText() const
  {
    return m_bSelectionActive;
  }

  void ObscureText( bool bObscure );

  void CopyTextToClipboard() const;
  bool PasteTextFromClipboard();
  void SelectAll();

  bool ScriptCharacterAdd( const SQChar i_strChar )
  {
    return CharacterAdd( i_strChar );
  }

  void SetInputRegEx( const std::string& i_strRegEx );
  bool SetText( const std::string& i_strText );

  void SetMaxInputSize( uint32_t i_uiMaxInputSize );

  static void ScriptBind();

private:

  void DeleteSelectedText();
  void SetSelectionInactive();
  void UpdateDisplay();
  void UpdateSelection( uint32_t i_uiPrevIndex );
  bool Validate() override;

  // The text that is drawn to the control
  std::string m_strText;

  // Allowable characters, or all character if nothing is set
  std::regex m_reInput;

  uint32_t m_uiMaxInputSize{ s_uiDefaultMaxInputSize };

  // The index into m_strText where the cursor can be found
  uint32_t m_uiCursorIndex{ 0 };

  // The point where text will begin to be drawn on the control, based on cursor movement
  uint32_t m_uiCursorAnchorOffset{ 0 };

  // The index into m_strText where text selection begins, m_uiCursorIndex is used as the selection end point
  uint32_t m_uiSelectionStartIndex{ 0 };

  // Whether or not text selection is active
  bool m_bSelectionActive{ false };

  // When true, only asterisks will be drawn
  bool m_bObscure{ false };

  // When true, the regular rexpression set in m_reInput will be used to determine if a character can be added
  bool m_bUseRegEx{ false };

  // TODO: This should be adjustable by the end-user
  static constexpr uint32_t s_uiDefaultMaxInputSize{ 256 };

  typedef rumClientControl super;
};

#endif // _C_TEXTBOX_H_
