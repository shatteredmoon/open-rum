#ifndef _U_STRUCTS_H_
#define _U_STRUCTS_H_

#include <u_assert.h>
#include <u_script.h>

#undef max
#undef min

// Convenvience function for binding all structs
namespace rumStructs
{
  void ScriptBind();
}

struct rumVector;


struct rumPoint
{
  rumPoint() = default;
  virtual ~rumPoint() = default;

  rumPoint( const rumPoint& i_rcPoint ) : m_iX( i_rcPoint.m_iX ), m_iY( i_rcPoint.m_iY )
  {}

  rumPoint( int32_t i_iX, int32_t i_iY ) : m_iX( i_iX ), m_iY( i_iY )
  {}

  rumPoint operator=( const rumPoint& i_rcPoint )
  {
    m_iX = i_rcPoint.m_iX;
    m_iY = i_rcPoint.m_iY;
    return *this;
  }

  rumPoint operator+( const rumPoint& i_rcPoint ) const
  {
    return Add( i_rcPoint );
  }

  rumPoint operator+( const rumVector& i_rcVector ) const;

  virtual void operator+=( const rumPoint& i_rcPoint )
  {
    AddEqual( i_rcPoint );
  }

  virtual void operator+=( const rumVector& i_rcVector );

  rumPoint Add( int32_t i_iX, int32_t i_iY ) const
  {
    return Add( rumPoint( i_iX, i_iY ) );
  }

  rumPoint Add( const rumPoint& i_rcPoint ) const
  {
    return rumPoint( m_iX + i_rcPoint.m_iX, m_iY + i_rcPoint.m_iY );
  }

  rumPoint Add( const rumVector& i_rcVector ) const;

  virtual void AddEqual( int32_t i_iX, int32_t i_iY )
  {
    AddEqual( rumPoint( i_iX, i_iY ) );
  }

  virtual void AddEqual( const rumPoint& i_rcPoint )
  {
    m_iX += i_rcPoint.m_iX;
    m_iY += i_rcPoint.m_iY;
  }

  virtual void AddEqual( const rumVector& i_rcVector );

  rumPoint operator-( const rumPoint &i_rcPoint ) const
  {
    return Subtract( i_rcPoint );
  }

  virtual void operator-=( const rumPoint& i_rcPoint )
  {
    SubtractEqual( i_rcPoint );
  }

  rumPoint Subtract( int32_t i_iX, int32_t i_iY ) const
  {
    return Subtract( rumPoint( i_iX, i_iY ) );
  }

  rumPoint Subtract( const rumPoint& i_rcPoint ) const
  {
    return rumPoint( m_iX - i_rcPoint.m_iX, m_iY - i_rcPoint.m_iY );
  }

  virtual void SubtractEqual( int32_t i_iX, int32_t i_iY )
  {
    SubtractEqual( rumPoint( i_iX, i_iY ) );
  }

  virtual void SubtractEqual( const rumPoint& i_rcPoint )
  {
    m_iX -= i_rcPoint.m_iX;
    m_iY -= i_rcPoint.m_iY;
  }

  bool operator==( const rumPoint& i_rcPoint ) const
  {
    return ( ( m_iX == i_rcPoint.m_iX ) && ( m_iY == i_rcPoint.m_iY ) );
  }

  bool operator!=( const rumPoint& i_rcPoint ) const
  {
    return !( Equals( i_rcPoint ) );
  }

  bool Equals( const rumPoint& i_rcPoint ) const
  {
    return ( *this == i_rcPoint );
  }

  virtual std::string ToString() const;

  static void ScriptBind();

  int32_t m_iX{ 0 };
  int32_t m_iY{ 0 };

  static SQInteger ScriptOperatorAdd( HSQUIRRELVM i_pcVM );
};


struct rumPosition : public rumPoint
{
  rumPosition() = default;

  rumPosition( int32_t i_iX, int32_t i_iY )
    : rumPoint( i_iX, i_iY )
  {}

  rumPosition( const rumPoint& i_rcPoint )
    : rumPoint( i_rcPoint )
  {}

  rumPosition( const rumPosition& i_rcPosition )
    : rumPoint( i_rcPosition )
  {}

  rumPosition operator+( const rumPoint& i_rcPoint ) const
  {
    return Add( i_rcPoint.m_iX, i_rcPoint.m_iY );
  }

  rumPosition operator+( const rumVector& i_rcVector ) const;

