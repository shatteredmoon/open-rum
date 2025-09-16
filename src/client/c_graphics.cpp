#include <c_graphics.h>

#include <u_db.h>
#include <u_log.h>
#include <u_resource.h>
#include <u_zlib.h>

#include <unordered_map>

#ifdef _DEBUG
#include <imgui.h>
#endif

rumGraphic* g_pcBackBuffer{ nullptr };
rumGraphic* g_pcPrimaryBuffer{ nullptr };

// Static initializations
int32_t rumClientGraphicBase::s_iWindowPosX{ 0 };
int32_t rumClientGraphicBase::s_iWindowPosY{ 0 };
uint32_t rumClientGraphicBase::s_uiFullScreenHeight{ DEFAULT_SCREEN_HEIGHT };
uint32_t rumClientGraphicBase::s_uiFullScreenWidth{ DEFAULT_SCREEN_WIDTH };
uint32_t rumClientGraphicBase::s_uiWindowedScreenHeight{ DEFAULT_SCREEN_HEIGHT };
uint32_t rumClientGraphicBase::s_uiWindowedScreenWidth{ DEFAULT_SCREEN_WIDTH };
bool rumClientGraphicBase::s_bFullscreen{ false };
bool rumClientGraphicBase::s_bInitialized{ false };


// static
void rumClientGraphicBase::DrawCircle_VM( const rumPoint& i_rcPos, int32_t i_iRadius, const rumColor& i_rcColor )
{
  if( g_pcBackBuffer )
  {
    g_pcBackBuffer->DrawCircle( i_rcPos, i_iRadius, i_rcColor );
  }
}


// static
void rumClientGraphicBase::DrawLine_VM( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumColor& i_rcColor )
{
  if( g_pcBackBuffer )
  {
    g_pcBackBuffer->DrawLine( i_rcPos1, i_rcPos2, i_rcColor );
  }
}


// static
void rumClientGraphicBase::DrawRect_VM( const rumRectangle& i_rcRect, const rumColor& i_rcColor )
{
  if( g_pcBackBuffer )
  {
    g_pcBackBuffer->DrawRect( i_rcRect, i_rcColor );
  }
}

// static
void rumClientGraphicBase::DrawRectUnfilled_VM( const rumRectangle& i_rcRect, const rumColor& i_rcColor )
{
  if( g_pcBackBuffer )
  {
    g_pcBackBuffer->DrawRectUnfilled( i_rcRect, i_rcColor );
  }
}

// static
void rumClientGraphicBase::DrawTriangle_VM( const rumPoint& i_rcPos1,
                                            const rumPoint& i_rcPos2,
                                            const rumPoint& i_rcPos3,
                                            const rumColor& i_rcColor )
{
  if( g_pcBackBuffer )
  {
    g_pcBackBuffer->DrawTriangle( i_rcPos1, i_rcPos2, i_rcPos3, i_rcColor );
  }
}


// static
rumGraphic& rumClientGraphicBase::GetBackBuffer()
{
  return *g_pcBackBuffer;
}


// static
rumGraphic& rumClientGraphicBase::GetPrimaryBuffer()
{
  return *g_pcPrimaryBuffer;
}


// static
int32_t rumClientGraphicBase::Init( const rumConfig& i_rcConfig )
{
  if( s_bInitialized )
  {
    return RESULT_SUCCESS;
  }

  super::Init( false /* don't force creation of all graphics */ );

  g_pcBackBuffer = Create();
  g_pcPrimaryBuffer = Create();

#ifdef _DEBUG
  IMGUI_CHECKVERSION();
  g_pcImGuiTLSContext = ImGui::CreateContext();

  ImGuiIO& rImGuiIO{ ImGui::GetIO() };
  rImGuiIO.DisplaySize.x = static_cast<float>( i_rcConfig.m_uiWindowedScreenWidth );
  rImGuiIO.DisplaySize.y = static_cast<float>( i_rcConfig.m_uiWindowedScreenHeight );
#endif // _DEBUG

  s_bInitialized = true;

  return RESULT_SUCCESS;
}


