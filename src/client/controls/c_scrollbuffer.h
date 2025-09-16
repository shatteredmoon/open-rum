#ifndef _C_SCROLLBUFFER_H_
#define _C_SCROLLBUFFER_H_

#include <controls/c_control.h>
#include <u_structs.h>

// Controls inheriting from rumClientScrollBuffer have a surface that can be
// larger than the viewport size, and therefore can scroll around on the
// buffer surface.
class rumClientScrollBuffer : public rumClientControl
{
public:
  rumClientScrollBuffer();
  ~rumClientScrollBuffer() override;

  // Move to the very bottom of the scroll buffer
  virtual void BufferEnd();

  // Move to the very top of the scroll buffer
  virtual void BufferHome();

  // Move one viewport height towards the bottom of the scroll buffer
  virtual void BufferPageDown()
  {
    BufferDown( GetHeight() );
  }

  // Move one viewport height towards the top of the scroll buffer
  virtual void BufferPageUp()
  {
    BufferUp( GetHeight() );
  }

  // Move specified pixel amount toward the bottom of the scroll buffer
  virtual void BufferDown( uint32_t i_uiPixelHeight );

  // Move specified pixel amount toward the top of the scroll buffer
  virtual void BufferUp( uint32_t i_uiPixelHeight );

  uint32_t GetBufferPixelHeight() const
  {
    return m_uiBufferPixelHeight;
  }
  void SetBufferPixelHeight( uint32_t i_uiHeight );

  // Gets the top and bottom pixel position of the buffer
  rumPoint GetBufferVerticalExtents();

  virtual void ScriptSetBufferPixelHeight( uint32_t i_uiHeight )
  {
    SetBufferPixelHeight( i_uiHeight );
  }

  void SetScrollbarColors( const rumColor& i_cColor, const rumColor& i_cBgColor )
  {
    m_cScrollbarColor = i_cColor;
    m_cScrollbarBackground = i_cBgColor;
  }

  bool SetScrollbarWidth( uint32_t i_uiWidth ); // Invalidates
  void ShowScrollbar( bool i_bShow );

  static void ScriptBind();

protected:

  virtual void DrawScrollbar();

  void UpdateDisplay();
  bool Validate() override;

  static constexpr uint32_t DEFAULT_SCROLLBAR_WIDTH{ 3 };

  rumGraphic* m_pcDisplayBuffer{ nullptr };

  // Where the display is relative to the display buffer
  rumPoint m_cBufferOffset{ 0, 0 };

  uint32_t m_uiBufferPixelHeightUsed{ 0 };

  uint32_t m_uiScrollbarOffset{ 0 };
  uint32_t m_uiScrollbarWidth{ DEFAULT_SCROLLBAR_WIDTH };

  rumColor m_cScrollbarColor{ rumColor::s_cYellow };
  rumColor m_cScrollbarBackground{ rumColor::s_cGray };

  bool m_bScrollbarVisible{ true };

  typedef rumClientControl super;

private:

  uint32_t m_uiBufferPixelHeight;
};

#endif // _C_SCROLLBUFFER_H_
