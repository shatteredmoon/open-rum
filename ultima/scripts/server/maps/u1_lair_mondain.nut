class U1_Lair_Mondain_Map extends Map
{
  static s_fExplosionFrequency = 0.5;
  static s_fCleanupInterval = 1.0;

  m_bCreateExplosions = true;


  function CreateExplosion()
  {
    if( !m_bCreateExplosions )
    {
      return;
    }

    local ciPos = rumPos( rand()%GetNumColumns(), rand()%GetNumRows() );
    local ciPosData = GetPositionData( ciPos );
    local eTileID = ciPosData.GetTileID();
    if( U1_Floor_Lair_TileID == eTileID )
    {
      local ciExplosion = ::rumCreate( U1_Explosion_WidgetID );

      if( AddPawn( ciExplosion, ciPos ) )
      {
        // See if a player is on the explosion
        ciPosData = GetPositionData( ciPos );
        local ciPawn;
        while( ciPawn = ciPosData.GetNext( rumCreaturePawnType ) )
        {
          if( ciPawn instanceof Player )
          {
            ciPawn.Damage( rand()%100 + 100, null )
          }
        }

        ::rumSchedule( this, FreeExplosion, s_fCleanupInterval, ciExplosion );
      }
    }

    ::rumSchedule( this, CreateExplosion, s_fExplosionFrequency );
  }


  function FreeExplosion( ciExplosion )
  {
    RemovePawn( ciExplosion );
  }


  function OnLoaded()
  {
    if( !::rumInEditor() )
    {
      ::rumSchedule( this, CreateExplosion, s_fExplosionFrequency );
    }
  }


  function StopExplosions()
  {
    m_bCreateExplosions = false;
  }
}