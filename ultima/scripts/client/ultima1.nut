/*

----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88                                ooooo
888    88   888  o888oo  oooo  oo ooo oooo     ooooooo       888
888    88   888   888     888   888 888 888    ooooo888      888
888    88   888   888     888   888 888 888  888    888      888
888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o    ooooo

----------------------------------------------------------------------------

Classic Ultima Online (CUO)

MIT License

Copyright 2015 Jonathon Blake Wood-Brooks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Ultima is a registered trademark of Electronic Arts (EA)

Ultima I: The First Age of Darkness
Developers: Richard Garriott, Origin Systems
Publishers: California Pacific Computer Co., Origin Systems, Electronic Arts
Designers:  Richard Garriott, Ken W. Arnold

*/

g_tblU1QuestArray <-
[
  // LostKing
  { eTokenID1 = U1_Gelatinous_Cube_Creature_client_StringID,
    eTokenID2 = U1_Daemon_Creature_client_StringID },

  // Rondorin
  { eTokenID1 = U1_Carrion_Creeper_Creature_client_StringID,
    eTokenID2 = U1_Invisible_Seeker_Creature_client_StringID },

  // BlackDragon
  { eTokenID1 = U1_Lich_Creature_client_StringID,
    eTokenID2 = U1_Zorn_Creature_client_StringID },

  // Shamino
  { eTokenID1 = U1_Balron_Creature_client_StringID,
    eTokenID2 = U1_Mind_Whipper_Creature_client_StringID },

  // LordBritish
  { eTokenID1 = u1_sign_1_client_StringID,   // tower of knowledge
    eTokenID2 = u1_sign_5_client_StringID }, // eastern signpost

  // Barataria
  { eTokenID1 = u1_sign_3_client_StringID,   // pillar of ozymandias
    eTokenID2 = u1_sign_6_client_StringID }, // the signpost

  // WhiteDragon
  { eTokenID1 = u1_sign_4_client_StringID,   // grave of the lost soul
    eTokenID2 = u1_sign_2_client_StringID }, // pillars of the argonauts

  // Olympus
  { eTokenID1 = u1_sign_7_client_StringID,   // southern signpost
    eTokenID2 = u1_sign_0_client_StringID }  // pillars of protection
]


function U1_Animate()
{
  local ciVector = GetDirectionVector( g_ciCUO.m_eWindDirection );

  // Shift water in the direction of the wind
  ::rumGetGraphic( U1_Water_GraphicID ).Shift( ciVector );

  // Shift other scrolling tiles down
  ::rumGetGraphic( U4_Field_Force_GraphicID ).Shift( rumVector( 0, 1 ) );
}


function U1_CastSpell()
{
  local strDesc = ::rumGetString( command_cast_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();

  local ePlayerClassID = ciPlayer.GetProperty( U1_PlayerClass_PropertyID, U1_Fighter_Class_CustomID );
  local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );

  local bCastsSpells = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
  if( !bCastsSpells )
  {
    local strClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );
    strDesc = format( ::rumGetString( msg_cast_restricted_client_StringID ), strClass );
    ShowString( strDesc, g_strColorTagArray.Red );
    return;
  }

  local eTransportType = TransportType.None;
  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null )
  {
    eTransportType = ciTransport.GetType();
  }

  if( TransportType.SpaceShip == eTransportType )
  {
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
    return;
  }

  if( ciPlayer.IsNegated() || ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.025|0.1" );

  // Build the list of spells
  foreach( iIndex, eSpellID in g_eU1SpellArray )
  {
    local ciSpell = ::rumGetCustomAsset( eSpellID );
    local iClassFlags = ciSpell.GetProperty( Spell_Class_Compatibility_Flags_PropertyID, 0 );
    if( iClassFlags & ( 1 << ciPlayerClass.GetProperty( Class_ID_PropertyID, 0 ) ) )
    {
      local strSpell = ::rumGetStringByName( ciSpell.GetName() + "_client_StringID" );
      local iSpellIndex = ciSpell.GetProperty( Spell_Key_ID_PropertyID, 0 );
      local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iSpellIndex ) );
      local strEntry = format( "%s|%s", strAlpha, strSpell );
      g_ciUI.m_ciGameListView.SetEntry( iIndex, strEntry, rumKeypress.KeyA() + iSpellIndex );
    }
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U1_CastSpellSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = U1_CastSpellFinished;
  g_ciUI.m_ciGameListView.m_funcIndexChanged = Ultima_CastSpellSelectionChanged;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  // Switch to the spells stat page
  g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;
  U1_Stat_Update( U1_StatPage.Spells );
}


