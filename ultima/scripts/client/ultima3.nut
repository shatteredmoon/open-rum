/*

----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88                                ooooo ooooo ooooo
888    88   888  o888oo  oooo  oo ooo oooo     ooooooo       888   888   888
888    88   888   888     888   888 888 888    ooooo888      888   888   888
888    88   888   888     888   888 888 888  888    888      888   888   888
888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o    ooooo ooooo ooooo

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

Ultima III: Exodus
Developer:  Richard Garriott
Publisher:  Origin Systems
Designers:  Richard Garriott, Ken W. Arnold

*/

g_eU3CardStringArray <-
[
  u3_card_0_client_StringID,
  u3_card_1_client_StringID,
  u3_card_2_client_StringID,
  u3_card_3_client_StringID
]

g_eU3MarkStringArray <-
[
  u3_mark_0_client_StringID,
  u3_mark_1_client_StringID,
  u3_mark_2_client_StringID,
  u3_mark_3_client_StringID
]


function U3_Animate()
{
  local ciVector = GetDirectionVector( g_ciCUO.m_eWindDirection );

  // Shift water in the direction of the wind
  ::rumGetGraphic( U3_Water_GraphicID ).Shift( ciVector );

  ciVector = rumVector( 0, 1 );

  // Shift other scrolling tiles down
  ::rumGetGraphic( U3_Force_Field_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U3_Lava_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U3_Moongate_GraphicID ).Shift( ciVector );

  ::rumGetGraphic( U3_Mark_Rod_GraphicID ).Shift( ciVector );
}


function U3_Bribe()
{
  local ciPlayer = ::rumGetMainPlayer();

  local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
  if( iPlayerGold >= U3_NPC.s_iBribeCost )
  {
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, U3_BribeCallback );

    local strDesc = format( "%s: <%s>",
                            ::rumGetString( command_bribe_client_StringID ),
                            ::rumGetString( token_direction_client_StringID ) );
    g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
  }
  else
  {
    // Not enough gold!
    g_ciUI.m_ciGameInputTextBox.SetText( ::rumGetString( command_bribe_client_StringID ) );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
  }
}


function U3_BribeCallback( i_eDir )
{
  // Send the bribe request to the server
  local ciBroadcast = ::rumCreate( Player_Bribe_BroadcastID, i_eDir );
  ::rumSendBroadcast( ciBroadcast );

  local ciPlayer = ::rumGetMainPlayer();
  BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
}


function U3_CastSpell()
{
  local strDesc = ::rumGetString( command_cast_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();

  local ePlayerClassID = ciPlayer.GetProperty( U3_PlayerClass_PropertyID, U3_Fighter_Class_CustomID );
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

  // Determine the spell school of the class
  local eSchoolType = ciPlayerClass.GetProperty( Spell_School_PropertyID, SpellSchool.Divine );
  if( eSchoolType != SpellSchool.Both )
  {
    local eClassID = ciPlayerClass.GetProperty( Class_ID_PropertyID, 0 );
    U3_CastSpellBySchool( eSchoolType, eClassID );
  }
  else
  {
    local strDesc = ::rumGetString( u3_cast_spell_division_client_StringID );
    ShowString( strDesc );

    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.DisableMultiSelect();
    g_ciUI.m_ciGameListView.SetFormat( "0.05" );

    g_ciUI.m_ciGameListView.SetEntry( SpellSchool.Divine, ::rumGetString( token_divine_client_StringID ),
                                      rumKeypress.KeyA() );
    g_ciUI.m_ciGameListView.SetEntry( SpellSchool.Arcane, ::rumGetString( token_arcane_client_StringID ),
                                      rumKeypress.KeyB() );

    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 2 );

    g_ciUI.m_ciGameListView.m_funcAccept = U3_CastSpellSchoolSelected;
    g_ciUI.m_ciGameListView.m_funcCancel = U3_CastSpellFinished;
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();
  }
}


