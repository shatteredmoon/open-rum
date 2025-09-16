class U2_Force_Field_Widget extends U2_Widget
{
  function OnCollisionTest( i_ciObject )
  {
    if( i_ciObject instanceof Player )
    {
      local bHasRing = i_ciObject.GetProperty( U2_Magic_Ring_PropertyID, false );
      if( bHasRing )
      {
        return false;
      }
    }

    return true;
  }
}


class U3_Force_Field_Widget extends U3_Widget
{
  function OnCollisionTest( i_ciObject )
  {
    if( i_ciObject instanceof Player )
    {
      local iFlags = i_ciObject.GetProperty( U3_Marks_PropertyID, 0 );
      if( ::rumBitOn( iFlags, U3_MarkType.Force ) )
      {
        return false;
      }
    }

    return true;
  }
}