function U1_CastSpellFinished( i_eDelay = ActionDelay.Short )
{
  // Restore the player's previous stat page
  U1_Stat_Update( g_ciUI.m_ePreviousStatPage );
  Ultima_ListSelectionEnd( i_eDelay );
}


function U1_CastSpellSelected()
{
  local eKey = g_ciUI.m_ciGameListView.GetSelectedKey();
  local eSpellID = g_eU1SpellArray[eKey];
  local ciPlayer = ::rumGetMainPlayer();
  local bDone = ciPlayer.CastSpell( eSpellID );

  if( bDone )
  {
    U1_CastSpellFinished();
  }
  else
  {
    g_ciUI.m_ciGameListView.SetActive( false );
    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.DisableMultiSelect();
    g_ciUI.m_ciGameListView.m_funcAccept = null;
    g_ciUI.m_ciGameListView.m_funcCancel = null;
    g_ciUI.m_ciGameListView.m_funcIndexChanged = OnGameListViewIndexChanged;

    g_ciUI.m_ciGameInputTextBox.ShowPrompt( true );
    g_ciUI.m_ciGameInputTextBox.Focus();
  }
}


function U1_Hyperjump()
{
  Ultima_Hyperjump();
}


function U1_Init()
{}


function U1_KeyPressedGame( i_ciKeyboard )
{
  local eKey = i_ciKeyboard.GetKey();

  if( rumKeypress.KeyA() == eKey )
  {
    local bForceRanged = i_ciKeyboard.ShiftPressed();
    Ultima_Attack( bForceRanged );
  }
  else if( rumKeypress.KeyB() == eKey )
  {
    Ultima_Board();
  }
  else if( rumKeypress.KeyC() == eKey )
  {
    U1_CastSpell();
  }
  else if( rumKeypress.KeyD() == eKey )
  {
    // In Ultima 1, the D command is Drop
    Ultima_Descend();
  }
  else if( rumKeypress.KeyE() == eKey )
  {
    UsePortal( PortalUsageType.Enter );
  }
  else if( rumKeypress.KeyF() == eKey )
  {
    Ultima_FireTransportWeapon();
  }
  else if( rumKeypress.KeyG() == eKey )
  {
    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local bFound = false;

    local ciWidget;
    local ciPosData = ciMap.GetPositionData( ciPlayer.GetPosition() );
    while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
    {
      if( ciWidget.IsVisible() )
      {
        local eAssetID = ciWidget.GetAssetID();
        if( ( U1_Chest_WidgetID == eAssetID ) || ( U1_Gem_Immortality_WidgetID == eAssetID ) )
        {
          Ultima_Get( eAssetID );
          ciPosData.Stop();
          bFound = true;
        }
      }
    }

    if( !bFound )
    {
      ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
    }
  }
  else if( rumKeypress.KeyH() == eKey )
  {
    // Same as trade
    U1_HandEquipment();
  }
  else if( rumKeypress.KeyI() == eKey )
  {
    if( i_ciKeyboard.AltPressed() )
    {
      Ultima_ExtinguishLight();
    }
    else
    {
      Ultima_IgniteTorch( U1_Torches_PropertyID );
    }
  }
  else if( rumKeypress.KeyJ() == eKey )
  {
    // In Ultima 1, there is no J command originally
    Ultima_JimmyLock( U1_Door_Widget, U1_Keys_PropertyID );
  }
  else if( rumKeypress.KeyK() == eKey )
  {
    Ultima_Klimb();
  }
  else if( rumKeypress.KeyL() == eKey )
  {
    if( i_ciKeyboard.AltPressed() )
    {
      Ultima_AdvanceMapLabel();
    }
    else
    {
      // In Ultima 1, there is no L command originally
      Ultima_Look();
    }
  }
  else if( rumKeypress.KeyM() == eKey )
  {
    // In Ultima 1, M was Modify Order

    if( i_ciKeyboard.AltPressed() )
    {
      U1_ShowAssociatedMap();
    }
  }
  else if( rumKeypress.KeyN() == eKey )
  {
    // In the original game, N was Noise toggle
    local bOnSelf = true;
    if( i_ciKeyboard.AltPressed() )
    {
      bOnSelf = false;
    }

    Ultima_NipPotion( U1_Potions_PropertyID, bOnSelf );
  }
  else if( rumKeypress.KeyO() == eKey )
  {
    Ultima_Open( U1_Door_Widget );
  }
  else if( rumKeypress.KeyQ() == eKey )
  {
    Ultima_Quit();
  }
  else if( rumKeypress.KeyR() == eKey )
  {
    Ultima_ReadyWeapon();
  }
  else if( rumKeypress.KeyS() == eKey )
  {
    Ultima_Steal();
  }
  else if( rumKeypress.KeyT() == eKey )
  {
    Ultima_Talk( command_transact_client_StringID );
  }
  else if( rumKeypress.KeyU() == eKey )
  {
    // In the original game, this was Unlock
    U1_UseItem();
  }
  else if( rumKeypress.KeyV() == eKey )
  {
    // In the original game, V was used to View
    Ultima_Volume();
  }
  else if( rumKeypress.KeyW() == eKey )
  {
    // In the original game, R was used to Ready armour
    Ultima_WearArmour();
  }
  else if( rumKeypress.KeyX() == eKey )
  {
    Ultima_Xit();
  }
  else if( rumKeypress.KeyY() == eKey )
  {
    // In the original game, Y was unused
    Ultima_Yell();
  }
  else if( rumKeypress.KeyZ() == eKey )
  {
    if( i_ciKeyboard.AltPressed() || i_ciKeyboard.CtrlPressed() )
    {
      // Alt-Z always shows the main stat panel
      U1_Stat_Update( U1_StatPage.Main );
    }
    else
    {
      U1_Stat();
    }
  }
  else if( rumKeypress.KeyTab() == eKey )
  {
    Ultima_AdvanceMapLabel();
  }
  else if( ( rumKeypress.KeyPadSubtract() == eKey ) || ( rumKeypress.KeyDash() == eKey ) )
  {
    Ultima_Resurrect( ResurrectionType.Void, GameType.Ultima1 );
  }
  else if( ( rumKeypress.KeyPadAdd() == eKey ) || ( rumKeypress.KeyEquals() == eKey ) )
  {
    Ultima_Resurrect( ResurrectionType.Body, GameType.Ultima1 );
  }
}


