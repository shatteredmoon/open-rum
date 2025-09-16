// Received from client when player attempts to steal something
class Player_Steal_Broadcast extends rumBroadcast
{
  var1 = 0; // Direction or Class/Property
  var2 = 0; // Amount

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Direction or Class/Property
    }
    else if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Class or property
      var2 = vargv[1]; // Amount
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eDir = var1;

    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local bFound = false;

    if( !( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() ) )
    {
      local ciTransport = i_ciPlayer.GetTransport();
      if( null == ciTransport )
      {
        local ciVector = GetDirectionVector( eDir );

        // Save the first target position
        local ciPos = i_ciPlayer.GetPosition() + ciVector;

        local ciMap = i_ciPlayer.GetMap();
        local ciPosData = ciMap.GetPositionData( ciPos );

        // If the player is stealing a sign, advance to the next tile
        local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
        if( IsSignTile( ciTile ) )
        {
          bFound = TrySteal( i_ciPlayer, ciMap, ciPos + ciVector, eDir );
          if( !bFound )
          {
            // If the vector was diagonal, check other nearby spaces
            if( ciVector.x != 0 && ciVector.y != 0 )
            {
              // Try a vertical check
              local ciVectorTest = rumVector( 0, ciVector.y );
              bFound = TrySteal( i_ciPlayer, ciMap, ciPos + ciVectorTest, eDir );
              if( !bFound )
              {
                // Try a horizontal check
                ciVectorTest = rumVector( ciVector.x, 0 );
                bFound = TrySteal( i_ciPlayer, ciMap, ciPos + ciVectorTest, eDir );
              }
            }
          }
        }
        else
        {
          bFound = TrySteal( i_ciPlayer, ciMap, ciPos, eDir );
        }
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_only_on_foot_client_StringID );
        bFound = true;
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      bFound = true;
    }

    if( !bFound )
    {
      i_ciPlayer.ActionFailed( msg_no_effect_client_StringID );
    }

    i_ciPlayer.PopPacket();
  }


  function TrySteal( i_ciPlayer, i_ciMap, i_ciPos, i_eDir )
  {
    local bFound = false;
    local ciVector = GetDirectionVector( i_eDir );
    local ciPosData = i_ciMap.GetPositionData( i_ciPos );

    // Skip across widgets that allow it
    local ciWidget = ciPosData.GetNext( rumWidgetPawnType );
    if( ciWidget != null )
    {
      local bForward = ciWidget.GetProperty( Widget_Forwards_Interaction_PropertyID, false );
      if( bForward )
      {
        bFound = TrySteal( i_ciPlayer, i_ciMap, i_ciPos + ciVector, i_eDir );
        if( !bFound )
        {
          // If the vector was diagonal, check other nearby spaces
          if( ciVector.x != 0 && ciVector.y != 0 )
          {
            // Try a vertical check
            local ciVectorTest = rumVector( 0, ciVector.y );
            bFound = TrySteal( i_ciPlayer, i_ciMap, i_ciPos + ciVectorTest, i_eDir );
            if( !bFound )
            {
              // Try a horizontal check
              ciVectorTest = rumVector( ciVector.x, 0 );
              bFound = TrySteal( i_ciPlayer, i_ciMap, i_ciPos + ciVectorTest, i_eDir );
            }
          }
        }
      }
    }

    if( !bFound )
    {
      ciPosData.Reset();

      local ciCreature = null;
      while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
      {
        if( ciCreature.IsVisible() && !ciCreature.IsDead() )
        {
          bFound = true;

          if( NPCType.Merchant == ciCreature.m_eDefaultNpcType )
          {
            i_ciMap.Steal( ciCreature, i_ciPlayer );
          }
          else if( ciCreature.GetAssetID() == U1_Mondain_CreatureID )
          {
            ciCreature.OnSteal( i_ciPlayer );
          }
          else if( i_ciPlayer.AttemptSteal( ciCreature ) )
          {
            ciCreature.OnSteal( i_ciPlayer );
          }

          ciPosData.Stop();
        }
      }
    }

    return bFound;
  }
}