function U3_CastSpellBySchool( i_eSchoolType, eClassID )
{
  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.025|0.1" );

  local eSpellArray = g_eU3ArcaneSpellArray;
  if( i_eSchoolType != SpellSchool.Arcane )
  {
    eSpellArray = g_eU3DivineSpellArray;
  }

  // Build the list of spells
  foreach( iIndex, eSpellID in eSpellArray )
  {
    local ciSpell = ::rumGetCustomAsset( eSpellID );
    local iClassFlags = ciSpell.GetProperty( Spell_Class_Compatibility_Flags_PropertyID, 0 );
    if( iClassFlags & ( 1 << eClassID ) )
    {
      local strSpell = ::rumGetStringByName( ciSpell.GetName() + "_client_StringID" );
      local iSpellIndex = ciSpell.GetProperty( Spell_Key_ID_PropertyID, 0 );
      local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iSpellIndex ) );
      local strEntry = format( "%s|%s", strAlpha, strSpell );
      g_ciUI.m_ciGameListView.SetEntry( iIndex, strEntry, rumKeypress.KeyA() + iSpellIndex );
    }
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U3_CastSpellSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = U3_CastSpellFinished;
  g_ciUI.m_ciGameListView.m_funcIndexChanged = Ultima_CastSpellSelectionChanged;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = i_eSchoolType;
}


function U3_CastSpellFinished( i_eDelay = ActionDelay.Short )
{
  // Restore the player's previous stat page
  U3_Stat_Update( g_ciUI.m_ePreviousStatPage );
  Ultima_ListSelectionEnd( i_eDelay );
}


function U3_CastSpellSchoolSelected()
{
  local eKey = g_ciUI.m_ciGameListView.GetSelectedKey();

  local ciPlayer = ::rumGetMainPlayer();
  local ePlayerClassID = ciPlayer.GetProperty( U3_PlayerClass_PropertyID, U3_Fighter_Class_CustomID );
  local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
  local eClassID = ciPlayerClass.GetProperty( Class_ID_PropertyID, 0 );

  if( SpellSchool.Divine == eKey )
  {
    ShowString( format( "%s %s:",
                        ::rumGetString( token_divine_client_StringID ),
                        ::rumGetString( token_spell_client_StringID ) ) );
    U3_CastSpellBySchool( SpellSchool.Divine, eClassID );
  }
  else if( SpellSchool.Arcane == eKey )
  {
    ShowString( format( "%s %s:",
                        ::rumGetString( token_arcane_client_StringID ),
                        ::rumGetString( token_spell_client_StringID ) ) );
    U3_CastSpellBySchool( SpellSchool.Arcane, eClassID );
  }
  else
  {
    U3_CastSpellFinished();
  }

  g_ciUI.m_ciGameInputTextBox.Clear();
}


function U3_CastSpellSelected()
{
  local strText = g_ciUI.m_ciGameListView.GetCurrentEntry();
  local strArray = split( strText, "|" );
  ShowString( format( "%s: %s", ::rumGetString( token_spell_client_StringID ), strArray[1] ) );

  local eKey = g_ciUI.m_ciGameListView.GetSelectedKey();

  local eSchoolType = g_ciUI.m_ciGameInputTextBox.m_vPayload;
  local eSpellArray = g_eU3ArcaneSpellArray;
  if( eSchoolType != SpellSchool.Arcane )
  {
    eSpellArray = g_eU3DivineSpellArray;
  }

  local eSpellID = eSpellArray[eKey];

  local ciPlayer = ::rumGetMainPlayer();
  ciPlayer.CastSpell( eSpellID );

  U3_CastSpellFinished();
}


function U3_Endgame( i_ciMessageArray )
{
  if( i_ciMessageArray[1] < i_ciMessageArray[0].len() )
  {
    ShowString( i_ciMessageArray[0][i_ciMessageArray[1]] );

    if( ( i_ciMessageArray[1] + 1 ) < i_ciMessageArray[0].len() )
    {
      ++i_ciMessageArray[1];
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, U3_Endgame, i_ciMessageArray );
    }
  }
}


function U3_HandEquipment()
{
  // TODO - same as trade
}


function U3_Init()
{}