function U1_PlayerTransaction( i_ciTarget )
{
  if( i_ciTarget && ( i_ciTarget instanceof Player ) )
  {
    local ciPlayer = ::rumGetMainPlayer();
    local bInParty = ciPlayer.PlayerInParty( i_ciTarget ) || ( ciPlayer == i_ciTarget );

    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.DisableMultiSelect();
    g_ciUI.m_ciGameListView.SetFormat( "0.05" );
    g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.Tell, ::rumGetString( talk_tell_client_StringID ) );
    g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.Give, ::rumGetString( talk_give_client_StringID ) );

    if( !bInParty )
    {
      g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.PartyInvite,
                                        ::rumGetString( talk_invite_client_StringID ) );
    }
    else if( ciPlayer.GetID() == g_ciCUO.m_uiPartyLeaderID )
    {
      g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.PartyDismiss,
                                        ::rumGetString( talk_dismiss_client_StringID ) );
      g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.PartyPromote,
                                        ::rumGetString( talk_promote_client_StringID ) );
    }

    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

    g_ciUI.m_ciGameListView.m_funcAccept = U1_PlayerTransactionTypeSelected;
    g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();

    g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
    g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciTarget.GetID();
  }
}


function U1_PlayerTransactionTypeSelected()
{
  // Push the selection
  local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strDesc );
  g_ciUI.m_ciGameInputTextBox.Clear();

  local eType = g_ciUI.m_ciGameListView.GetSelectedKey();
  local uiPlayerID = g_ciUI.m_ciGameInputTextBox.m_vPayload;

  Ultima_ListSelectionEnd();

  if( rumInvalidGameID == uiPlayerID )
  {
    return;
  }

  local ciTarget = ::rumFetchPawn( uiPlayerID );
  if( null == ciTarget )
  {
    return;
  }

  switch( eType )
  {
    case PlayerTransactionType.Tell:
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Player_Chat );
      g_ciUI.m_ciGameInputTextBox.SetText( format( "/tell %s ", ciTarget.GetPlayerName() ) );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case PlayerTransactionType.Give:
      U1_PlayerTransactionGive( ciTarget );
      break;

    case PlayerTransactionType.PartyInvite:
      local strCommand = "/party invite " + ciTarget.GetPlayerName();
      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strCommand );
      ::rumSendBroadcast( ciBroadcast );
      Ultima_ListSelectionEnd();
      break;

    case PlayerTransactionType.PartyDismiss:
      local strCommand = "/party dismiss " + ciTarget.GetPlayerName();
      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strCommand );
      ::rumSendBroadcast( ciBroadcast );
      Ultima_ListSelectionEnd();
      break;

    case PlayerTransactionType.PartyPromote:
      local strCommand = "/party promote " + ciTarget.GetPlayerName();
      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strCommand );
      ::rumSendBroadcast( ciBroadcast );
      Ultima_ListSelectionEnd();
      break;
  }
}


