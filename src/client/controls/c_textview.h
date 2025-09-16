#ifndef _C_TEXTVIEW_H_
#define _C_TEXTVIEW_H_

#include <u_structs.h>

#include <controls/c_control.h>
#include <controls/c_scrollbuffer.h>

#include <list>


class rumClientTextView : public rumClientScrollBuffer
{
public:

  virtual ~rumClientTextView();

  // Move to the very bottom of the scroll buffer
  virtual void BufferEnd();

  // Move to the very top of the scroll buffer
  virtual void BufferHome();

  // Move one viewport height towards the bottom of the scroll buffer
  virtual void BufferPageDown();

  // Move one viewport height towards the top of the scroll buffer
  virtual void BufferPageUp();

  uint32_t CalculateEntryHeight( const std::string& i_strText );

  void Clear()
  {
    m_listText.clear();
    super::Clear();
  }

  void PopText();
  uint32_t PushText( const std::string& i_strText );

  uint32_t GetMaxEntries() const
  {
    return m_uiMaxEntries;
  }
  void SetMaxEntries( uint32_t i_uiMax )
  {
    m_uiMaxEntries = i_uiMax;
  }

  static void ScriptBind();

  void SetNewestFirst( bool i_bNewestFirst );

private:
  virtual void DrawScrollbar();

  // Returns the pixel height of what was drawn
  uint32_t DrawEntry( const std::string& i_strText, bool i_bTestOnly = false );
  virtual bool Validate();

  typedef std::list<std::string> Container;
  static constexpr uint32_t DEFAULT_MAX_ENTRIES{ 32 };

  Container m_listText;

  uint32_t m_uiMaxEntries{ DEFAULT_MAX_ENTRIES };

  // When true, entires are drawn top-down with the newest entry listed
  // at the top edge of the buffer. This defaults to false so that the
  // newest entry is drawn at the bottom edge of the buffer, much like
  // typical chat programs and classic Ultima games
  bool m_bNewestFirst{ false };

  typedef rumClientScrollBuffer super;
};

#endif // _C_TEXTVIEW_H_
