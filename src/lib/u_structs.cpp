#include <u_structs.h>

#include <u_utility.h>

const rumColor rumColor::s_cBlack( 0, 0, 0 );
const rumColor rumColor::s_cWhite( 255, 255, 255 );
const rumColor rumColor::s_cRed( 255, 0, 0 );
const rumColor rumColor::s_cGreen( 0, 255, 0 );
const rumColor rumColor::s_cBlue( 0, 0, 255 );
const rumColor rumColor::s_cYellow( 255, 255, 0 );
const rumColor rumColor::s_cMagenta( 255, 0, 255 );
const rumColor rumColor::s_cCyan( 0, 255, 255 );
const rumColor rumColor::s_cGray( 127, 127, 127 );
const rumColor rumColor::s_cOrange( 255, 127, 0 );
const rumColor rumColor::s_cBlackTransparent( 0, 0, 0, RUM_ALPHA_TRANSPARENT );
const rumColor rumColor::s_cWhiteTransparent( 255, 255, 255, RUM_ALPHA_TRANSPARENT );
const rumColor rumColor::s_cBlackOpaque( 0, 0, 0, RUM_ALPHA_OPAQUE );
const rumColor rumColor::s_cWhiteOpaque( 255, 255, 255, RUM_ALPHA_OPAQUE );


namespace rumStructs
{
  rumPoint FindLineIntersection( const rumPoint& i_rcL1P1, const rumPoint& i_rcL1P2,
                                 const rumPoint& i_rcL2P1, const rumPoint& i_rcL2P2 )
  {
    const int32_t iA1{ i_rcL1P2.m_iY - i_rcL1P1.m_iY };
    const int32_t iB1{ i_rcL1P1.m_iX - i_rcL1P2.m_iX };
    const int32_t iC1{ iA1 * i_rcL1P1.m_iX + iB1 * i_rcL1P1.m_iY };

    const int32_t iA2{ i_rcL2P2.m_iY - i_rcL2P1.m_iY };
    const int32_t iB2{ i_rcL2P1.m_iX - i_rcL2P2.m_iX };
    const int32_t iC2{ iA2 * i_rcL2P1.m_iX + iB2 * i_rcL2P1.m_iY };

    const int32_t iDet{ iA1 * iB2 - iA2 * iB1 };
    if( iDet == 0 )
    {
      // Lines are parallel or coincident, no unique intersection
      return { 0, 0 };
    }

    return { ( iB2 * iC1 - iB1 * iC2 ) / iDet, ( iA1 * iC2 - iA2 * iC1 ) / iDet };
  }


  void ScriptBind()
  {
    rumPoint::ScriptBind();
    rumPosition::ScriptBind();
    rumVector::ScriptBind();
    rumRectangle::ScriptBind();
    rumColor::ScriptBind();

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::RootTable( pcVM )
      .Func( "rumFindLineIntersection", FindLineIntersection );
  }
}


rumPoint rumPoint::Add( const rumVector& i_rcVector ) const
{
  return rumPoint( m_iX + (int32_t)i_rcVector.m_fX, m_iY + (int32_t)i_rcVector.m_fY );
}


rumPoint rumPoint::operator+( const rumVector& i_rcVector ) const
{
  return Add( i_rcVector );
}


// virtual
void rumPoint::operator+=( const rumVector& i_rcVector )
{
  AddEqual( i_rcVector );
}


// virtual
void rumPoint::AddEqual( const rumVector& i_rcVector )
{
  m_iX += (int32_t)i_rcVector.m_fX;
  m_iY += (int32_t)i_rcVector.m_fY;
}


void rumPoint::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumPoint> cPoint( pcVM, "rumPoint" );
  cPoint
    .Ctor()
    .Ctor< const rumPoint& >()
    .Ctor< int32_t, int32_t >()
    .Var( "x", &rumPoint::m_iX )
    .Var( "y", &rumPoint::m_iY )
    .Func( "ToString", &ToString )
    .Func( "Equals", &Equals )
    .Func( "_subtract", &operator- )
    .SquirrelFunc( "_add", &ScriptOperatorAdd );
  Sqrat::RootTable( pcVM ).Bind( "rumPoint", cPoint );
}