function U1_PlayerTransactionGive( i_ciTarget )
{
  local ciBroadcast = ::rumCreate( Player_Give_BroadcastID, TradeBroadcastType.GiveNotify, i_ciTarget.GetID() );
  ::rumSendBroadcast( ciBroadcast );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  for( local i = 0; i < U1_ClientTradeType.NumTradeTypes; ++i )
  {
    local strEntry = format( "u1_give%d_client_StringID", i );
    g_ciUI.m_ciGameListView.SetEntry( i, ::rumGetStringByName( strEntry ) );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U1_PlayerTransactionGiveTypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  ciBroadcast = ::rumCreate( Player_Give_BroadcastID, TradeBroadcastType.Give, i_ciTarget.GetID() );
  ::rumSendBroadcast( ciBroadcast );

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = ciBroadcast;
}


function U1_PlayerTransactionGiveEquipment( i_ciBroadcast )
{
  i_ciBroadcast.SetItemSubtype( U1_ClientTradeType.Equipment );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  for( local i = 0; i < g_eU1EquipmentPropertyArray.len(); ++i )
  {
    local ciAsset = ::rumGetPropertyAsset( g_eU1EquipmentPropertyArray[i] );
    local strName = ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" );
    g_ciUI.m_ciGameListView.SetEntry( i, strName );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U1_PlayerTransactionGiveSubtypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciBroadcast;

  U1_Stat_Update( U1_StatPage.Equipment );
}


function U1_PlayerTransactionGiveSubtypeSelected()
{
  local ciBroadcast = g_ciUI.m_ciGameInputTextBox.m_vPayload;
  local iIndex = g_ciUI.m_ciGameListView.GetSelectedIndex();

  Ultima_ListSelectionEnd();

  // Reset the subtype to a class based on the current subtype enum
  local ePropertyID = rumInvalidAssetID;
  switch( ciBroadcast.GetItemSubtype() )
  {
    case U1_ClientTradeType.Equipment:
      ePropertyID = g_eU1EquipmentPropertyArray[iIndex];
      break;
  }

  if( ePropertyID != rumInvalidAssetID )
  {
    local ciProperty = ::rumGetPropertyAsset( ePropertyID );
    local strDesc = ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" );
    ShowString( strDesc );
    g_ciUI.m_ciGameInputTextBox.Clear();

    ciBroadcast.SetItemSubtype( ePropertyID );

    ShowString( ::rumGetString( talk_how_much_client_StringID ) );
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
  }
  else
  {
    ShowString( ::rumGetString( msg_none_owned_client_StringID ), g_strColorTagArray.Red );
  }
}


function U1_PlayerTransactionGiveTypeSelected()
{
  local ciBroadcast = g_ciUI.m_ciGameInputTextBox.m_vPayload;

  // Push the selection
  local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strDesc );
  g_ciUI.m_ciGameInputTextBox.Clear();

  local eType = g_ciUI.m_ciGameListView.GetSelectedKey();

  Ultima_ListSelectionEnd();

  switch( eType )
  {
    case U1_ClientTradeType.Food:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      ciBroadcast.SetItemSubtype( U1_Food_PropertyID );
      ShowString( ::rumGetString( talk_how_much_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case U1_ClientTradeType.Coin:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      ciBroadcast.SetItemSubtype( U1_Coin_PropertyID );
      ShowString( ::rumGetString( talk_how_much_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case U1_ClientTradeType.Equipment:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      U1_PlayerTransactionGiveEquipment( ciBroadcast );
      break;

    case U1_ClientTradeType.Weapon:
      ciBroadcast.SetItemType( SharedTradeType.Inventory );
      Ultima_PlayerTransactionGiveWeapon( GameType.Ultima1, ciBroadcast );
      break;

    case U1_ClientTradeType.Armour:
      ciBroadcast.SetItemType( SharedTradeType.Inventory );
      Ultima_PlayerTransactionGiveArmour( GameType.Ultima1, ciBroadcast );
      break;
  }
}


function U1_RenderGame()
{
  local ciPlayer = ::rumGetMainPlayer();
  if( null == ciPlayer )
  {
    return;
  }

  local ciMap = ciPlayer.GetMap();
  if( null == ciMap )
  {
    return;
  }

  ::rumGetGraphic( Border_GraphicID ).Draw( rumPoint() );

  local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
  if( eMapType != MapType.Space )
  {
    local ciMoonPhase = ::rumGetGraphic( Moon_Phases_GraphicID );
    ciMoonPhase.DrawAnimation( g_ciUI.m_ciMoonDisplayPos, 0, g_ciCUO.m_eTrammelPhase );
    ciMoonPhase.DrawAnimation( rumPoint( g_ciUI.m_ciMoonDisplayPos.x + g_ciUI.s_iBorderPixelWidth,
                                         g_ciUI.m_ciMoonDisplayPos.y ),
                               0, g_ciCUO.m_eFeluccaPhase );
  }

  if( ( GameMode.Game == g_ciCUO.m_eCurrentGameMode ) && !RenderGameOverride() )
  {
    local bPeering = g_ciCUO.m_bPeering || ciMap.GetProperty( Map_Always_Peer_PropertyID, false );
    local ciPos = ciPlayer.GetPosition();
    local iIntensity = ciPlayer.GetScreenShakeIntensity().tointeger();

    // Determine pixel offset coordinate to where the map is drawn on screen
    local iOffsetX = g_ciUI.m_ciMapRect.p1.x;
    local iOffsetY = g_ciUI.m_ciMapRect.p1.y;
    local iRadius = g_ciCUO.s_iLOSRadius;

    if( bPeering )
    {
      iOffsetX = g_ciUI.m_ciPeerMapRect.p1.x;
      iOffsetY = g_ciUI.m_ciPeerMapRect.p1.y;
      iRadius = g_ciCUO.s_iLOSRadiusPeering;
    }

    if( iIntensity > 0 )
    {
      // Determine a random offset within the +/- iIntensity range
      iOffsetX += ( ( rand() % ( iIntensity * 2 + 1 ) ) - iIntensity );
      iOffsetY += ( ( rand() % ( iIntensity * 2 + 1 ) ) - iIntensity );
    }

    ciMap.Draw( rumPoint( iOffsetX, iOffsetY ), ciPos );

    RenderNamesAndVitals( ciPlayer, U1_Hitpoints_PropertyID, U1_Mana_PropertyID, bPeering, iRadius );
    RenderOffscreenMemberIcons( ciPlayer, U1_Graphic_ID_PropertyID, iRadius );
    RenderAttacks( ciPos, bPeering );
    RenderEffects( ciPlayer );
  }

  if( ciPlayer.IsDead() )
  {
    // All the rest of the rendering applies only to living players
    return;
  }

  local bSpaceAce = ciPlayer.GetProperty( U1_Space_Enemies_Killed_PropertyID, 0 ) >= 20;
  if( bSpaceAce )
  {
    local ciSpaceAceGraphic = ::rumGetGraphic( U1_Space_Enemy_GraphicID );

    local ciGdp = ciSpaceAceGraphic.GetAttributes();
    ciGdp.DrawScaled = true;
    ciGdp.HorizontalScale = 0.5;
    ciGdp.VerticalScale = 0.5;

    ciSpaceAceGraphic.SetAttributes( ciGdp );
    ciSpaceAceGraphic.Draw( g_ciUI.m_ciAnkhPos );

    ciGdp.DrawScaled = false;
    ciGdp.HorizontalScale = 1.0;
    ciGdp.VerticalScale = 1.0;
  }
}


function U1_RenderStatAttributes()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_attributes_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05|0.5" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local ciPlayer = ::rumGetMainPlayer();
  local iOffset = 0;

  foreach( ePropertyID in g_eU1StatPropertyArray )
  {
    local ciAsset = ::rumGetPropertyAsset( ePropertyID );
    if( ciAsset != null )
    {
      local iValue = ciPlayer.GetProperty( ePropertyID, 0 );
      local strEntry = ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" );
      g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iValue );
    }
  }
}


function U1_RenderStatItems()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_items_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05|0.5" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local strEntry;
  local iOffset = 0;

  local ciPlayer = ::rumGetMainPlayer();
  local iGems = ciPlayer.GetProperty( U1_Gems_PropertyID, 0 );

  for( local i = 0; i < U1_GemType.NumGems; ++i )
  {
    if( ::rumBitOn( iGems, i ) )
    {
      strEntry = ::rumGetStringByName( format( "U1_Gem_%d_Property_client_StringID", i ) );
      g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry );
    }
  }

  local bSpaceAce = ciPlayer.GetProperty( U1_Space_Enemies_Killed_PropertyID, 0 ) >= 20;
  if( bSpaceAce )
  {
    strEntry = ::rumGetString( msg_space_ace_client_StringID );
    g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry );
  }
}


