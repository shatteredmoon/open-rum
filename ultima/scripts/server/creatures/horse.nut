class U2_Horse_Creature extends U2_NPC
{
  function OnStolen( i_ciPlayer )
  {
    local ciTransport = CreateTransport( U2_Horse_WidgetID, 0, GetMap(), GetPosition() );
    if( ciTransport != null && ciTransport.Board( i_ciPlayer ) )
    {
      // Adjust notoriety for stealing
      i_ciPlayer.AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 10 );

      SetVisibility( false );
      ScheduleRespawn();
    }
  }
}


class U3_Horse_Creature extends U3_NPC
{
  function OnStolen( i_ciPlayer )
  {
    local ciTransport = CreateTransport( U3_Horse_WidgetID, 0, GetMap(), GetPosition() );
    if( ciTransport != null && ciTransport.Board( i_ciPlayer ) )
    {
      // Adjust notoriety for stealing
      i_ciPlayer.AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 10 );

      SetVisibility( false );
      ScheduleRespawn();
    }
  }
}


class U4_Horse_Creature extends U4_NPC
{
  function OnStolen( i_ciPlayer )
  {
    local ciTransport = CreateTransport( U4_Horse_WidgetID, 0, GetMap(), GetPosition() );
    if( ciTransport != null && ciTransport.Board( i_ciPlayer ) )
    {
      // Adjust notoriety for stealing
      i_ciPlayer.AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 10 );

      // Deduct virtues and notoriety for stealing
      i_ciPlayer.AffectVirtue( VirtueType.Honesty, -1, true, true );
      i_ciPlayer.AffectVirtue( VirtueType.Justice, -1, true, false );
      i_ciPlayer.AffectVirtue( VirtueType.Honor, -1, true, false );
      i_ciPlayer.AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 10 );

      SetVisibility( false );
      ScheduleRespawn();
    }
  }
}