std::string rumPoint::ToString() const
{
  std::string strPoint{ "Point (" };
  strPoint += rumStringUtils::ToString( m_iX );
  strPoint += ", ";
  strPoint += rumStringUtils::ToString( m_iY );
  strPoint += ')';

  return strPoint;
}


// static
SQInteger rumPoint::ScriptOperatorAdd( HSQUIRRELVM i_pcVM )
{
  if( sq_gettop( i_pcVM ) == 2 )
  {
    // Point + point
    Sqrat::Var<const rumPoint&> sqPoint1( i_pcVM, 1 );
    Sqrat::Var<const rumPoint&> sqPoint2( i_pcVM, 2 );
    if( !Sqrat::Error::Occurred( i_pcVM ) )
    {
      rumPoint cPoint{ sqPoint1.value + sqPoint2.value };
      Sqrat::PushVar( i_pcVM, cPoint );
      return 1;
    }
    Sqrat::Error::Clear( i_pcVM );

    // Point + vector
    Sqrat::Var<const rumVector&> sqVector( i_pcVM, 2 );
    if( !Sqrat::Error::Occurred( i_pcVM ) )
    {
      rumPoint cPoint{ sqPoint1.value + sqVector.value };
      Sqrat::PushVar( i_pcVM, cPoint );
      return 1;
    }

    Sqrat::Error::Clear( i_pcVM );
    return 0;
  }

  // Invalid number of arguments
  return 0;
}


rumPosition rumPosition::Add( const rumVector& i_rcVector ) const
{
  rumPosition cPosition( super::Add( i_rcVector ) );
  cPosition.m_eStatus = POSITION_UNCHECKED;
  return cPosition;
}


void rumPosition::AddEqual( const rumVector& i_rcVector )
{
  m_eStatus = POSITION_UNCHECKED;
  super::AddEqual( i_rcVector );
}


rumPosition rumPosition::operator+( const rumVector& i_rcVector ) const
{
  return Add( i_rcVector );
}


void rumPosition::operator+=( const rumVector& i_rcVector )
{
  AddEqual( i_rcVector );

}


void rumPosition::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumPosition, rumPoint> cPosition( pcVM, "rumPos" );
  cPosition
    .Ctor()
    .Ctor<int32_t, int32_t>()
    .Ctor<const rumPoint&>()
    .Func( "_subtract", &operator- )
    .SquirrelFunc( "_add", &ScriptOperatorAdd );
  Sqrat::RootTable( pcVM ).Bind( "rumPos", cPosition );
}


// static
SQInteger rumPosition::ScriptOperatorAdd( HSQUIRRELVM i_pcVM )
{
  if( sq_gettop( i_pcVM ) == 2 )
  {
    // Position + Position
    Sqrat::Var<const rumPosition&> sqPosition1( i_pcVM, 1 );
    Sqrat::Var<const rumPosition&> sqPosition2( i_pcVM, 2 );
    if( !Sqrat::Error::Occurred( i_pcVM ) )
    {
      rumPosition cPosition{ sqPosition1.value + sqPosition2.value };
      Sqrat::PushVar( i_pcVM, cPosition );
      return 1;
    }
    Sqrat::Error::Clear( i_pcVM );

    // Point + vector
    Sqrat::Var<const rumVector&> sqVector( i_pcVM, 2 );
    if( !Sqrat::Error::Occurred( i_pcVM ) )
    {
      rumPosition cPosition{ sqPosition1.value + sqVector.value };
      Sqrat::PushVar( i_pcVM, cPosition );
      return 1;
    }

    Sqrat::Error::Clear( i_pcVM );
    return 0;
  }

  // Invalid number of arguments
  return 0;
}


// override
std::string rumPosition::ToString() const
{
  std::string strPosition{ "Position (" };
  strPosition += rumStringUtils::ToString( m_iX );
  strPosition += ", ";
  strPosition += rumStringUtils::ToString( m_iY );
  strPosition += ')';

  return strPosition;
}


float rumVector::Magnitude() const
{
  return sqrt( m_fX * m_fX + m_fY * m_fY );
}


rumVector rumVector::Normalized() const
{
  const float fMagnitude{ Magnitude() };
  return rumVector( m_fX / fMagnitude, m_fY / fMagnitude );
}