  void operator+=( const rumPoint& i_rcPoint ) override
  {
    m_eStatus = POSITION_UNCHECKED;
    super::operator+=( i_rcPoint );
  }

  void operator+=( const rumVector& i_rcVector ) override;

  rumPosition Add( int32_t i_iX, int32_t i_iY ) const
  {
    return Add( rumPoint( i_iX, i_iY ) );
  }

  rumPosition Add( const rumPoint& i_rcPoint ) const
  {
    rumPosition cPosition( super::Add( i_rcPoint ) );
    cPosition.m_eStatus = POSITION_UNCHECKED;
    return cPosition;
  }

  rumPosition Add( const rumVector& i_rcVector ) const;

  void AddEqual( int32_t i_iX, int32_t i_iY ) override
  {
    m_eStatus = POSITION_UNCHECKED;
    super::AddEqual( rumPoint( i_iX, i_iY ) );
  }

  void AddEqual( const rumPoint& i_rcPoint ) override
  {
    m_eStatus = POSITION_UNCHECKED;
    super::AddEqual( i_rcPoint );
  }

  void AddEqual( const rumVector& i_rcVector ) override;

  rumPosition operator-( const rumPoint& i_rcPoint ) const
  {
    return Subtract( i_rcPoint );
  }

  void operator-=( const rumPoint& i_rcPoint )
  {
    m_eStatus = POSITION_UNCHECKED;
    super::operator-=( i_rcPoint );
  }

  rumPosition Subtract( int32_t i_iX, int32_t i_iY ) const
  {
    return Subtract( rumPoint( i_iX, i_iY ) );
  }

  rumPosition Subtract( const rumPoint& i_rcPoint ) const
  {
    rumPosition cPosition( super::Subtract( i_rcPoint ) );
    cPosition.m_eStatus = POSITION_UNCHECKED;
    return cPosition;
  }

  void SubtractEqual( int32_t i_iX, int32_t i_iY ) override
  {
    SubtractEqual( rumPoint( i_iX, i_iY ) );
  }

  void SubtractEqual( const rumPoint& i_rcPoint ) override
  {
    m_eStatus = POSITION_UNCHECKED;
    super::SubtractEqual( i_rcPoint );
  }

  std::string ToString() const override;

  static void ScriptBind();

  static SQInteger ScriptOperatorAdd( HSQUIRRELVM i_pcVM );

  enum PositionValidationEnum
  {
    POSITION_UNCHECKED = -1,
    POSITION_OK,
    POSITION_OUT_OF_BOUNDS
  };

  PositionValidationEnum m_eStatus{ POSITION_UNCHECKED };
  using super = rumPoint;
};


struct rumVector
{
  rumVector() = default;

  rumVector( int32_t i_iX, int32_t i_iY )
    : m_fX( (float)i_iX )
    , m_fY( (float)i_iY )
  {}

  rumVector( const rumPoint& i_rcPoint )
    : m_fX( (float)i_rcPoint.m_iX )
    , m_fY( (float)i_rcPoint.m_iY )
  {}

  rumVector( float i_fX, float i_fY )
    : m_fX( i_fX )
    , m_fY( i_fY )
  {}

  rumVector( const rumVector& i_rcVector )
    : m_fX( i_rcVector.m_fX )
    , m_fY( i_rcVector.m_fY )
  {}

  float Dot( const rumVector& i_rcVector ) const
  {
    return m_fX * i_rcVector.m_fX + m_fY * i_rcVector.m_fY;
  }

  float Magnitude() const;

  rumVector Normalized() const;
  void Normalize();

  std::string ToString() const;

  static void ScriptBind();

  float m_fX{ 0.f };
  float m_fY{ 0.f };
};


struct rumRectangle
{
  rumRectangle() = default;

  rumRectangle( const rumRectangle& i_rcRect )
    : m_cPoint1( i_rcRect.m_cPoint1 )
    , m_cPoint2( i_rcRect.m_cPoint2 )
  {}

  rumRectangle( const rumPoint& i_iPoint1, const rumPoint& i_iPoint2 )
    : m_cPoint1( i_iPoint1 )
    , m_cPoint2( i_iPoint2 )
  {}

  rumRectangle( const rumPoint& i_iPoint, uint32_t i_uiWidth, uint32_t i_uiHeight )
    : m_cPoint1( i_iPoint )
    , m_cPoint2( m_cPoint1.m_iX + i_uiWidth - 1, m_cPoint1.m_iY + i_uiHeight - 1 )
  {
    rumAssert( i_uiHeight > 0 );
    rumAssert( i_uiWidth > 0 );
  }