function U3_Insert()
{
  local strFailMsg = null;

  local ciPlayer = ::rumGetMainPlayer();
  local iFlags = ciPlayer.GetProperty( U3_Cards_PropertyID, 0 );
  if( iFlags != 0 )
  {
    local ciMap = ciPlayer.GetMap();
    if( ciMap.GetAssetID() == U3_Castle_Fire_2_MapID )
    {
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, U3_InsertDirCallback );

      local strDesc = format( "%s: <%s>",
                              ::rumGetString( command_insert_client_StringID ),
                              ::rumGetString( token_direction_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
    }
    else
    {
      strFailMsg = msg_not_here_client_StringID;
    }
  }
  else
  {
    strFailMsg = msg_none_owned_client_StringID;
  }

  if( strFailMsg != null )
  {
    ShowString( ::rumGetString( command_insert_client_StringID ) );
    ShowString( ::rumGetString( strFailMsg ), g_strColorTagArray.Red );
  }
}


function U3_InsertCardCallback()
{
  // Push the selection
  local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strDesc );
  g_ciUI.m_ciGameInputTextBox.Clear();

  local eDir = g_ciUI.m_ciGameInputTextBox.m_vPayload;
  local eCardType = g_ciUI.m_ciGameListView.GetSelectedKey();

  local ciBroadcast = ::rumCreate( Player_Insert_BroadcastID, eCardType, eDir );
  ::rumSendBroadcast( ciBroadcast );

  Ultima_ListSelectionEnd( ActionDelay.Long );
}


function U3_InsertDirCallback( i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciMap = ciPlayer.GetMap();

  // Is there an Exodus component at this location?
  local ciPos = ciPlayer.GetPosition() + GetDirectionVector( i_eDir );
  local ciPosData = ciMap.GetPositionData( ciPos );
  local ciExodus = ciPosData.GetNext( rumWidgetPawnType, U3_Exodus_WidgetID );
  if( ciExodus != null )
  {
    g_ciUI.m_ciGameInputTextBox.SetText( ::rumGetString( token_card_prompt_client_StringID ) );

    // Build the card list
    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.DisableMultiSelect();
    g_ciUI.m_ciGameListView.SetFormat( "0.05" );

    g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

    local iFlags = ciPlayer.GetProperty( U3_Cards_PropertyID, 0 );

    // Build the list of cards
    for( local eCard = 0; eCard < U3_CardType.NumCards; ++eCard )
    {
      if( ::rumBitOn( iFlags, eCard ) )
      {
        local strCard = format( "u3_card_%d_client_StringID", eCard );
        local strCardDesc = ::rumGetString( u3_card_client_StringID ) + " " + ::rumGetStringByName( strCard );
        g_ciUI.m_ciGameListView.SetEntry( eCard, strCardDesc );
      }
    }

    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

    g_ciUI.m_ciGameListView.m_funcAccept = U3_InsertCardCallback;
    g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();

    g_ciUI.m_ciGameInputTextBox.m_vPayload = i_eDir;
  }
  else
  {
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
    Ultima_ListSelectionEnd();
  }
}


function U3_KeyPressedGame( i_ciKeyboard )
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
    U3_CastSpell();
  }
  else if( rumKeypress.KeyD() == eKey )
  {
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
    Ultima_Get( U3_Chest_WidgetID );
  }
  else if( rumKeypress.KeyH() == eKey )
  {
    // Same as trade
    U3_HandEquipment();
  }
  else if( rumKeypress.KeyI() == eKey )
  {
    if( i_ciKeyboard.AltPressed() )
    {
      Ultima_ExtinguishLight();
    }
    else
    {
      Ultima_IgniteTorch( U3_Torches_PropertyID );
    }
  }
  else if( rumKeypress.KeyJ() == eKey )
  {
    // In the original game, this was Join Gold
    Ultima_JimmyLock( U3_Door_Widget, U3_Keys_PropertyID );
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
      Ultima_Look();
    }
  }
  else if( rumKeypress.KeyM() == eKey )
  {
    // In the original game, this was Modify Order
    if( i_ciKeyboard.AltPressed() )
    {
      local ciPlayer = ::rumGetMainPlayer();
      if( !( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() ) )
      {
        g_ciUI.m_eOverlayGraphicID = U3_Map_GraphicID;
        g_ciUI.m_ciOverlayGraphicOffset = rumPoint( 0, 0 );
        g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnOverlayGraphicDone );
      }
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

    Ultima_NipPotion( U3_Potions_PropertyID, bOnSelf );
  }
  else if( rumKeypress.KeyO() == eKey )
  {
    // In the original game, this was only the Other Command
    if( i_ciKeyboard.AltPressed() )
    {
      U3_OtherCommand();
    }
    else
    {
      Ultima_Open( U3_Door_Widget );
    }
  }
  else if( rumKeypress.KeyP() == eKey )
  {
    Ultima_Peer( U3_Gems_PropertyID );
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
    U3_UseItem();
  }
  else if( rumKeypress.KeyV() == eKey )
  {
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
      U3_Stat_Update( U3_StatPage.Main );
    }
    else
    {
      U3_Stat();
    }
  }
  else if( rumKeypress.KeyTab() == eKey )
  {
    Ultima_AdvanceMapLabel();
  }
  else if( rumKeypress.KeySpace() == eKey )
  {
    U3_OtherCommand();
  }
  else if( ( rumKeypress.KeyPadSubtract() == eKey ) || ( rumKeypress.KeyDash() == eKey ) )
  {
    Ultima_Resurrect( ResurrectionType.Void, GameType.Ultima3 );
  }
  else if( ( rumKeypress.KeyPadAdd() == eKey ) || ( rumKeypress.KeyEquals() == eKey ) )
  {
    Ultima_Resurrect( ResurrectionType.Body, GameType.Ultima3 );
  }
}


