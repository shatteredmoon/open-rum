class U4_Volcano_Portal extends Portal
{
  function Use( i_ciPlayer, i_eUsageType )
  {
    if( i_ciPlayer.GetTransportID() != rumInvalidGameID )
    {
      // Player cannot use this portal while on a transport - this should've been caught by the client
      i_ciPlayer.IncrementHackAttempts();
      return false;
    }

    // Player must be 8-parts Avatar to enter
    if( i_ciPlayer.IsEightPartsAvatar() &&
        ( i_ciPlayer.GetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.NotFound ) == U4_QuestItemState.Abyss_Used ) &&
        ( i_ciPlayer.GetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.NotFound ) == U4_QuestItemState.Abyss_Used ) &&
        ( i_ciPlayer.GetProperty( U4_Item_Candle_PropertyID, U4_QuestItemState.NotFound ) == U4_QuestItemState.Abyss_Used ) )
    {
      return base.Use( i_ciPlayer, i_eUsageType );
    }

    i_ciPlayer.ActionFailed( msg_strange_force_client_StringID );
    return false;
  }
}