// static
// Refer to http://en.wikipedia.org/wiki/BMP_file_format for the technical
// specs of bitmap files
bool rumClientGraphicBase::IsBufferBMP( const char* i_pcBuffer, uint32_t i_uiSize )
{
  const int32_t BMP_HEADER_SIZE = 14;
  const int32_t BMP_MANUFACTURER_ID = ( 0x42 << 8 ) | 0x4D; // "BM"
  const int32_t BMP_RESERVED_0 = 0;

  return( i_pcBuffer &&
          i_uiSize > BMP_HEADER_SIZE &&
          ( ( ( i_pcBuffer[0] << 8 ) | i_pcBuffer[1] ) == BMP_MANUFACTURER_ID ) &&
            ( i_pcBuffer[6] == BMP_RESERVED_0 ) &&
            ( i_pcBuffer[7] == BMP_RESERVED_0 ) &&
            ( i_pcBuffer[8] == BMP_RESERVED_0 ) &&
            ( i_pcBuffer[9] == BMP_RESERVED_0 ) );
}


// static
bool rumClientGraphicBase::IsBufferJPG( const char* i_pcBuffer, uint32_t i_uiSize )
{
  const int32_t JPG_HEADER_SIZE = 10;
  return( i_pcBuffer &&
          i_uiSize > JPG_HEADER_SIZE &&
          ( i_pcBuffer[0] == 0xff ) && ( i_pcBuffer[1] == 0xd8 ) && ( i_pcBuffer[2] == 0xff ) ) &&
          ( ( ( i_pcBuffer[6] == 'J' ) && ( i_pcBuffer[7] == 'F' ) && ( i_pcBuffer[8] == 'I' ) && ( i_pcBuffer[9] == 'F' ) ) ||
            ( ( i_pcBuffer[6] == 'E' ) && ( i_pcBuffer[7] == 'x' ) && ( i_pcBuffer[8] == 'i' ) && ( i_pcBuffer[9] == 'f' ) ) );
}


// static
// Refer to http://www.qzx.com/pc-gpe/pcx.txt for the ZSoft PCX File Format
// Technical Reference Manual
bool rumClientGraphicBase::IsBufferPCX( const char* i_pcBuffer, uint32_t i_uiSize )
{
  const int32_t PCX_HEADER_SIZE = 128;
  const int32_t PCX_MANUFACTURER_ID = 10;
  const int32_t PCX_VERSION_2_5 = 0;
  const int32_t PCX_VERSION_3 = 5;
  const int32_t PCX_RLE_BYTE = 1;
  const int32_t PCX_RESERVED_0 = 0;

  return ( i_pcBuffer &&
           i_uiSize > PCX_HEADER_SIZE &&
           ( i_pcBuffer[0] == PCX_MANUFACTURER_ID ) &&
           i_pcBuffer[1] >= PCX_VERSION_2_5 &&
           i_pcBuffer[1] <= PCX_VERSION_3 &&
           ( i_pcBuffer[2] == PCX_RLE_BYTE ) &&
           ( i_pcBuffer[64] == PCX_RESERVED_0 ) &&
           ( i_pcBuffer[74] == PCX_RESERVED_0 ) );
}


// static
bool rumClientGraphicBase::IsBufferPNG( const char* i_pcBuffer, uint32_t i_uiSize )
{
  const int32_t PNG_HEADER_SIZE = 4;
  return ( i_pcBuffer &&
           i_uiSize > PNG_HEADER_SIZE &&
           ( i_pcBuffer[1] == 'P' ) && ( i_pcBuffer[2] == 'N' ) && ( i_pcBuffer[3] == 'G' ) );
}


