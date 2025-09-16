/*

----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88                                ooooo ooooo
888    88   888  o888oo  oooo  oo ooo oooo     ooooooo       888   888
888    88   888   888     888   888 888 888    ooooo888      888   888
888    88   888   888     888   888 888 888  888    888      888   888
888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o    ooooo ooooo

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

Ultima II: The Revenge of the Enchantress
Developer:  Richard Garriott
Publishers: Sierra On-Line, Origin Systems (re-release)
Designer:   Richard Garriott

*/

g_eU2QuickswordMaterialStringArray <-
[
  u2_quicksword_material0_client_StringID,
  u2_quicksword_material1_client_StringID,
  u2_quicksword_material2_client_StringID,
]

g_eU2StatDescriptionArray <-
[
  msg_strength_increased_client_StringID,
  msg_agility_increased_client_StringID,
  msg_wisdom_increased_client_StringID,
  msg_stamina_increased_client_StringID,
  msg_charisma_increased_client_StringID,
  msg_intelligence_increased_client_StringID
]


function U2_Animate()
{
  local ciVector = GetDirectionVector( g_ciCUO.m_eWindDirection );

  // Shift water in the direction of the wind
  ::rumGetGraphic( U2_Water_GraphicID ).Shift( ciVector );

  // Shift other scrolling tiles down
  ::rumGetGraphic( U2_Force_Field_GraphicID ).Shift( rumVector( 0, 1 ) );
}


function U2_CastSpell()
{
  local strDesc = ::rumGetString( command_cast_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();

  local ePlayerClassID = ciPlayer.GetProperty( U2_PlayerClass_PropertyID, U2_Fighter_Class_CustomID );
  local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );

  local bCastsSpells = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
  if( !bCastsSpells )
  {
    local strClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );
    strDesc = format( ::rumGetString( msg_cast_restricted_client_StringID ), strClass );
    ShowString( strDesc, g_strColorTagArray.Red );
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
  foreach( iIndex, eSpellID in g_eU2SpellArray )
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

  g_ciUI.m_ciGameListView.m_funcAccept = U2_CastSpellSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = U2_CastSpellFinished;
  g_ciUI.m_ciGameListView.m_funcIndexChanged = Ultima_CastSpellSelectionChanged;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  // Switch to the spells stat page
  g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;
  U2_Stat_Update( U2_StatPage.Spells );
}


function U2_CastSpellFinished(i_eDelay = ActionDelay.Short )
{
  // Restore the player's previous stat page
  U2_Stat_Update( g_ciUI.m_ePreviousStatPage );
  Ultima_ListSelectionEnd( i_eDelay );
}


function U2_CastSpellSelected()
{
  local eKey = g_ciUI.m_ciGameListView.GetSelectedKey();
  local eSpellID = g_eU2SpellArray[eKey];
  local ciPlayer = ::rumGetMainPlayer();
  local bDone = ciPlayer.CastSpell( eSpellID );

  if( bDone )
  {
    U2_CastSpellFinished();
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


function U2_Endgame( i_ciMessageArray )
{
  if( i_ciMessageArray[1] < i_ciMessageArray[0].len() )
  {
    ShowString( i_ciMessageArray[0][i_ciMessageArray[1]] );

    if( ( i_ciMessageArray[1] + 1 ) < i_ciMessageArray[0].len() )
    {
      ++i_ciMessageArray[1];
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, U2_Endgame, i_ciMessageArray );
    }
  }
}


function U2_Hyperjump()
{
  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local ciMap = ciPlayer.GetMap();
  local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
  if( eMapType != MapType.Space )
  {
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local ciTransport = ciPlayer.GetTransport();
  if( null == ciTransport )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local eTransportType = ciTransport.GetType();
  if( TransportType.Rocket != eTransportType )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  ShowString( ::rumGetString( command_hyperwarp_client_StringID ) );
  ShowString( ::rumGetString( command_xeno_client_StringID ) );

  g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, U2_Hyperjump_Xeno );
}


function U2_Hyperjump_Xeno( i_iCoordinate )
{
  ShowString( i_iCoordinate.tostring() );
  g_ciUI.m_ciGameInputTextBox.Clear();

  if( i_iCoordinate >= 0 )
  {
    local iCoordinateArray = [i_iCoordinate, 0, 0];
    ShowString( ::rumGetString( command_yako_client_StringID ) );
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, U2_Hyperjump_Yako, iCoordinateArray );
  }

}


