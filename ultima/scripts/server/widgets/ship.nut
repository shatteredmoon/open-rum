class U2_Ship_Widget extends Ship_Transport_Widget
{
  function Board( i_ciPlayer, i_bForce = false )
  {
    local bBoarded = false;

    // Players must own a blue tassle to board a ship
    local iTassles = i_ciPlayer.GetProperty( U2_Blue_Tassles_PropertyID, 0 );
    if( i_bForce || iTassles > 0 )
    {
      if( base.Board( i_ciPlayer, i_bForce ) )
      {
        if( !i_bForce )
        {
          i_ciPlayer.SetProperty( U2_Blue_Tassles_PropertyID, iTassles - 1 );
        }

        bBoarded = true;
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_blue_tassle_req_client_StringID );
    }

    return bBoarded;
  }
}
