class U4_Codex_Widget extends U4_Widget
{
  function Use( i_ciPlayer )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    // Begin the Codex virtue tests
    i_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, 1 );

    local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.Virtue1 );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}