function U2_Hyperjump_Yako( i_iCoordinateArray, i_iCoordinate )
{
  ShowString( i_iCoordinate.tostring() );
  g_ciUI.m_ciGameInputTextBox.Clear();

  if( i_iCoordinate >= 0 )
  {
    i_iCoordinateArray[1] = i_iCoordinate;
    ShowString( ::rumGetString( command_zabo_client_StringID ) );
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, U2_Hyperjump_Zabo, i_iCoordinateArray );
  }
}


function U2_Hyperjump_Zabo( i_iCoordinateArray, i_iCoordinate )
{
  ShowString( i_iCoordinate.tostring() );
  g_ciUI.m_ciGameInputTextBox.Clear();

  if( i_iCoordinate >= 0 )
  {
    i_iCoordinateArray[2] = i_iCoordinate;

    // Hyperjump!
    local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.HyperJump,
                                     i_iCoordinateArray );
    ::rumSendBroadcast( ciBroadcast );
  }
}


function U2_Init()
{}


function U2_KeyPressedGame( i_ciKeyboard )
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
    U2_CastSpell();
  }
  else if( rumKeypress.KeyD() == eKey )
  {
    Ultima_Descend();
  }
  else if( rumKeypress.KeyE() == eKey )
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( ciPlayer.IsFlying() )
    {
      ShowString( ::rumGetString( msg_must_land_client_StringID ), g_strColorTagArray.Red );
    }
    else
    {
      UsePortal( PortalUsageType.Enter );
    }
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
        if( ( U2_Chest_WidgetID == eAssetID ) ||
            ( U2_Sword_WidgetID == eAssetID ) ||
            ( U2_Shield_WidgetID == eAssetID ) )
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
    U2_Hyperjump();
  }
  else if( rumKeypress.KeyI() == eKey )
  {
    if( i_ciKeyboard.AltPressed() )
    {
      Ultima_ExtinguishLight();
    }
    else
    {
      Ultima_IgniteTorch( U2_Torches_PropertyID );
    }
  }
  else if( rumKeypress.KeyJ() == eKey )
  {
    // In the original game, this was Jump Up and Down
    Ultima_JimmyLock( U2_Door_Widget, U2_Keys_PropertyID );
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
      // In the original game, this was Launch & Land
      Ultima_Look();
    }
  }
  else if( rumKeypress.KeyM() == eKey )
  {
    if( i_ciKeyboard.AltPressed() )
    {
      local ciPlayer = ::rumGetMainPlayer();
      if( !( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() ) )
      {
        g_ciUI.m_eOverlayGraphicID = U2_Map_GraphicID;
        g_ciUI.m_ciOverlayGraphicOffset = rumPoint( 0, 0 );
        g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnOverlayGraphicDone );
      }
    }
    else
    {
      U2_Magic();
    }
  }
  else if( rumKeypress.KeyN() == eKey )
  {
    // In the original game, this was Negate Time
    local bOnSelf = true;
    if( i_ciKeyboard.AltPressed() )
    {
      bOnSelf = false;
    }

    Ultima_NipPotion( U2_Potions_PropertyID, bOnSelf );
  }
  else if( rumKeypress.KeyO() == eKey )
  {
    // In the original game, this was Offer Gold
    Ultima_Open( U2_Door_Widget );
  }
  else if( rumKeypress.KeyP() == eKey )
  {
    // In the original game, this was V for View
    Ultima_Peer( U2_Helms_PropertyID );
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
    U2_UseItem();
  }
  else if( rumKeypress.KeyV() == eKey )
  {
    // In the original game, V was used to View
    Ultima_Volume();
  }
  else if( rumKeypress.KeyW() == eKey )
  {
    Ultima_WearArmour();
  }
  else if( rumKeypress.KeyX() == eKey )
  {
    Ultima_Xit();
  }
  else if( rumKeypress.KeyY() == eKey )
  {
    Ultima_Yell();
  }
  else if( rumKeypress.KeyZ() == eKey )
  {
    if( i_ciKeyboard.AltPressed() || i_ciKeyboard.CtrlPressed() )
    {
      // Alt-Z always shows the main stat panel
      U2_Stat_Update( U2_StatPage.Main );
    }
    else
    {
      U2_Stat();
    }
  }
  else if( rumKeypress.KeyTab() == eKey )
  {
    Ultima_AdvanceMapLabel();
  }
  else if( ( rumKeypress.KeyPadSubtract() == eKey ) || ( rumKeypress.KeyDash() == eKey ) )
  {
    Ultima_Resurrect( ResurrectionType.Void, GameType.Ultima2 );
  }
  else if( ( rumKeypress.KeyPadAdd() == eKey ) || ( rumKeypress.KeyEquals() == eKey ) )
  {
    Ultima_Resurrect( ResurrectionType.Body, GameType.Ultima2 );
  }
}