// static
bool rumClientGraphicBase::IsBufferTGA( const char* i_pcBuffer, uint32_t i_uiSize )
{
  const int32_t TGA_HEADER_SIZE = 18;
  if( i_pcBuffer && i_uiSize > TGA_HEADER_SIZE )
  {
    // Interpret header (endian independent parsing)
    //int idlen          = (int32_t)i_pcBuffer[0];
    int32_t cmaptype     = (int32_t)i_pcBuffer[1];
    int32_t imagetype    = (int32_t)i_pcBuffer[2];
    //int cmapfirstidx   = (int32_t)i_pcBuffer[3]  | (((int32_t)i_pcBuffer[4]) << 8);
    //int cmaplen        = (int32_t)i_pcBuffer[5]  | (((int32_t)i_pcBuffer[6]) << 8);
    //int cmapentrysize  = (int32_t)i_pcBuffer[7];
    //int xorigin        = (int32_t)i_pcBuffer[8]  | (((int32_t)i_pcBuffer[9]) << 8);
    //int yorigin        = (int32_t)i_pcBuffer[10] | (((int32_t)i_pcBuffer[11]) << 8);
    //int width          = (int32_t)i_pcBuffer[12] | (((int32_t)i_pcBuffer[13]) << 8);
    //int height         = (int32_t)i_pcBuffer[14] | (((int32_t)i_pcBuffer[15]) << 8);
    int32_t bitsperpixel = (int32_t)i_pcBuffer[16];
    //int imageinfo      = (int32_t)i_pcBuffer[17];

    // Validate TGA header (is this a TGA file?)
    return ( ( ( cmaptype == 0 ) || ( cmaptype == 1 ) ) &&
             ( ( imagetype >= 1 && imagetype <= 3 ) || ( imagetype >= 9 && imagetype <= 11 ) ) &&
             ( ( bitsperpixel == 8 ) || ( bitsperpixel == 24 ) || ( bitsperpixel == 32 ) ) );
  }

  return false;
}


// static
bool rumClientGraphicBase::IsFullscreen()
{
  return s_bFullscreen;
}


// static
void rumClientGraphicBase::MessageBox( const std::string& i_strMsg )
{
#ifdef WIN32
  MessageBoxA( NULL, i_strMsg.c_str(), "RUM Client", MB_OK | MB_SYSTEMMODAL | MB_SETFOREGROUND );
#elif
#error // implement for your platform
#endif
}


// static
void rumClientGraphicBase::OnWindowSizeChanged()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  if( s_bFullscreen )
  {
    rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnWindowSizeChanged", s_uiFullScreenWidth,
                                 s_uiFullScreenHeight );
  }
  else
  {
    rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnWindowSizeChanged", s_uiWindowedScreenWidth,
                                 s_uiWindowedScreenHeight );
  }
}


// static
void rumClientGraphicBase::RenderGame()
{
#ifdef _DEBUG
  ImGui::NewFrame();

  // The ImGui test window
  //ImGui::ShowDemoWindow();

  ImGui::Render();

  ImGui::EndFrame();
#endif // _DEBUG
}


// static
void rumClientGraphicBase::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumClearScreen", ClearScreenColor )
    .Func( "rumDrawCircle", DrawCircle_VM )
    .Func( "rumDrawLine", DrawLine_VM )
    .Func( "rumDrawRect", DrawRect_VM )
    .Func( "rumDrawRectUnfilled", DrawRectUnfilled_VM )
    .Func( "rumDrawTriangle", DrawTriangle_VM )
    .Func( "rumGetFullScreenHeight", GetFullScreenHeight )
    .Func( "rumGetFullScreenWidth", GetFullScreenWidth )
    .Func( "rumGetScreenHeight", GetScreenBufferHeight )
    .Func( "rumGetScreenWidth", GetScreenBufferWidth )
    .Func( "rumGetWindowedScreenHeight", GetWindowedScreenHeight )
    .Func( "rumGetWindowedScreenWidth", GetWindowedScreenWidth )
    .Func( "rumIsFullscreen", IsFullscreen )
    .Overload<void( * )( void )>( "rumClearScreen", ClearScreen )
    .Overload<void( * )( const rumRectangle& )>( "rumClearScreen", ClearScreen )
    .Overload<void( * )( const rumRectangle&, const rumColor& )>( "rumClearScreen", ClearScreen );

  Sqrat::DerivedClass<rumClientGraphicBase, rumGraphic, Sqrat::NoConstructor<rumClientGraphicBase>>
    cClientGraphicBase( pcVM, "rumClientGraphicBase" );
  Sqrat::RootTable( pcVM ).Bind( "rumClientGraphicBase", cClientGraphicBase );
}


// static
void rumClientGraphicBase::Shutdown()
{
#ifdef _DEBUG
  if( g_pcImGuiTLSContext )
  {
    ImGui::DestroyContext( g_pcImGuiTLSContext );
    g_pcImGuiTLSContext = nullptr;
  }
#endif // _DEBUG

  if( g_pcBackBuffer )
  {
    g_pcBackBuffer->Free();
    g_pcBackBuffer = nullptr;
  }

  if( g_pcPrimaryBuffer )
  {
    g_pcPrimaryBuffer->Free();
    g_pcPrimaryBuffer = nullptr;
  }

  super::Shutdown();
}