function U3_Meditate()
{
  ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_meditate_client_StringID ) );

  local ciPlayer = ::rumGetMainPlayer();
  local ciMap = ciPlayer.GetMap();
  if( null == ciMap )
  {
    return;
  }

  local eMapID = ciMap.GetAssetID();
  local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
  if( MapType.Shrine == eMapType )
  {
    local bCanMeditate = false;
    local ciPos = ciPlayer.GetPosition();

    // Is the player next to a shrine forcefield?
    local ciPawnArray = ciMap.GetPawns( ciPos, 1, false /* no LOS check */ );
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.IsVisible() && ( ciPawn.GetAssetID() == U3_Force_Field_WidgetID ) )
      {
        bCanMeditate = true;
        break;
      }
    }

    if( bCanMeditate )
    {
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold >= Player_Meditate_Broadcast.s_iDonation )
      {
        local strDesc = format( ::rumGetString( msg_shrine_offer_client_StringID ),
                                Player_Meditate_Broadcast.s_iDonation );
        ShowString( strDesc, g_strColorTagArray.Yellow );

        local ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID );
        ::rumSendBroadcast( ciBroadcast );

        BlockInput( ciPlayer.GetActionDelay( ActionDelay.Long ) );
      }
      else
      {
        // Can't pay
        ShowString( ::rumGetString( msg_hmmm_no_effect_client_StringID ), g_strColorTagArray.Red );
      }
    }
    else
    {
      // Not inside of the shrine
      ShowString( ::rumGetString( msg_hmmm_no_effect_client_StringID ), g_strColorTagArray.Red );
    }
  }
  else if( U3_Towne_Yew_MapID == eMapID )
  {
    // Is the player within the Circle of Light?
    local ciPos = ciPlayer.GetPosition();
    if( ciPos.x >= 45 && ciPos.x <= 51 && ciPos.y >= 45 && ciPos.y <= 51 )
    {
      // Yes
      local ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID );
      ::rumSendBroadcast( ciBroadcast );

      BlockInput( ciPlayer.GetActionDelay( ActionDelay.Long ) );
    }
    else
    {
      // No
      ShowString( ::rumGetString( msg_no_effect_client_StringID ), g_strColorTagArray.Red );
    }
  }
  else
  {
    // Not on a shrine map
    ShowString( ::rumGetString( msg_no_effect_client_StringID ), g_strColorTagArray.Red );
  }
}