function U2_Magic()
{
  // TODO - Magic readies a magic spell you know for casting
}


function U2_PlayerTransaction( i_ciTarget )
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

    g_ciUI.m_ciGameListView.m_funcAccept = U2_PlayerTransactionTypeSelected;
    g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();

    g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
    g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciTarget.GetID();
  }
}


function U2_PlayerTransactionGive( i_ciTarget )
{
  local ciBroadcast = ::rumCreate( Player_Give_BroadcastID, TradeBroadcastType.GiveNotify, i_ciTarget.GetID() );
  ::rumSendBroadcast( ciBroadcast );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  for( local i = 0; i < U2_ClientTradeType.NumTradeTypes; ++i )
  {
    local strEntry = format( "u2_give%d_client_StringID", i );
    g_ciUI.m_ciGameListView.SetEntry( i, ::rumGetStringByName( strEntry ) );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U2_PlayerTransactionGiveTypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  ciBroadcast = ::rumCreate( Player_Give_BroadcastID, TradeBroadcastType.Give, i_ciTarget.GetID() );
  ::rumSendBroadcast( ciBroadcast );

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = ciBroadcast;
}


function U2_PlayerTransactionGiveEquipment( i_ciBroadcast )
{
  i_ciBroadcast.SetItemSubtype( U2_ClientTradeType.Equipment );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  for( local i = 0; i < g_eU2EquipmentPropertyArray.len(); ++i )
  {
    local ciAsset = ::rumGetPropertyAsset( g_eU2EquipmentPropertyArray[i] );
    local strName = ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" );
    g_ciUI.m_ciGameListView.SetEntry( i, strName );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U2_PlayerTransactionGiveSubtypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciBroadcast;

  U2_Stat_Update( U2_StatPage.Equipment );
}


function U2_PlayerTransactionGiveSubtypeSelected()
{
  local ciBroadcast = g_ciUI.m_ciGameInputTextBox.m_vPayload;
  local iIndex = g_ciUI.m_ciGameListView.GetSelectedIndex();

  Ultima_ListSelectionEnd();

  // Reset the subtype to a class based on the current subtype enum
  local ePropertyID = rumInvalidAssetID;
  switch( ciBroadcast.GetItemSubtype() )
  {
    case U2_ClientTradeType.Equipment:
      ePropertyID = g_eU2EquipmentPropertyArray[iIndex];
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


function U2_PlayerTransactionGiveTypeSelected()
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
    case U2_ClientTradeType.Food:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      ciBroadcast.SetItemSubtype( U2_Food_PropertyID );
      ShowString( ::rumGetString( talk_how_much_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case U2_ClientTradeType.Gold:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      ciBroadcast.SetItemSubtype( U2_Gold_PropertyID );
      ShowString( ::rumGetString( talk_how_much_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case U2_ClientTradeType.Equipment:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      U2_PlayerTransactionGiveEquipment( ciBroadcast );
      break;

    case U2_ClientTradeType.Weapon:
      ciBroadcast.SetItemType( SharedTradeType.Inventory );
      Ultima_PlayerTransactionGiveWeapon( GameType.Ultima2, ciBroadcast );
      break;

    case U2_ClientTradeType.Armour:
      ciBroadcast.SetItemType( SharedTradeType.Inventory );
      Ultima_PlayerTransactionGiveArmour( GameType.Ultima2, ciBroadcast );
      break;
  }
}


function U2_PlayerTransactionTypeSelected()
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
      U2_PlayerTransactionGive( ciTarget );
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


function U2_RenderGame()
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
  if( eMapType != MapType.Dungeon && eMapType != MapType.Tower )
  {
    local eTimePeriod = ciMap.GetProperty( U2_Map_Time_Period_PropertyID, null );
    if( eTimePeriod != null )
    {
      // Does the player know what period this is?
      local iFlags = ciPlayer.GetProperty( U2_TimePeriods_PropertyID, 0 );
      if( ::rumBitOn( iFlags, eTimePeriod ) )
      {
        local ciTimePeriod = ::rumGetGraphic( U2_Time_Periods_GraphicID );
        ciTimePeriod.DrawAnimation( g_ciUI.m_ciMoonDisplayPos, 0, eTimePeriod );
        if( ::rumBitOn( iFlags, g_ciCUO.m_eTimeGatePeriod ) )
        {
          ciTimePeriod.DrawAnimation( rumPoint( g_ciUI.m_ciMoonDisplayPos.x + g_ciUI.s_iBorderPixelWidth,
                                                g_ciUI.m_ciMoonDisplayPos.y ),
                                      0, g_ciCUO.m_eTimeGatePeriod );
        }
      }
    }
  }

  if( ( GameMode.Game == g_ciCUO.m_eCurrentGameMode ) && !RenderGameOverride() )
  {
    local bPeering = ( g_ciCUO.m_bAloft || g_ciCUO.m_bPeering );
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

    RenderNamesAndVitals( ciPlayer, U2_Hitpoints_PropertyID, U2_Mana_PropertyID, bPeering, iRadius );
    RenderOffscreenMemberIcons( ciPlayer, U2_Graphic_ID_PropertyID, iRadius );
    RenderAttacks( ciPos, g_ciCUO.m_bPeering );
    RenderEffects( ciPlayer);
  }

  if( ciPlayer.IsDead() )
  {
    // All the rest of the rendering applies only to living players
    return;
  }

  local bHasRing = ciPlayer.GetProperty( U2_Magic_Ring_PropertyID, false );
  if( bHasRing )
  {
    ::rumGetGraphic( U2_Ring_UI_GraphicID ).Draw( rumPoint( g_ciUI.m_ciAnkhPos.x, g_ciUI.m_ciAnkhPos.y ) );
  }
}


function U2_RenderStatAttributes()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_attributes_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05|0.5" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local ciPlayer = ::rumGetMainPlayer();
  local iOffset = 0;

  foreach( ePropertyID in g_eU2StatPropertyArray )
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


function U2_RenderStatItems()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_items_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local iOffset = 0;
  local ciPlayer = ::rumGetMainPlayer();

  local bHasRing = ciPlayer.GetProperty( U2_Magic_Ring_PropertyID, false );
  if( bHasRing )
  {
    local strEntry = ::rumGetString( U2_Magic_Ring_Property_client_StringID );
    g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
    ++iOffset;
  }

  local iFlags = ciPlayer.GetProperty( U2_Item_Quicksword_Materials_PropertyID, 0 );
  if( iFlags )
  {
    for( local eMaterial = 0; eMaterial < g_eU2QuickswordMaterialStringArray.len(); ++eMaterial )
    {
      if( ::rumBitOn( iFlags, eMaterial ) )
      {
        local strEntry = ::rumGetString( g_eU2QuickswordMaterialStringArray[eMaterial] );
        g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
        ++iOffset
      }
    }
  }
}


function U2_RenderStatMain()
{
  local ciPlayer = ::rumGetMainPlayer();
  g_ciUI.m_ciStatLabel.SetText( ciPlayer.GetPlayerName() );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05|0.5" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local iHitpoints = ciPlayer.GetProperty( U2_Hitpoints_PropertyID, 0 );
  local iMaxHitpoints = ciPlayer.GetMaxHitpoints();

  local iExp = ciPlayer.GetProperty( U2_Experience_PropertyID, 0 );
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
    local iMana = ciPlayer.GetProperty( U2_Mana_PropertyID, 0 );

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

  local ePlayerClassID = ciPlayer.GetProperty( U2_PlayerClass_PropertyID, U2_Fighter_Class_CustomID );
  local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
  local strClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );

  strEntry = ::rumGetString( U4_PlayerClass_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + strClass );

  strEntry = ::rumGetString( U4_Level_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iLevel );

  strEntry = ::rumGetString( U4_Experience_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iExp );
}


function U2_Stat()
{
  local strDesc = ::rumGetString( command_zstats_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  // Bookmark the current page
  local eCurrentPage = g_ciUI.m_eCurrentStatPage;

  ++g_ciUI.m_eCurrentStatPage;

  if( ( U2_StatPage.Party == g_ciUI.m_eCurrentStatPage ) && ( null == g_ciCUO.m_uiPartyIDTable ) )
  {
    // Don't show the party page if the player is not part of a party
    ++g_ciUI.m_eCurrentStatPage;
  }

  if( g_ciUI.m_eCurrentStatPage >= U2_StatPage.NumPages )
  {
    g_ciUI.m_eCurrentStatPage = U2_StatPage.Main;
  }
  else
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( ciPlayer.IsDead() && g_ciUI.m_eCurrentStatPage >= U2_StatPage.Party )
    {
      g_ciUI.m_eCurrentStatPage = U2_StatPage.Main;
    }
  }

  if( eCurrentPage != g_ciUI.m_eCurrentStatPage )
  {
    U2_Stat_Update( g_ciUI.m_eCurrentStatPage );
  }
}


function U2_Stat_Update( i_eStatPage )
{
  g_ciUI.m_eCurrentStatPage = i_eStatPage;

  switch( i_eStatPage )
  {
    case U2_StatPage.Main:       U2_RenderStatMain(); break;
    case U2_StatPage.Party:      RenderStatParty( U2_Graphic_ID_PropertyID,
                                                  U4_Adventurer_GraphicID,
                                                  U2_PlayerClass_PropertyID,
                                                  U2_Fighter_Class_CustomID ); break;
    case U2_StatPage.Attributes: U2_RenderStatAttributes(); break;
    case U2_StatPage.Weapons:    RenderStatWeapons( GameType.Ultima2 ); break;
    case U2_StatPage.Armour:     RenderStatArmour( GameType.Ultima2 ); break;
    case U2_StatPage.Equipment:  RenderStatEquipment( g_eU2EquipmentPropertyArray ); break;
    case U2_StatPage.Items:      U2_RenderStatItems(); break;
    case U2_StatPage.Spells:     RenderStatSpells( token_spells_client_StringID,
                                                   g_eU2SpellArray,
                                                   g_eU2SpellPropertyArray ); break;
    default:                     U2_RenderStatMain(); break;
  }
}


function U2_UseItem()
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

  if( ciPlayer.GetProperty( U2_Horse_PropertyID, rumInvalidAssetID ) != rumInvalidAssetID &&
      ( ciPlayer.GetTransportID() == rumInvalidGameID ) )
  {
    local iLastDismountTime = ciPlayer.GetProperty( U2_Horse_Dismount_Time_PropertyID, 0 );
    if( ::rumGetSecondsSinceEpoch() - iLastDismountTime > Transport_Widget.s_fIdleInterval )
    {
      local strEntry = ::rumGetString( U2_Horse_Creature_client_StringID );
      g_ciUI.m_ciGameListView.SetEntry( U2_Horse_PropertyID, strEntry );
    }
  }

  local eTransportWidgetID = ciPlayer.GetProperty( U2_Transport_Widget_PropertyID, rumInvalidAssetID );
  if( eTransportWidgetID != rumInvalidAssetID )
  {
    local ciMap = ciPlayer.GetMap();

    local eMapID = ciPlayer.GetProperty( U2_Transport_Map_PropertyID, rumInvalidAssetID );
    if( eMapID != rumInvalidAssetID && ( ciMap.GetAssetID() == eMapID ) )
    {
      g_ciUI.m_ciGameListView.SetEntry( U2_Transport_Widget_PropertyID, "Summon Transport" );
    }
  }

  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.ShowPrompt( false );

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U2_UseItemCallback;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();
}


function U2_UseItemCallback()
{
  g_ciUI.m_ciGameInputTextBox.Clear();

  local ciPlayer = ::rumGetMainPlayer();

  local strSelected = g_ciUI.m_ciGameListView.GetCurrentEntry();
  ShowString( strSelected );

  local ePropertyID = g_ciUI.m_ciGameListView.GetSelectedKey();
  if( 0 == ePropertyID )
  {
    // The map is being used
    if( !( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() ) )
    {
      g_ciUI.m_eOverlayGraphicID = U2_Map_GraphicID;
      g_ciUI.m_ciOverlayGraphicOffset = rumPoint( 0, 0 );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnOverlayGraphicDone );
    }
  }
  else
  {
    if( U2_Horse_PropertyID == ePropertyID )
    {
      // Is the map compatible with a horse?
      local ciMap = ciPlayer.GetMap();
      local eMapAssetID = ciMap.GetAssetID();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( ( MapType.Dungeon == eMapType )   ||
          ( MapType.Tower == eMapType )     ||
          ( MapType.Space == eMapType )     ||
          ( U2_Earth_Legends_Castle_Shadow_Guard_MapID == eMapAssetID ) )
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