function U1_RenderStatMain()
{
  local ciPlayer = ::rumGetMainPlayer();
  g_ciUI.m_ciStatLabel.SetText( ciPlayer.GetPlayerName() );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05|0.5" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local iHitpoints = ciPlayer.GetProperty( U1_Hitpoints_PropertyID, 0 );
  local iMaxHitpoints = ciPlayer.GetMaxHitpoints();

  local iExp = ciPlayer.GetProperty( U1_Experience_PropertyID, 0 );
  local iLevel = iExp / 1000 + 1;

  local iOffset = 0;

  local strEntry = ::rumGetString( Hitpoints_Property_client_StringID );
  local strColor = g_strColorTagArray.White;
  if( iMaxHitpoints * 0.25 > iHitpoints )
  {
    strColor = g_strColorTagArray.Red;
  }
  else if( iMaxHitpoints * 0.5 > iHitpoints )
  {
    strColor = g_strColorTagArray.Yellow;
  }

  local strHitpoints = format( "%s%d%s/ %d", strColor, iHitpoints, g_strColorTagArray.White, iMaxHitpoints );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + strHitpoints );

  local iMaxMana = ciPlayer.GetMaxMana();
  if( iMaxMana > 0 )
  {
    local iMana = ciPlayer.GetProperty( U1_Mana_PropertyID, 0 );

    strEntry = ::rumGetString( Mana_Property_client_StringID );
    strColor = g_strColorTagArray.White;
    if( iMaxMana * 0.25 > iMana )
    {
      strColor = g_strColorTagArray.Red;
    }
    else if( iMaxMana * 0.5 > iMana )
    {
      strColor = g_strColorTagArray.Yellow;
    }

    local strMana = format( "%s%d%s/ %d", strColor, iMana, g_strColorTagArray.White, iMaxMana );
    g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + strMana );
  }

  local ePlayerClassID = ciPlayer.GetProperty( U1_PlayerClass_PropertyID, U1_Fighter_Class_CustomID );
  local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
  local strClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );

  strEntry = ::rumGetString( U4_PlayerClass_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + strClass );

  strEntry = ::rumGetString( U4_Level_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iLevel );

  strEntry = ::rumGetString( U4_Experience_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iExp );

  local iCoin = ciPlayer.GetProperty( U1_Coin_PropertyID, 0 );
  local iCopper = iCoin % 10;
  local iSilver = ( iCoin / 10 ) % 10;
  local iGold = iCoin / 100;

  if( iCopper != 0 )
  {
    strEntry = ::rumGetString( token_copper_pieces_client_StringID );
    g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iCopper );
  }

  if( iSilver != 0 )
  {
    strEntry = ::rumGetString( token_silver_pieces_client_StringID );
    g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iSilver );
  }

  if( iGold != 0 )
  {
    strEntry = ::rumGetString( token_gold_crowns_client_StringID );
    g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iGold );
  }
}