  rumRectangle( int32_t i_iLeft, int32_t i_iTop, uint32_t i_uiWidth, uint32_t i_uiHeight )
    : m_cPoint1( i_iLeft, i_iTop )
    , m_cPoint2( i_iLeft + i_uiWidth - 1, i_iTop + i_uiHeight - 1 )
  {
    rumAssert( i_uiHeight > 0 );
    rumAssert( i_uiWidth > 0 );
  }

  rumPoint Center() const
  {
    return rumPoint( ( m_cPoint2.m_iX - m_cPoint1.m_iX ) / 2 + m_cPoint1.m_iX,
                     ( m_cPoint2.m_iY - m_cPoint1.m_iY ) / 2 + m_cPoint1.m_iY );
  }

  bool Contains( const rumPoint& i_rcPoint ) const
  {
    return ( i_rcPoint.m_iX >= m_cPoint1.m_iX &&
             i_rcPoint.m_iX <= m_cPoint2.m_iX &&
             i_rcPoint.m_iY >= m_cPoint1.m_iY &&
             i_rcPoint.m_iY <= m_cPoint2.m_iY );
  }

  int32_t GetHeight() const
  {
    return abs( m_cPoint2.m_iY - m_cPoint1.m_iY ) + 1;
  }

  int32_t GetWidth() const
  {
    return abs( m_cPoint2.m_iX - m_cPoint1.m_iX ) + 1;
  }

  void IncludePoint( const rumPoint& i_rcPoint )
  {
    m_cPoint1.m_iX = std::min( m_cPoint1.m_iX, i_rcPoint.m_iX );
    m_cPoint1.m_iY = std::min( m_cPoint1.m_iY, i_rcPoint.m_iY );
    m_cPoint2.m_iX = std::max( m_cPoint2.m_iX, i_rcPoint.m_iX );
    m_cPoint2.m_iY = std::max( m_cPoint2.m_iY, i_rcPoint.m_iY );
  }

  std::string ToString() const;

  static void ScriptBind();

  rumPoint m_cPoint1;
  rumPoint m_cPoint2;
};


struct rumCircle
{
public:
  rumCircle() = default;

  rumCircle( const rumCircle& i_rcCircle )
    : m_cPoint( i_rcCircle.m_cPoint )
    , m_fRadius( i_rcCircle.m_fRadius )
  {}

  rumCircle( int32_t i_iX, int32_t i_iY, float i_fRadius = 1.f )
    : m_cPoint( i_iX, i_iY )
    , m_fRadius( i_fRadius )
  {}

  rumCircle( const rumPoint& i_rcPoint, float i_fRadius = 1.f )
    : m_cPoint( i_rcPoint )
    , m_fRadius( i_fRadius )
  {}

  bool Intersects( const rumRectangle& i_rcRect ) const
  {
    // Distance
    const rumPoint& i_rcCenterPoint{ i_rcRect.Center() };
    const rumPoint cPoint( abs( m_cPoint.m_iX - i_rcCenterPoint.m_iX ), abs( m_cPoint.m_iY - i_rcCenterPoint.m_iY ) );

    const float fHalfWidth{ i_rcRect.GetWidth() / 2.f };
    const float fHalfHeight{ i_rcRect.GetHeight() / 2.f };

    if( cPoint.m_iX > ( fHalfWidth + m_fRadius ) || cPoint.m_iY > ( fHalfHeight + m_fRadius ) )
    {
      return false;
    }

    if( cPoint.m_iX <= fHalfWidth || cPoint.m_iY <= fHalfHeight )
    {
      return true;
    }

    const float fX{ cPoint.m_iX - fHalfWidth };
    const float fY{ cPoint.m_iY - fHalfHeight };
    const float fCornerDistance_sq = ( fX * fX ) + ( fY * fY );
    return ( fCornerDistance_sq <= ( m_fRadius * m_fRadius ) );
  }

  rumPoint m_cPoint;
  float m_fRadius{ 1.f };
};


#define RUM_ALPHA_OPAQUE        255
#define RUM_ALPHA_TRANSPARENT   0

// 32-bit color
struct rumColor
{
  rumColor() = default;

