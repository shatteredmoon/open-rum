class U3_Castle_Fire_Map extends Map
{
  static s_fExplosionFrequency = 0.5;
  static s_fCleanupInterval = 1.0;


  function CreateExplosion()
  {
    for( local i = 0; i < 5; ++i )
    {
      local ciPos = rumPos( rand()%GetNumColumns(), rand()%GetNumRows() );
      local ciPosData = GetPositionData( ciPos );
      local eTileID = ciPosData.GetTileID();
      if( U3_Floor_Brick_TileID == eTileID )
      {
        local ciExplosion = ::rumCreate( U1_Explosion_WidgetID );

        if( AddPawn( ciExplosion, ciPos ) )
        {
          local ciPawn;

          // See if a player is on the explosion
          local ciPosData = GetPositionData( ciPos );
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
}


class U3_Castle_Fire_2_Map extends Map
{
  m_bDestroyed = false;


  function Destroy()
  {
    if( !m_bDestroyed )
    {
      local eWidgetID = U3_Lava_WidgetID;
      if( rand()%100 < 20 )
      {
        eWidgetID = U4_Boulder_WidgetID;
      }

      // Add random rocks/lava
      local ciWidget = ::rumCreate( eWidgetID );
      if( ciWidget != null )
      {
        local iRandPosX = rand() % GetNumColumns();
        local iRandPosY = rand() % GetNumRows();
        local ciPos = rumPos( iRandPosX, iRandPosY );

        local eResult = MovePawn( ciWidget, ciPos, rumIgnoreDistanceMoveFlag | rumTestMoveFlag );
        if( rumSuccessMoveResultType == eResult )
        {
          AddPawn( ciWidget, ciPos );
        }
      }

      ::rumSchedule( this, Destroy, frand( 1.0 ) + 0.25 );
    }
  }


  function DestroyEnd()
  {
    // The castle is destroyed, so kill any remaining players
    local ciPlayersArray = GetAllPlayers();
    foreach( ciPlayer in ciPlayersArray )
    {
      ciPlayer.Kill( null, null, true );
    }

    m_bDestroyed = true;
  }


  function DestroyStart()
  {
    ::rumSchedule( this, DestroyEnd, 120.0 );
    Destroy();
    RandomizeMoongates();
  }


  function RandomizeMoongates()
  {
    local uiNumPlayers = GetNumPlayers();
    if( uiNumPlayers > 0 )
    {
      // Randomize moongate visibility
      local ciPawnsArray = GetAllPawns();
      foreach( ciPawn in ciPawnsArray )
      {
        if( ciPawn instanceof U3_Moongate_Alt_Widget )
        {
          ciPawn.SetVisibility( rand()%100 < 15 );
        }
      }

      ::rumSchedule( this, RandomizeMoongates, 5.0 );
    }
  }
}
