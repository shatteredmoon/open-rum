#include <controls/c_scrollbuffer.h>
#include <u_graphic.h>

rumClientScrollBuffer::rumClientScrollBuffer()
{
  m_pcDisplayBuffer = rumGraphic::Create();
}


rumClientScrollBuffer::~rumClientScrollBuffer()
{
  m_pcDisplayBuffer->Free();
}


void rumClientScrollBuffer::BufferEnd()
{
  if( m_uiBufferPixelHeightUsed >= GetHeight() )
  {
    const int32_t iMaxOffset{ static_cast<int32_t>( m_uiBufferPixelHeightUsed - GetHeight() ) };
    if( m_cBufferOffset.m_iY != iMaxOffset )
    {
      m_cBufferOffset.m_iY = iMaxOffset;
      UpdateDisplay();
    }
  }
}


void rumClientScrollBuffer::BufferHome()
{
  if( m_cBufferOffset.m_iY != 0 )
  {
    m_cBufferOffset.m_iY = 0;
    UpdateDisplay();
  }
}


void rumClientScrollBuffer::BufferDown( uint32_t i_uiPixelHeight )
{
  if( m_uiBufferPixelHeightUsed >= GetHeight() )
  {
    const int32_t iMaxOffset{ static_cast<int32_t>( GetBufferPixelHeight() - GetHeight() ) };
    if( m_cBufferOffset.m_iY != iMaxOffset )
    {
      m_cBufferOffset.m_iY += i_uiPixelHeight;
      if( m_cBufferOffset.m_iY > iMaxOffset )
      {
        m_cBufferOffset.m_iY = iMaxOffset;
      }

      UpdateDisplay();
    }
  }
}


void rumClientScrollBuffer::BufferUp( uint32_t i_uiPixelHeight )
{
  if( m_uiBufferPixelHeightUsed >= GetHeight() )
  {
    const int32_t iMaxOffset{ static_cast<int32_t>( GetBufferPixelHeight() - m_uiBufferPixelHeightUsed ) };
    if( m_cBufferOffset.m_iY != iMaxOffset )
    {
      m_cBufferOffset.m_iY -= i_uiPixelHeight;
      if( m_cBufferOffset.m_iY < iMaxOffset )
      {
        m_cBufferOffset.m_iY = iMaxOffset;
      }

      UpdateDisplay();
    }
  }
}



void rumClientScrollBuffer::DrawScrollbar()
{
  // Draw the scrollbar
  if( m_bScrollbarVisible && GetHeight() > 0 && GetWidth() > 0 && m_uiBufferPixelHeightUsed > GetHeight() )
  {
    const float fRatio{ ( m_cBufferOffset.m_iY / static_cast<float>( m_uiBufferPixelHeightUsed ) ) };
    const int32_t iOffset{ static_cast<int32_t>( fRatio * GetHeight() ) };
    const int32_t iSize
    {
      static_cast<int32_t>( GetHeight() / static_cast<float>( m_uiBufferPixelHeightUsed ) * GetHeight() )
    };

    // Display the scrollbar background
    m_pcDisplay->ClearRect( rumRectangle( rumPoint( m_uiScrollbarOffset, 0 ), m_uiScrollbarWidth, GetHeight() ),
                            m_cScrollbarBackground );

    // Display the scrollbar
    m_pcDisplay->ClearRect( rumRectangle( rumPoint( m_uiScrollbarOffset, iOffset ), m_uiScrollbarWidth, iSize ),
                            m_cScrollbarColor );
  }
}


rumPoint rumClientScrollBuffer::GetBufferVerticalExtents()
{
  return rumPoint( m_cBufferOffset.m_iY, m_cBufferOffset.m_iY + GetHeight() );
}


void rumClientScrollBuffer::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientScrollBuffer, rumClientControl> cClientScrollBuffer( pcVM, "rumClientScrollBuffer" );
  cClientScrollBuffer
    .Func( "End", &BufferEnd )
    .Func( "Home", &BufferHome )
    .Func( "PageDown", &BufferPageDown )
    .Func( "PageUp", &BufferPageUp )
    .Func( "ScrollDown", &BufferDown )
    .Func( "ScrollUp", &BufferUp )
    .Func( "SetBufferHeight", &ScriptSetBufferPixelHeight )
    .Func( "SetScrollbarColors", &SetScrollbarColors )
    .Func( "SetScrollbarWidth", &SetScrollbarWidth )
    .Func( "ShowScrollbar", &ShowScrollbar );
  Sqrat::RootTable( pcVM ).Bind( "rumScrollBuffer", cClientScrollBuffer );
}


void rumClientScrollBuffer::SetBufferPixelHeight( uint32_t i_uiHeight )
{
  // Make sure the buffer is always as large or larger than the displayed portion of the control
  if( i_uiHeight < GetHeight() )
  {
    i_uiHeight = GetHeight();
  }

  // Only invalidate if the size really changed
  if( m_uiBufferPixelHeight != i_uiHeight )
  {
    m_uiBufferPixelHeight = i_uiHeight;
    Invalidate();

    if( m_cBufferOffset.m_iY > static_cast<int32_t>(i_uiHeight) )
    {
      m_cBufferOffset.m_iY = static_cast<int32_t>(i_uiHeight);
    }
  }
}


bool rumClientScrollBuffer::SetScrollbarWidth( uint32_t i_uiWidth )
{
  bool bSuccess{ false };

  if( i_uiWidth < GetWidth() )
  {
    m_uiScrollbarWidth = i_uiWidth;
    Invalidate();
    bSuccess = true;
  }

  return bSuccess;
}


void rumClientScrollBuffer::ShowScrollbar( bool i_bShow )
{
  m_bScrollbarVisible = i_bShow;

  if( ( GetHeight() == 0 ) || ( GetWidth() == 0 ) )
  {
    return;
  }

  if( m_bScrollbarVisible && m_uiBufferPixelHeightUsed > GetHeight() )
  {
    DrawScrollbar();
  }
  else
  {
    // Clear the scrollbar by writing the background color over it
    m_pcDisplay->ClearRect( rumRectangle( rumPoint( m_uiScrollbarOffset, 0 ), m_uiScrollbarWidth, GetHeight() ),
                            m_cBackgroundColor );
  }
}


void rumClientScrollBuffer::UpdateDisplay()
{
  // Copy visible part of buffer to the viewport display
  m_pcDisplay->Blit( *m_pcDisplayBuffer, m_cBufferOffset, rumPoint(), GetWidth(), GetHeight() );
  DrawScrollbar();
}


bool rumClientScrollBuffer::Validate()
{
  bool bResult{ super::Validate() };

  m_uiBufferPixelHeightUsed = 0;
  m_uiScrollbarOffset = GetWidth() - m_uiScrollbarWidth;

  // Recreate the surfaces
  if( m_pcDisplayBuffer->InitData( GetWidth(), m_uiBufferPixelHeight ) )
  {
    m_pcDisplayBuffer->Clear( rumColor::s_cBlackTransparent );
    bResult &= true;
  }

  return bResult;
}
