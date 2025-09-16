class AttackEffect
{
  static s_fInterval = 0.001;

  m_eGraphicID = U4_Projectile_GraphicID;
  m_ciPos = null;
  m_ciTargetPos = null;
  m_uiAttackerID = rumInvalidGameID;
  m_uiTargetID = rumInvalidGameID;
  m_eAttackResult = null;

  // Difference from current position to target position
  m_iDeltaX = 0;
  m_iDeltaY = 0;

  // Amount to move when adjusting position
  m_iStepX = 0;
  m_iStepY = 0;

  m_iFraction = 0;


  function CalcStep()
  {
    m_iStepX = 1;
    m_iStepY = 1;

    // Convert to world coordinates
    m_ciPos.x *= g_ciUI.s_iTilePixelWidth;
    m_ciPos.y *= g_ciUI.s_iTilePixelWidth;
    m_ciTargetPos.x *= g_ciUI.s_iTilePixelWidth;
    m_ciTargetPos.y *= g_ciUI.s_iTilePixelWidth;

    m_iDeltaY = m_ciTargetPos.y - m_ciPos.y;
    m_iDeltaX = m_ciTargetPos.x - m_ciPos.x;

    if( m_iDeltaY < 0 )
    {
      m_iDeltaY = -m_iDeltaY;
      m_iStepY = -1;
    }

    if( m_iDeltaX < 0 )
    {
      m_iDeltaX = -m_iDeltaX;
      m_iStepX = -1;
    }

    m_iDeltaY *= 2;
    m_iDeltaX *= 2;

    if( m_iDeltaX > m_iDeltaY )
    {
      m_iFraction = m_iDeltaY - ( m_iDeltaX >> 1 ); // same as 2 * m_iDeltaY - m_iDeltaX
    }
    else
    {
      m_iFraction = m_iDeltaX - ( m_iDeltaY >> 1 );
    }
  }


  function Expire()
  {
    delete g_ciCUO.m_ciAttackEffectsTable[this];
  }


  function Step()
  {
    if( m_iDeltaX > m_iDeltaY )
    {
      for( local i = 0; i < 5; ++i )
      {
        if( m_ciPos.x != m_ciTargetPos.x )
        {
          if( m_iFraction >= 0 )
          {
            m_ciPos.y += m_iStepY;
            m_iFraction -= m_iDeltaX; // same as m_iFraction -= 2 * m_iDeltaX
          }

          m_ciPos.x += m_iStepX;
          m_iFraction += m_iDeltaY; // same as m_iFraction -= 2 * m_iDeltaY
        }
      }
    }
    else
    {
      for( local i = 0; i < 5; ++i )
      {
        if( m_ciPos.y != m_ciTargetPos.y )
        {
          if( m_iFraction >= 0 )
          {
            m_ciPos.x += m_iStepX;
            m_iFraction -= m_iDeltaY;
          }

          m_ciPos.y += m_iStepY;
          m_iFraction += m_iDeltaX;
        }
      }
    }

    if( m_ciPos.Equals( m_ciTargetPos ) )
    {
      // The attack has reached the target position - determine which effects to play
      local ciAttacker = ::rumFetchPawn( m_uiAttackerID );
      if( ciAttacker )
      {
        local ciTarget = ::rumFetchPawn( m_uiTargetID );
        if( ciTarget )
        {
          local ciPlayer = ::rumGetMainPlayer();
          if( AttackResultType.Hit == m_eAttackResult )
          {
            // TODO - Does the effect already handle 3D sound properly?
            DoDamageEffect( ciTarget, false );

            if( g_ciUI.m_bShowCombat && ( ciAttacker == ciPlayer ) && ( ciTarget instanceof NPC ) )
            {
              // The main player is the attacker
              local iHitpoints = ciTarget.GetHitpoints();
              local iMaxHitpoints = ciTarget.GetMaxHitpoints();
              local fHealthPercent = iHitpoints / iMaxHitpoints.tofloat();
              if( fHealthPercent > 0.0 )
              {
                local bNeverFlees = ciTarget.GetProperty( Creature_Never_Flees_PropertyID, false );
                local eStringID = rumInvalidAssetID;

                if( fHealthPercent > 0.8 )
                {
                  eStringID = msg_barely_wounded_client_StringID;
                }
                else if( fHealthPercent > 0.6 )
                {
                  eStringID = msg_lightly_wounded_client_StringID;
                }
                else if( fHealthPercent > 0.4 )
                {
                  eStringID = msg_heavily_wounded_client_StringID;
                }
                else if( fHealthPercent > 0.2 || ( ciTarget.GetMoveType() == MoveType.Stationary ) || bNeverFlees )
                {
                  eStringID = msg_critical_client_StringID;
                }
                else
                {
                  eStringID = msg_fleeing_client_StringID;
                }

                local strName = ciTarget.GetName() + "_client_StringID";
                local strDesc = format( "%s %s!", ::rumGetStringByName( strName ), ::rumGetString( eStringID ) );
                ShowString( strDesc, g_strColorTagArray.Yellow );
              }
            }

            if( ciTarget == ciPlayer )
            {
              // The main player was attacked
              PlaySound( Player_Hit_SoundID );
            }
            else if( ciTarget instanceof Player )
            {
              // A nearby player was hit
              PlaySound3D( Player_Hit_SoundID,
                           rumPos( m_ciTargetPos.x / g_ciUI.s_iTilePixelWidth,
                                   m_ciTargetPos.y / g_ciUI.s_iTilePixelWidth ) );
            }
            else
            {
              // A nearby creature was hit
              PlaySound3D( Creature_Hit_SoundID,
                           rumPos( m_ciTargetPos.x / g_ciUI.s_iTilePixelWidth,
                                   m_ciTargetPos.y / g_ciUI.s_iTilePixelWidth ) );
            }
          }
          else if( AttackResultType.Miss == m_eAttackResult )
          {
            if( ciAttacker == ciPlayer )
            {
              // The main player is the attacker
              if( g_ciUI.m_bShowCombat )
              {
                ShowString( ::rumGetString( msg_missed_client_StringID ), g_strColorTagArray.Red );
              }
            }

            if( ciTarget == ciPlayer )
            {
              // The main player was attacked
              PlaySound( Player_Miss_SoundID );
            }
            else if( ciTarget instanceof Player )
            {
              // A nearby player was missed
              PlaySound3D( Player_Miss_SoundID,
                           rumPos( m_ciTargetPos.x / g_ciUI.s_iTilePixelWidth,
                                   m_ciTargetPos.y / g_ciUI.s_iTilePixelWidth ) );
            }
            else
            {
              // A nearby creature was missed
              PlaySound3D( Creature_Miss_SoundID,
                           rumPos( m_ciTargetPos.x / g_ciUI.s_iTilePixelWidth,
                                   m_ciTargetPos.y / g_ciUI.s_iTilePixelWidth ) );
            }
          }
          else if( AttackResultType.Unaffected == m_eAttackResult )
          {
            if( ciAttacker == ciPlayer )
            {
              // The main player is the attacker
              if( g_ciUI.m_bShowCombat )
              {
                ShowString( ::rumGetString( msg_no_effect_client_StringID ), g_strColorTagArray.Yellow );
              }
            }

            if( ciTarget == ciPlayer )
            {
              // The main player was attacked
              PlaySound( Player_Miss_SoundID );
            }
            else if( ciTarget instanceof Player )
            {
              // A nearby player was missed
              PlaySound3D( Player_Miss_SoundID,
                           rumPos( m_ciTargetPos.x / g_ciUI.s_iTilePixelWidth,
                                   m_ciTargetPos.y / g_ciUI.s_iTilePixelWidth ) );
            }
            else
            {
              // A nearby creature was missed
              PlaySound3D( Creature_Miss_SoundID,
                           rumPos( m_ciTargetPos.x / g_ciUI.s_iTilePixelWidth,
                                   m_ciTargetPos.y / g_ciUI.s_iTilePixelWidth ) );
            }
          }
        }
      }

      ::rumSchedule( this, Expire, s_fInterval );
    }
    else
    {
      ::rumSchedule( this, Step, s_fInterval );
    }
  }
}


