class U2_Minax_Creature extends U2_NPC
{
  static s_iPosXArray = [60,  3, 57, 21, 39, 46, 31, 32];
  static s_iPosYArray = [3,  60, 61, 21, 37, 26,  9,  3];

  static s_fMaxAttackLeashRange = 999.0;
  static s_fMaxWanderLeashRange = 999.0;

  m_bRespawns = false;
  m_iPosIndex = 0;


  function AwardPlayer( i_ciPlayer )
  {
    if( null == i_ciPlayer )
    {
      return;
    }

    // Send endgame strings
    local ciMap = GetMap();
    local ciDestMap = GetOrCreateMap( i_ciPlayer, U2_Earth_AD_Castle_Lord_British_MapID );
    if( ciDestMap != null && ciMap.TransferPawn( i_ciPlayer, ciDestMap, rumPos( 31, 14 ) ) )
    {
      local strArray =
        [
          ::rumGetString( u2_endgame_1_server_StringID, i_ciPlayer.m_iLanguageID ),
          ::rumGetString( u2_endgame_2_server_StringID, i_ciPlayer.m_iLanguageID ),
          ::rumGetString( u2_endgame_3_server_StringID, i_ciPlayer.m_iLanguageID ),
          ::rumGetString( u2_endgame_4_server_StringID, i_ciPlayer.m_iLanguageID ),
          ::rumGetString( u2_endgame_5_server_StringID, i_ciPlayer.m_iLanguageID ),
          ::rumGetString( u2_endgame_6_server_StringID, i_ciPlayer.m_iLanguageID ),
          ::rumGetString( u2_endgame_7_server_StringID, i_ciPlayer.m_iLanguageID ),
          ::rumGetString( u2_endgame_8_server_StringID, i_ciPlayer.m_iLanguageID ),
          ::rumGetString( u2_endgame_9_server_StringID, i_ciPlayer.m_iLanguageID ),
          ::rumGetString( u2_endgame_10_server_StringID, i_ciPlayer.m_iLanguageID )
        ];

      local ciBroadcast = ::rumCreate( Player_Talk_U2_LordBritish_BroadcastID, U2_LBTalkType.Endgame, strArray );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }

    // Remember that the player has killed Minax
    local iFlags = i_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
    iFlags = ::rumBitSet( iFlags, UltimaCompletions.KilledMinax );
    i_ciPlayer.SetProperty( Ultima_Completions_PropertyID, iFlags );
  }


  function Damage( i_iAmount, i_ciSource, i_eWeaponType = rumInvalidAssetID, i_bSendClientEffect = true )
  {
    if( !( i_ciSource instanceof Player ) )
    {
      return;
    }

    // Minax doesn't really take damage
    i_iAmount = 1;
    base.Damage( i_iAmount, i_ciSource, i_eWeaponType, i_bSendClientEffect );

    // Where will Minax teleport to next?
    ++m_iPosIndex;
    if( m_iPosIndex < s_iPosXArray.len() )
    {
      i_ciSource.ActionSuccess( msg_minax_hit_client_StringID );
      i_ciSource.ActionSuccess( msg_minax_vanishes_client_StringID );

      // Minax teleports to her next location
      local ciMap = GetMap();
      local ciPos = rumPos( s_iPosXArray[m_iPosIndex], s_iPosYArray[m_iPosIndex] );
      ciMap.MovePawn( this, ciPos,
                      rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag |
                      rumIgnoreDistanceMoveFlag );

      // Heal
      AffectHitpoints( 9999 );
    }
    else
    {
      // Minax is defeated!
      AffectHitpoints(-9999);
      SendClientEffect( i_ciSource, ClientEffectType.ScreenShake );

      if( i_ciSource instanceof Player )
      {
        ::rumSchedule( this, AwardPlayer, 5.0, i_ciSource );

        // Give all players in the group credit
        local ciParty = i_ciSource.GetParty();
        if( ciParty != null )
        {
          foreach( uiMemberID in ciParty.m_uiRosterTable )
          {
            local ciMember = ::rumFetchPawn( uiMemberID );
            if( ciMember != i_ciSource )
            {
              ::rumSchedule( this, AwardPlayer, 5.0, ciMember );
            }
          }
        }
      }
    }
  }


  function NeedsLeash()
  {
    return false;
  }
}
