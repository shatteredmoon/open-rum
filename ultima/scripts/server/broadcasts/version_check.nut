// Received from client when an Ultima installation could not be found
class Version_Check_Broadcast extends rumBroadcast
{
  function OnRecv( i_iSocket, i_ciPlayer )
  {
    // NOTE: This could be exploited by hackers as an easy return to LB from anywhere in the game, just by spoofing
    // this packet. For this reason, it only works if the player's Ultima version is from U1-U3.

    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    if( eVersion >= GameType.Ultima1 && eVersion < GameType.Ultima4 )
    {
      // Return the player to U4 Lord British
      i_ciPlayer.ChangeWorld( GameType.Ultima4 );
    }
    else
    {
      // Trying to warp to LB from somewhere already within U4
      i_ciPlayer.IncrementHackAttempts();
    }

    i_ciPlayer.PopPacket();
  }
}