function U3_OtherCommand()
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

  local strDesc = ::rumGetString( command_other_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.SetEntry( U3_OtherCommandType.Search, ::rumGetString( command_search_client_StringID ) );

  local bCanBribe = false;
  local bCanInsert = false;
  local bCanMeditate = false;
  local bHasCards = ciPlayer.GetProperty( U3_Cards_PropertyID, 0 ) > 0;
  local bHasGold = ciPlayer.GetProperty( U3_Gold_PropertyID, 0 ) > 0;
  local bInShrine = ( ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid ) == MapType.Shrine );

  if( ciMap.GetAssetID() == U3_Towne_Yew_MapID )
  {
    local ciPos = ciPlayer.GetPosition();
    if( ciPos.x >= 45 && ciPos.x <= 51 && ciPos.y >= 45 && ciPos.y <= 51 )
    {
      bCanMeditate = true;
    }
  }

  // Build the list of available other commands based on hint widgets or proximity checks
  local ciPawnArray = ciMap.GetPawns( ciPlayer.GetPosition(), 1, false /* no LOS check */ );
  foreach( ciPawn in ciPawnArray )
  {
    if( ciPawn.IsVisible() )
    {
      local eAssetID = ciPawn.GetAssetID();
      if( ( U3_Exodus_WidgetID == eAssetID ) && bHasCards )
      {
        bCanInsert = true;
      }
      else if( ( U3_Guard_CreatureID == eAssetID ) && bHasGold )
      {
        bCanBribe = true;
      }
      else if( ( U3_Force_Field_WidgetID == eAssetID ) && bInShrine )
      {
        bCanMeditate = true;
      }
    }
  }

  if( bCanBribe )
  {
    g_ciUI.m_ciGameListView.SetEntry( U3_OtherCommandType.Bribe, ::rumGetString( command_bribe_client_StringID ) );
  }

  if( bCanMeditate )
  {
    g_ciUI.m_ciGameListView.SetEntry( U3_OtherCommandType.Meditate, ::rumGetString( command_meditate_client_StringID ) );
  }

  if( bCanInsert )
  {
    g_ciUI.m_ciGameListView.SetEntry( U3_OtherCommandType.Insert, ::rumGetString( command_insert_client_StringID ) );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U3_OtherCommandSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
}


function U3_OtherCommandSelected()
{
  local eCommandType = g_ciUI.m_ciGameListView.GetSelectedKey();

  Ultima_ListSelectionEnd();

  switch( eCommandType )
  {
    case U3_OtherCommandType.Search:   Ultima_Search(); break;
    case U3_OtherCommandType.Bribe:    U3_Bribe();      break;
    case U3_OtherCommandType.Meditate: U3_Meditate();   break;
    case U3_OtherCommandType.Insert:   U3_Insert();     break;
    default:
      ShowString( ::rumGetString( msg_no_effect_client_StringID ), g_strColorTagArray.Red );
      break;
  }
}


function U3_PlayerTransaction( i_ciTarget )
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

    g_ciUI.m_ciGameListView.m_funcAccept = U3_PlayerTransactionTypeSelected;
    g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();

    g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
    g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciTarget.GetID();
  }
}