// Received from server when a player or NPC attacks something
class Attack_Update_Broadcast extends rumBroadcast
{
  var1 = 0; // Attacker unique id
  var2 = 0; // Target unique id
  var3 = 0; // Weapon class
  var4 = 0; // Attack result


  function OnRecv()
  {
    local uiAttackerID = var1;
    local uiTargetID = var2;
    local eWeaponType = var3;
    local eResult = var4;

    local ciAttacker = ::rumFetchPawn( uiAttackerID );
    if( ciAttacker )
    {
      local ciTarget = ::rumFetchPawn( uiTargetID );
      if( ciTarget )
      {
        local ciWeapon = eWeaponType ? ::rumGetInventoryAsset( eWeaponType ) : null;

        local ciAttack = AttackEffect();
        ciAttack.m_ciPos = ciAttacker.GetPosition();
        ciAttack.m_ciTargetPos = ciTarget.GetPosition();
        ciAttack.m_uiAttackerID = uiAttackerID;
        ciAttack.m_uiTargetID = uiTargetID;
        ciAttack.m_eAttackResult = eResult;

        if( ciWeapon != null )
        {
          ciAttack.m_eGraphicID = ciWeapon.GetProperty( Inventory_Weapon_Attack_Graphic_ID_PropertyID,
                                                        U4_Projectile_GraphicID );
        }

        ciAttack.CalcStep();

        g_ciCUO.m_ciAttackEffectsTable[ciAttack] <- ciAttack;

        ::rumSchedule( ciAttack, ciAttack.Step, ciAttack.s_fInterval );
      }
    }
  }
}
