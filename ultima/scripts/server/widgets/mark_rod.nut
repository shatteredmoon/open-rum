class U3_Mark_Rod_Widget extends U3_Widget
{
  // The amount of damage applied when a player obtains a mark
  static s_iDamage = 50;


  function Use( i_ciPlayer )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local iMarkFlags = i_ciPlayer.GetProperty( U3_Marks_PropertyID, 0 );
    local eMarkType = GetProperty( Widget_Mark_Type_PropertyID, 0 );
    iMarkFlags = ::rumBitSet( iMarkFlags, eMarkType );

    i_ciPlayer.Damage( s_iDamage, this );

    i_ciPlayer.SetProperty( U3_Marks_PropertyID, iMarkFlags );

    local strMark = format( "msg_mark_%d_client_StringID", eMarkType );
    local eStringID = getconsttable()[strMark];
    i_ciPlayer.ActionSuccess( eStringID );
  }
}