function U3_PlayerTransactionGive( i_ciTarget )
{
  local ciBroadcast = ::rumCreate( Player_Give_BroadcastID, TradeBroadcastType.GiveNotify, i_ciTarget.GetID() );
  ::rumSendBroadcast( ciBroadcast );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  for( local i = 0; i < U3_ClientTradeType.NumTradeTypes; ++i )
  {
    local strEntry = format( "u3_give%d_client_StringID", i );
    g_ciUI.m_ciGameListView.SetEntry( i, ::rumGetStringByName( strEntry ) );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U3_PlayerTransactionGiveTypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  ciBroadcast = ::rumCreate( Player_Give_BroadcastID, TradeBroadcastType.Give, i_ciTarget.GetID() );
  ::rumSendBroadcast( ciBroadcast );

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = ciBroadcast;
}


function U3_PlayerTransactionGiveEquipment( i_ciBroadcast )
{
  i_ciBroadcast.SetItemSubtype( U3_ClientTradeType.Equipment );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  for( local i = 0; i < g_eU3EquipmentPropertyArray.len(); ++i )
  {
    local ciAsset = ::rumGetPropertyAsset( g_eU3EquipmentPropertyArray[i] );
    local strName = ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" );
    g_ciUI.m_ciGameListView.SetEntry( i, strName );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U3_PlayerTransactionGiveSubtypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciBroadcast;

  U3_Stat_Update( U3_StatPage.Equipment );
}


function U3_PlayerTransactionGiveSubtypeSelected()
{
  local ciBroadcast = g_ciUI.m_ciGameInputTextBox.m_vPayload;
  local iIndex = g_ciUI.m_ciGameListView.GetSelectedIndex();

  Ultima_ListSelectionEnd();

  // Reset the subtype to a class based on the current subtype enum
  local ePropertyID = rumInvalidAssetID;
  switch( ciBroadcast.GetItemSubtype() )
  {
    case U3_ClientTradeType.Equipment:
      ePropertyID = g_eU3EquipmentPropertyArray[iIndex];
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


function U3_PlayerTransactionGiveTypeSelected()
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
    case U3_ClientTradeType.Food:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      ciBroadcast.SetItemSubtype( U3_Food_PropertyID );
      ShowString( ::rumGetString( talk_how_much_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case U3_ClientTradeType.Gold:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      ciBroadcast.SetItemSubtype( U3_Gold_PropertyID );
      ShowString( ::rumGetString( talk_how_much_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case U3_ClientTradeType.Equipment:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      U3_PlayerTransactionGiveEquipment( ciBroadcast );
      break;

    case U3_ClientTradeType.Weapon:
      ciBroadcast.SetItemType( SharedTradeType.Inventory );
      Ultima_PlayerTransactionGiveWeapon( GameType.Ultima3, ciBroadcast );
      break;

    case U3_ClientTradeType.Armour:
      ciBroadcast.SetItemType( SharedTradeType.Inventory );
      Ultima_PlayerTransactionGiveArmour( GameType.Ultima3, ciBroadcast );
      break;
  }
}


function U3_PlayerTransactionTypeSelected()
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
      U3_PlayerTransactionGive( ciTarget );
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


function U3_RenderGame()
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
  if( eMapType != MapType.Dungeon )
  {
    local ciMoonPhase = ::rumGetGraphic( Moon_Phases_GraphicID );
    ciMoonPhase.DrawAnimation( g_ciUI.m_ciMoonDisplayPos, 0, g_ciCUO.m_eTrammelPhase );
    ciMoonPhase.DrawAnimation( rumPoint( g_ciUI.m_ciMoonDisplayPos.x + g_ciUI.s_iBorderPixelWidth,
                                         g_ciUI.m_ciMoonDisplayPos.y ),
                               0, g_ciCUO.m_eFeluccaPhase );
  }

  if( ( GameMode.Game == g_ciCUO.m_eCurrentGameMode ) && !RenderGameOverride() )
  {
    local ciPos = ciPlayer.GetPosition();
    local iIntensity = ciPlayer.GetScreenShakeIntensity().tointeger();

    // Determine pixel offset coordinate to where the map is drawn on screen
    local iOffsetX = g_ciUI.m_ciMapRect.p1.x;
    local iOffsetY = g_ciUI.m_ciMapRect.p1.y;
    local iRadius = g_ciCUO.s_iLOSRadius;

    if( g_ciCUO.m_bPeering )
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

    RenderNamesAndVitals( ciPlayer, U3_Hitpoints_PropertyID, U3_Mana_PropertyID, g_ciCUO.m_bPeering, iRadius );
    RenderOffscreenMemberIcons( ciPlayer, U3_Graphic_ID_PropertyID, iRadius );
    RenderAttacks( ciPos, g_ciCUO.m_bPeering );
    RenderEffects( ciPlayer );
  }

  if( ciPlayer.IsDead() )
  {
    // All the rest of the rendering applies only to living players
    return;
  }

  local ciSerpentGraphic = ::rumGetGraphic( U3_Serpent_UI_GraphicID );
  local uiCardFlags = ciPlayer.GetProperty( U3_Cards_PropertyID, 0 );
  local uiMarkFlags = ciPlayer.GetProperty( U3_Marks_PropertyID, 0 );
  if( ::rumBitAllOn( uiCardFlags, 0xf ) && ::rumBitAllOn( uiMarkFlags, 0xf ) )
  {
    // Draw the entire serpent graphic
    ciSerpentGraphic.Draw( g_ciUI.m_ciAnkhPos );
  }
  else if( uiCardFlags > 0 || uiMarkFlags > 0 )
  {
    local iAnkhY = g_ciUI.m_ciAnkhPos.y;

    for( local i = 0; i < U3_MarkType.NumMarks; ++i )
    {
      if( ::rumBitOn( uiMarkFlags, i ) )
      {
        // Draw only the portion of the ankh for each attained virtue
        ciSerpentGraphic.DrawAnimation( rumPoint( g_ciUI.m_ciAnkhPos.x, iAnkhY ), 0, i );
      }

      iAnkhY += 2;
    }

    for( local i = 0; i < U3_CardType.NumCards; ++i )
    {
      if( ::rumBitOn( uiCardFlags, i ) )
      {
        // Draw only the portion of the ankh for each attained virtue
        ciSerpentGraphic.DrawAnimation( rumPoint( g_ciUI.m_ciAnkhPos.x, iAnkhY ), 0, i + U3_MarkType.NumMarks );
      }

      iAnkhY += 2;
    }
  }
}


function U3_RenderStatAttributes()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_attributes_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05|0.5" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local ciPlayer = ::rumGetMainPlayer();
  local iOffset = 0;

  foreach( ePropertyID in g_eU3StatPropertyArray )
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


function U3_RenderStatCards()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_cards_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local ciPlayer = ::rumGetMainPlayer();
  local iFlags = ciPlayer.GetProperty( U3_Cards_PropertyID, 0 );
  if( iFlags )
  {
    for( local eCard = 0; eCard < U3_CardType.NumCards; ++eCard )
    {
      if( ::rumBitOn( iFlags, eCard ) )
      {
        local strEntry = ::rumGetString( u3_card_client_StringID ) + " " +
                         ::rumGetString( g_eU3CardStringArray[eCard] );
        g_ciUI.m_ciStatListView.SetEntry( eCard + 1, strEntry );
      }
    }
  }
}


function U3_RenderStatMain()
{
  local ciPlayer = ::rumGetMainPlayer();
  g_ciUI.m_ciStatLabel.SetText( ciPlayer.GetPlayerName() );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05|0.5" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local iHitpoints = ciPlayer.GetProperty( U3_Hitpoints_PropertyID, 0 );
  local iMaxHitpoints = ciPlayer.GetMaxHitpoints();

  local iExp = ciPlayer.GetProperty( U3_Experience_PropertyID, 0 );
  local iLevel = ciPlayer.GetProperty( U3_Level_PropertyID, 1 );

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
    local iMana = ciPlayer.GetProperty( U3_Mana_PropertyID, 0 );

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

  local ePlayerClassID = ciPlayer.GetProperty( U3_PlayerClass_PropertyID, U3_Fighter_Class_CustomID );
  local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
  local strClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );

  strEntry = ::rumGetString( U4_PlayerClass_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + strClass );

  strEntry = ::rumGetString( U4_Level_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iLevel );

  strEntry = ::rumGetString( U4_Experience_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iExp );
}


function U3_RenderStatMarks()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_marks_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local ciPlayer = ::rumGetMainPlayer();
  local iFlags = ciPlayer.GetProperty( U3_Marks_PropertyID, 0 );
  if( iFlags )
  {
    for( local eMark = 0; eMark < U3_MarkType.NumMarks; ++eMark )
    {
      if( ::rumBitOn( iFlags, eMark ) )
      {
        local strEntry = ::rumGetString( g_eU3MarkStringArray[eMark] );
        g_ciUI.m_ciStatListView.SetEntry( eMark + 1, strEntry );
      }
    }
  }
}


function U3_Stat()
{
  local ciPlayer = ::rumGetMainPlayer();

  local strDesc = ::rumGetString( command_zstats_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  // Bookmark the current page
  local eCurrentPage = g_ciUI.m_eCurrentStatPage;

  ++g_ciUI.m_eCurrentStatPage;

  if( ( U3_StatPage.Party == g_ciUI.m_eCurrentStatPage ) && ( null == g_ciCUO.m_uiPartyIDTable ) )
  {
    // Don't show the party page if the player is not part of a party
    ++g_ciUI.m_eCurrentStatPage;
  }

  if( U3_StatPage.Marks == g_ciUI.m_eCurrentStatPage )
  {
    // Don't show the Marks page if the player doesn't have any
    local iMarks = ciPlayer.GetProperty( U3_Marks_PropertyID, 0 );
    if( 0 == iMarks )
    {
      ++g_ciUI.m_eCurrentStatPage;
    }
  }

  if( U3_StatPage.Cards == g_ciUI.m_eCurrentStatPage )
  {
    // Don't show the Cards page if the player doesn't have any
    local iCards = ciPlayer.GetProperty( U3_Cards_PropertyID, 0 );
    if( 0 == iCards )
    {
      ++g_ciUI.m_eCurrentStatPage;
    }
  }

  if( g_ciUI.m_eCurrentStatPage >= U3_StatPage.NumPages )
  {
    g_ciUI.m_eCurrentStatPage = U3_StatPage.Main;
  }
  else if( ciPlayer.IsDead() && g_ciUI.m_eCurrentStatPage >= U3_StatPage.Party )
  {
    g_ciUI.m_eCurrentStatPage = U3_StatPage.Main;
  }

  if( eCurrentPage != g_ciUI.m_eCurrentStatPage )
  {
    U3_Stat_Update( g_ciUI.m_eCurrentStatPage );
  }
}


function U3_Stat_Update( i_eStatPage )
{
  g_ciUI.m_eCurrentStatPage = i_eStatPage;

  switch( i_eStatPage )
  {
    case U3_StatPage.Main:       U3_RenderStatMain(); break;
    case U3_StatPage.Party:      RenderStatParty( U3_Graphic_ID_PropertyID,
                                                  U4_Adventurer_GraphicID,
                                                  U3_PlayerClass_PropertyID,
                                                  U3_Fighter_Class_CustomID ); break;
    case U3_StatPage.Marks:      U3_RenderStatMarks(); break;
    case U3_StatPage.Cards:      U3_RenderStatCards(); break;
    case U3_StatPage.Attributes: U3_RenderStatAttributes(); break;
    case U3_StatPage.Weapons:    RenderStatWeapons( GameType.Ultima3 ); break;
    case U3_StatPage.Armour:     RenderStatArmour( GameType.Ultima3 ); break;
    case U3_StatPage.Equipment:  RenderStatEquipment( g_eU3EquipmentPropertyArray ); break;
    default:                     U3_RenderStatMain(); break;
  }
}


function U3_UseItem()
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

  if( ciPlayer.GetProperty( U3_Horse_PropertyID, rumInvalidAssetID ) != rumInvalidAssetID &&
      ( ciPlayer.GetTransportID() == rumInvalidGameID ) )
  {
    local iLastDismountTime = ciPlayer.GetProperty( U3_Horse_Dismount_Time_PropertyID, 0 );
    if( ::rumGetSecondsSinceEpoch() - iLastDismountTime > Transport_Widget.s_fIdleInterval )
    {
      local strEntry = ::rumGetString( U3_Horse_Creature_client_StringID );
      g_ciUI.m_ciGameListView.SetEntry( U3_Horse_PropertyID, strEntry );
    }
  }

  local eTransportWidgetID = ciPlayer.GetProperty( U3_Transport_Widget_PropertyID, rumInvalidAssetID );
  if( eTransportWidgetID != rumInvalidAssetID )
  {
    local ciMap = ciPlayer.GetMap();

    local eMapID = ciPlayer.GetProperty( U3_Transport_Map_PropertyID, rumInvalidAssetID );
    if( eMapID != rumInvalidAssetID && ( ciMap.GetAssetID() == eMapID ) )
    {
      g_ciUI.m_ciGameListView.SetEntry( U3_Transport_Widget_PropertyID, "Summon Transport" );
    }
  }

  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.ShowPrompt( false );

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U3_UseItemCallback;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();
}


function U3_UseItemCallback()
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
      g_ciUI.m_eOverlayGraphicID = U3_Map_GraphicID;
      g_ciUI.m_ciOverlayGraphicOffset = rumPoint( 0, 0 );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnOverlayGraphicDone );
    }
  }
  else
  {
    if( U3_Horse_PropertyID == ePropertyID )
    {
      // Is the map compatible with a horse?
      local ciMap = ciPlayer.GetMap();
      local eMapAssetID = ciMap.GetAssetID();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( ( MapType.Dungeon == eMapType )         ||
          ( MapType.Shrine  == eMapType )         ||
          ( U3_Castle_Fire_MapID == eMapAssetID ) ||
          ( U3_Castle_Fire_2_MapID == eMapAssetID ) )
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