function U1_RenderStatTransports()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_transports_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();
  g_ciUI.m_ciStatListView.SetFormat( "0.05" );

  local ciPlayer = ::rumGetMainPlayer();
  local iIndex = 0;

  if( ciPlayer.GetProperty( U1_Aircar_PropertyID, false ) )
  {
    local strName = ::rumGetString( U1_Aircar_Widget_client_StringID );
    strName = strName.slice( 0, 1 ).toupper() + strName.slice( 1, strName.len() );
    g_ciUI.m_ciStatListView.SetEntry( iIndex++, strName );
  }

  if( ciPlayer.GetProperty( U1_Cart_PropertyID, false ) )
  {
    local strName = ::rumGetString( U1_Cart_Widget_client_StringID );
    strName = strName.slice( 0, 1 ).toupper() + strName.slice( 1, strName.len() );
    g_ciUI.m_ciStatListView.SetEntry( iIndex++, strName );
  }

  if( ciPlayer.GetProperty( U1_Horse_PropertyID, false ) )
  {
    local strName = ::rumGetString( U1_Horse_Widget_client_StringID );
    strName = strName.slice( 0, 1 ).toupper() + strName.slice( 1, strName.len() );
    g_ciUI.m_ciStatListView.SetEntry( iIndex++, strName );
  }

  if( ciPlayer.GetProperty( U1_Raft_PropertyID, false ) )
  {
    local strName = ::rumGetString( U1_Raft_Widget_client_StringID );
    strName = strName.slice( 0, 1 ).toupper() + strName.slice( 1, strName.len() );
    g_ciUI.m_ciStatListView.SetEntry( iIndex++, strName );
  }

  if( ciPlayer.GetProperty( U1_Shuttle_Pass_PropertyID, false ) )
  {
    local strName = ::rumGetString( U1_Shuttle_Pass_Property_client_StringID );
    g_ciUI.m_ciStatListView.SetEntry( iIndex++, strName );
  }
}