void rumVector::Normalize()
{
  const float fMagnitude{ Magnitude() };
  m_fX = m_fX / fMagnitude;
  m_fY = m_fY / fMagnitude;
}


void rumVector::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumVector> cVector( pcVM, "rumVector" );
  cVector
    .Ctor()
    .Ctor<int32_t, int32_t>()
    .Ctor<float, float>()
    //.Ctor<const Point&>() // TODO - how to pull this off?
    .Ctor<const rumVector&>()
    .Var( "x", &rumVector::m_fX )
    .Var( "y", &rumVector::m_fY )
    .Func( "Magnitude", &Magnitude )
    .Func( "Normalize", &Normalize )
    .Func( "ToString", &ToString );
  Sqrat::RootTable( pcVM ).Bind( "rumVector", cVector );
}


std::string rumVector::ToString() const
{
  std::string strVector{ "Vector <" };
  strVector += rumStringUtils::ToFloatString( m_fX );
  strVector += ", ";
  strVector += rumStringUtils::ToFloatString( m_fY );
  strVector += '>';

  return strVector;
}


void rumRectangle::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumRectangle> cRectangle( pcVM, "Rect" );
  cRectangle
    .Ctor()
    .Ctor<const rumRectangle&>()
    .Ctor<const rumPoint&, const rumPoint&>()
    .Ctor<const rumPoint&, uint32_t, uint32_t>()
    .Ctor<int32_t, int32_t, uint32_t, uint32_t>()
    .Var( "p1", &rumRectangle::m_cPoint1 )
    .Var( "p2", &rumRectangle::m_cPoint2 )
    .Func( "ContainsPoint", &Contains )
    .Func( "ToString", &ToString );
  Sqrat::RootTable( pcVM ).Bind( "rumRect", cRectangle );
}


std::string rumRectangle::ToString() const
{
  std::string strOutput{ "Rectangle: " };
  strOutput += m_cPoint1.ToString();
  strOutput += " ";
  strOutput += m_cPoint2.ToString();

  return strOutput;
}


void rumColor::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumColor> cColor( pcVM, "rumColor" );
  cColor
    .Ctor()
    .Ctor<int32_t, int32_t, int32_t>()
    .Ctor<int32_t, int32_t, int32_t, int32_t>()
    .Ctor<const rumColor&>()
    .Func( "GetRedComponent", &GetRed )
    .Func( "GetBlueComponent", &GetBlue )
    .Func( "GetGreenComponent", &GetGreen )
    .Func( "GetAlphaComponent", &GetAlpha )
    .Func( "SetRedComponent", &SetRed )
    .Func( "SetBlueComponent", &SetBlue )
    .Func( "SetGreenComponent", &SetGreen )
    .Func( "SetAlphaComponent", &SetAlpha )
    .Func( "SetValue", &SetVal )
    .Func( "CopyValue", &CopyVal )
    .Func( "GetValue", &GetRGBA );
  Sqrat::RootTable( pcVM ).Bind( "rumColor", cColor );

  Sqrat::RootTable( pcVM )
    .SetInstance( "rumColorBlack", &s_cBlack )
    .SetInstance( "rumColorWhite", &s_cWhite )
    .SetInstance( "rumColorRed", &s_cRed )
    .SetInstance( "rumColorGreen", &s_cGreen )
    .SetInstance( "rumColorBlue", &s_cBlue )
    .SetInstance( "rumColorYellow", &s_cYellow )
    .SetInstance( "rumColorMagenta", &s_cMagenta )
    .SetInstance( "rumColorCyan", &s_cCyan )
    .SetInstance( "rumColorGray", &s_cGray )
    .SetInstance( "rumColorOrange", &s_cOrange )
    .SetInstance( "rumColorBlackTransparent", &s_cBlackTransparent )
    .SetInstance( "rumColorWhiteTransparent", &s_cWhiteTransparent )
    .SetInstance( "rumColorBlackOpaque", &s_cBlackOpaque )
    .SetInstance( "rumColorWhiteOpaque", &s_cWhiteOpaque );

  Sqrat::ConstTable( pcVM )
    .Const( "rumAlphaOpaque", RUM_ALPHA_OPAQUE )
    .Const( "rumAlphaTransparent", RUM_ALPHA_TRANSPARENT );
}