  rumColor( rumByte i_iRed, rumByte i_iGreen, rumByte i_iBlue, rumByte i_iAlpha = RUM_ALPHA_OPAQUE )
    : m_iRed(   i_iRed )
    , m_iGreen( i_iGreen )
    , m_iBlue(  i_iBlue )
    , m_iAlpha( i_iAlpha )
  {}

  rumColor( rumDWord i_iValue )
    : m_iRed(   rumByte( ( i_iValue & 0xff000000 ) >> 24 ) )
    , m_iGreen( rumByte( ( i_iValue & 0x00ff0000 ) >> 16 ) )
    , m_iBlue(  rumByte( ( i_iValue & 0x0000ff00 ) >> 8 ) )
    , m_iAlpha( rumByte( i_iValue & 0xff ) )
  {}

  rumColor( const rumColor& i_rcColor )
    : m_iRed(   i_rcColor.m_iRed )
    , m_iGreen( i_rcColor.m_iGreen )
    , m_iBlue(  i_rcColor.m_iBlue )
    , m_iAlpha( i_rcColor.m_iAlpha )
  {}

  bool IsOpaque() const
  {
    return ( m_iAlpha == RUM_ALPHA_OPAQUE );
  }

  bool IsTransparent() const
  {
    return m_iAlpha != RUM_ALPHA_OPAQUE;
  }

  rumDWord GetRGBA() const
  {
    return ( m_iRed << 24 ) | ( m_iGreen << 16 ) | ( m_iBlue << 8 ) | m_iAlpha;
  }

  rumDWord GetABGR() const
  {
    return ( m_iAlpha << 24 ) | ( m_iBlue << 16 ) | ( m_iGreen << 8 ) | m_iRed;
  }

  uint32_t GetRed() const
  {
    return m_iRed;
  }

  uint32_t GetGreen() const
  {
    return m_iGreen;
  }

  uint32_t GetBlue() const
  {
    return m_iBlue;
  }

  uint32_t GetAlpha() const
  {
    return m_iAlpha;
  }

  void SetRed( uint32_t i_uiRed )
  {
    m_iRed = (rumByte)i_uiRed;
  }

  void SetGreen( uint32_t i_uiGreen )
  {
    m_iGreen = (rumByte)i_uiGreen;
  }

  void SetBlue( uint32_t i_uiBlue )
  {
    m_iBlue = (rumByte)i_uiBlue;
  }

  void SetAlpha( uint32_t i_uiAlpha )
  {
    m_iAlpha = (rumByte)i_uiAlpha;
  }

  void SetVal( rumDWord i_uiValue )
  {
    m_iRed   = rumByte( ( i_uiValue & 0xff000000 ) >> 24 );
    m_iGreen = rumByte( ( i_uiValue & 0x00ff0000 ) >> 16 );
    m_iBlue  = rumByte( ( i_uiValue & 0x0000ff00 ) >> 8 );
    m_iAlpha = rumByte( i_uiValue & 0xff );
  }

  void CopyVal( const rumColor& i_rcColor )
  {
    m_iRed   = i_rcColor.m_iRed;
    m_iGreen = i_rcColor.m_iGreen;
    m_iBlue  = i_rcColor.m_iBlue;
    m_iAlpha = i_rcColor.m_iAlpha;
  }

  bool operator==( const rumColor& i_rcColor ) const
  {
    return ( ( i_rcColor.m_iRed == m_iRed )     &&
             ( i_rcColor.m_iGreen == m_iGreen ) &&
             ( i_rcColor.m_iBlue == m_iBlue ) );
  }

  static void ScriptBind();

  // Opaque colors
  static const rumColor s_cBlack;
  static const rumColor s_cWhite;
  static const rumColor s_cRed;
  static const rumColor s_cGreen;
  static const rumColor s_cBlue;
  static const rumColor s_cYellow;
  static const rumColor s_cMagenta;
  static const rumColor s_cCyan;
  static const rumColor s_cGray;
  static const rumColor s_cOrange;

  // Transparent colors
  static const rumColor s_cBlackTransparent;
  static const rumColor s_cWhiteTransparent;

  // Opaque colors
  static const rumColor s_cBlackOpaque;
  static const rumColor s_cWhiteOpaque;

  rumByte m_iRed{ 0 };
  rumByte m_iGreen{ 0 };
  rumByte m_iBlue{ 0 };
  rumByte m_iAlpha{ RUM_ALPHA_OPAQUE };
};

#endif // _U_STRUCTS_H_