function U1_ShowAssociatedMap()
{
  g_ciUI.m_eOverlayGraphicID = rumInvalidAssetID;

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    return;
  }

  local ciMap = ciPlayer.GetMap();
  if( ciMap.GetAssetID() == U1_Sosaria_MapID )
  {
    local iHorizMid = ciMap.GetNumColumns() / 2;
    local iVertMid = ciMap.GetNumRows() / 2;

    local ciPos = ciPlayer.GetPosition();
    if( ciPos.x < iHorizMid )
    {
      if( ciPos.y < iVertMid )
      {
        g_ciUI.m_eOverlayGraphicID = U1_Map_Lord_British_GraphicID;
      }
      else
      {
        g_ciUI.m_eOverlayGraphicID = U1_Map_Dark_Unknown_GraphicID;
      }
    }
    else
    {
      if( ciPos.y < iVertMid )
      {
        g_ciUI.m_eOverlayGraphicID = U1_Map_Feudal_Lords_GraphicID;
      }
      else
      {
        g_ciUI.m_eOverlayGraphicID = U1_Map_Danger_Despair_GraphicID;
      }
    }
  }
  else
  {
    g_ciUI.m_eOverlayGraphicID = ciMap.GetProperty( Map_GraphicID_PropertyID, rumInvalidAssetID );
  }

  if( g_ciUI.m_eOverlayGraphicID != rumInvalidAssetID )
  {
    g_ciUI.m_ciOverlayGraphicOffset = rumPoint( 0, 0 );
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnOverlayGraphicDone );
  }
  else
  {
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
  }
}


function U1_Stat()
{
  local strDesc = ::rumGetString( command_zstats_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  // Bookmark the current page
  local eCurrentPage = g_ciUI.m_eCurrentStatPage;

  ++g_ciUI.m_eCurrentStatPage;

  if( ( U1_StatPage.Party == g_ciUI.m_eCurrentStatPage ) && ( null == g_ciCUO.m_uiPartyIDTable ) )
  {
    // Don't show the party page if the player is not part of a party
    ++g_ciUI.m_eCurrentStatPage;
  }

  if( g_ciUI.m_eCurrentStatPage >= U1_StatPage.NumPages )
  {
    g_ciUI.m_eCurrentStatPage = U1_StatPage.Main;
  }
  else
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( ciPlayer.IsDead() && g_ciUI.m_eCurrentStatPage >= U1_StatPage.Party )
    {
      g_ciUI.m_eCurrentStatPage = U1_StatPage.Main;
    }
  }

  if( eCurrentPage != g_ciUI.m_eCurrentStatPage )
  {
    U1_Stat_Update( g_ciUI.m_eCurrentStatPage );
  }
}


function U1_Stat_Update( i_eStatPage )
{
  g_ciUI.m_eCurrentStatPage = i_eStatPage;

  switch( i_eStatPage )
  {
    case U1_StatPage.Main:       U1_RenderStatMain(); break;
    case U2_StatPage.Party:      RenderStatParty( U1_Graphic_ID_PropertyID,
                                                  U4_Adventurer_GraphicID,
                                                  U1_PlayerClass_PropertyID,
                                                  U1_Fighter_Class_CustomID ); break;
    case U1_StatPage.Attributes: U1_RenderStatAttributes(); break;
    case U1_StatPage.Weapons:    RenderStatWeapons( GameType.Ultima1 ); break;
    case U1_StatPage.Armour:     RenderStatArmour( GameType.Ultima1 ); break;
    case U1_StatPage.Equipment:  RenderStatEquipment( g_eU1EquipmentPropertyArray ); break;
    case U1_StatPage.Items:      U1_RenderStatItems(); break;
    case U1_StatPage.Spells:     RenderStatSpells( token_spells_client_StringID,
                                                   g_eU1SpellArray,
                                                   g_eU1SpellPropertyArray ); break;
    case U1_StatPage.Transports: U1_RenderStatTransports(); break;
    default:                     U1_RenderStatMain(); break;
  }
}


