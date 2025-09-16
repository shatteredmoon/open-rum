class U1_Villager_Creature extends U1_NPC
{
  function AIWander()
  {
    // NOTE: This should only be called from AIDetermine()

    local eDelay = ActionDelay.Short;

    // 25% chance of moving in a random direction
    if( rand() % 256 < 64 )
    {
      local ciMap = GetMap();
      if( ciMap )
      {
        // Get a random position adjacent to the current position
        local ciPos = GetPosition() + GetRandomDirectionVector();
        if( ciMap.IsPositionWithinRadius( ciPos, m_ciOriginPos, s_fMaxWanderLeashRange ) )
        {
          // Get all pawns at the current position
          local ciPosData = ciMap.GetPositionData( ciPos );
          local ciHotspot = null;
          while( ciHotspot = ciPosData.GetNext( rumWidgetPawnType ) )
          {
            if( ciHotspot instanceof U1_Hotspot_Widget )
            {
              if( ciMap.MovePawn( this, ciPos ) == rumSuccessMoveResultType )
              {
                local ciPosData = ciMap.GetPositionData( ciPos );
                local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
                local fWeight = ciTile.GetWeight();
                if( fWeight >= 2.0 )
                {
                  eDelay = ActionDelay.Long;
                }
                else if( fWeight >= 1.0 )
                {
                  eDelay = ActionDelay.Medium;
                }
                else
                {
                  eDelay = ActionDelay.Short;
                }
                ciPosData.Stop();
              }
            }
          }
        }
      }
    }

    return eDelay;
  }
}
