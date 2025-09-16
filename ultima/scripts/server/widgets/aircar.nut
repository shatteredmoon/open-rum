class U1_Aircar_Widget extends Transport_Widget
{
  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    if( base.Exit( i_ciPlayer, i_ciPos, i_eMoveType ) )
    {
      // Stamp the exit time on the player
      i_ciPlayer.SetProperty( U1_Aircar_Deboard_Time_PropertyID, ::rumGetSecondsSinceEpoch() );

      return true;
    }

    return false;
  }
}