function U1_UseItem()
{
  local strDesc = ::rumGetString( command_use_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameListView.Clear();

  // The map, which is always available
  g_ciUI.m_ciGameListView.SetEntry( 0, ::rumGetString( token_map_client_StringID ) );

  if( ciPlayer.GetTransportID() == rumInvalidGameID )
  {
    if( ciPlayer.GetProperty( U1_Aircar_PropertyID, false ) )
    {
      local iLastDismountTime = ciPlayer.GetProperty( U1_Aircar_Deboard_Time_PropertyID, 0 );
      if( ::rumGetSecondsSinceEpoch() - iLastDismountTime > Transport_Widget.s_fIdleInterval )
      {
        local strEntry = ::rumGetString( U1_Aircar_Widget_client_StringID );
        strEntry = strEntry.slice( 0, 1 ).toupper() + strEntry.slice( 1, strEntry.len() );
        g_ciUI.m_ciGameListView.SetEntry( U1_Aircar_PropertyID, strEntry );
      }
    }

    if( ciPlayer.GetProperty( U1_Cart_PropertyID, false ) )
    {
      local iLastDismountTime = ciPlayer.GetProperty( U1_Cart_Deboard_Time_PropertyID, 0 );
      if( ::rumGetSecondsSinceEpoch() - iLastDismountTime > Transport_Widget.s_fIdleInterval )
      {
        local strEntry = ::rumGetString( U1_Cart_Widget_client_StringID );
        strEntry = strEntry.slice( 0, 1 ).toupper() + strEntry.slice( 1, strEntry.len() );
        g_ciUI.m_ciGameListView.SetEntry( U1_Cart_PropertyID, strEntry );
      }
    }

    if( ciPlayer.GetProperty( U1_Horse_PropertyID, false ) )
    {
      local iLastDismountTime = ciPlayer.GetProperty( U1_Horse_Dismount_Time_PropertyID, 0 );
      if( ::rumGetSecondsSinceEpoch() - iLastDismountTime > Transport_Widget.s_fIdleInterval )
      {
        local strEntry = ::rumGetString( U1_Horse_Widget_client_StringID );
        strEntry = strEntry.slice( 0, 1 ).toupper() + strEntry.slice( 1, strEntry.len() );
        g_ciUI.m_ciGameListView.SetEntry( U1_Horse_PropertyID, strEntry );
      }
    }

    if( ciPlayer.GetProperty( U1_Raft_PropertyID, false ) )
    {
      local iLastDismountTime = ciPlayer.GetProperty( U1_Raft_Deboard_Time_PropertyID, 0 );
      if( ::rumGetSecondsSinceEpoch() - iLastDismountTime > Transport_Widget.s_fIdleInterval )
      {
        local strEntry = ::rumGetString( U1_Raft_Widget_client_StringID );
        strEntry = strEntry.slice( 0, 1 ).toupper() + strEntry.slice( 1, strEntry.len() );
        g_ciUI.m_ciGameListView.SetEntry( U1_Raft_PropertyID, strEntry );
      }
    }
  }

  local eTransportWidgetID = ciPlayer.GetProperty( U1_Transport_Widget_PropertyID, rumInvalidAssetID );
  if( eTransportWidgetID != rumInvalidAssetID )
  {
    local ciMap = ciPlayer.GetMap();

    local eMapID = ciPlayer.GetProperty( U1_Transport_Map_PropertyID, rumInvalidAssetID );
    if( eMapID != rumInvalidAssetID && ( ciMap.GetAssetID() == eMapID ) )
    {
      g_ciUI.m_ciGameListView.SetEntry( U1_Transport_Widget_PropertyID, "Summon Transport" );
    }
  }

  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.ShowPrompt( false );

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U1_UseItemCallback;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();
}


function U1_UseItemCallback()
{
  g_ciUI.m_ciGameInputTextBox.Clear();

  local strSelected = g_ciUI.m_ciGameListView.GetCurrentEntry();
  ShowString( strSelected );

  local ePropertyID = g_ciUI.m_ciGameListView.GetSelectedKey();
  if( 0 == ePropertyID )
  {
    // The map is being used
    U1_ShowAssociatedMap();
  }
  else
  {
    if( ( U1_Aircar_PropertyID == ePropertyID ) ||
        ( U1_Horse_PropertyID == ePropertyID )  ||
        ( U1_Cart_PropertyID == ePropertyID )   ||
        ( U1_Raft_PropertyID == ePropertyID ) )
    {
      // Is the map compatible with a horse?
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();
      local eMapAssetID = ciMap.GetAssetID();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( ( MapType.Dungeon == eMapType )          ||
          ( MapType.Space == eMapType )            ||
          ( MapType.SpaceStation == eMapType )     ||
          ( U1_Time_Machine_MapID == eMapAssetID ) ||
          ( U1_Lair_Mondain_MapID == eMapAssetID ) )
      {
        ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
        Ultima_ListSelectionEnd();
        return;
      }
    }

    // Send the class id of the item used to the server
    local ciBroadcast = ::rumCreate( Player_Use_BroadcastID, ePropertyID );
    ::rumSendBroadcast( ciBroadcast );
  }

  Ultima_ListSelectionEnd( ActionDelay.Short );
}
