// Received from server when player transacts with a magic/spell merchant
class Merchant_Magic_Broadcast extends rumBroadcast
{
  var1 = 0; // Magic transaction type
  var2 = 0; // Merchant response
  var3 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 3 == vargv.len() )
    {
      var1 = vargv[0]; // Magic transaction type
      var2 = vargv[1]; // Spell or potion type to buy
      var3 = vargv[2]; // Amount
    }
  }


  function OnRecv()
  {
    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciMerchantRef = this;

    local eTransactionType = var1;

    if( MerchantMagicTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      // Show the relevant magic/spell stat page
      g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;

      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Spells; break;
        case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Spells; break;
      }

      Ultima_Stat_Update();

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTokenID = ciMap.GetProperty( Merchant_Magic_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      local strDesc = format( "%s %s.<b>", ::rumGetString( merchant_magic_greet_client_StringID ), strShopName );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      BuySelect();
    }
    else if( MerchantMagicTransaction.Purchase == eTransactionType )
    {
      local strDesc = var2;
      OfferService( strDesc );
    }
    else if( MerchantMagicTransaction.ServerTerminated == eTransactionType )
    {
      local strReason = var2;
      EndTransaction( false /* do not inform server */, strReason );
    }
  }


  function BuySelect()
  {
    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local eInventoryArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_magic_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice out the spell data
      eInventoryArray = ::rumGetDataTableRow( merchant_magic_DataTableID, iRow ).slice( 3, 9 );
    }
    else
    {
      EndTransaction( true /* inform server */ );
      return;
    }

    local iMaxItems = eInventoryArray.len();
    local iIndex = eInventoryArray.find( rumInvalidAssetID );
    local iNumAvailable = ( null == iIndex ) ? iMaxItems : iIndex;

    // Sort ascending
    eInventoryArray.resize( iNumAvailable );
    eInventoryArray.sort();

    // Build the spell list
    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.SetFormat( "0.05|0.2|0.75" );
    g_ciUI.m_ciGameListView.ShowPrompt( false );

    local fDiscount = ciPlayer.GetDiscountPercent();
    local eSpellArray = null;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1:
        eSpellArray = g_eU1SpellArray;
        g_ciUI.m_eCurrentStatPage = U1_StatPage.Spells;
        break;

      case GameType.Ultima2:
        eSpellArray = g_eU2SpellArray;
        g_ciUI.m_eCurrentStatPage = U2_StatPage.Spells;
        break;
    }

    Ultima_Stat_Update();

    // Visit each spell
    foreach( iIndex, eSpellID in eSpellArray )
    {
      // Is this spell one the merchant sells?
      if( ValueInContainer( eSpellID, eInventoryArray ) )
      {
        local ciSpell = ::rumGetCustomAsset( eSpellID );
        if( ciSpell != null )
        {
          local strSpellDesc = ::rumGetStringByName( ciSpell.GetName() + "_client_StringID" );
          local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iIndex ) );

          local iPrice = ciSpell.GetProperty( Merchant_Price_PropertyID, 0 );
          iPrice = iPrice - ( iPrice * fDiscount ).tointeger();
          iPrice = max( iPrice, 1 );

          g_ciUI.m_ciGameListView.SetEntry( iIndex, strAlpha + "|" + strSpellDesc + "|" + iPrice );
        }
      }
    }

    local iIndex = eSpellArray.len();
    local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iIndex ) );
    local strEntry = ::rumGetString( u4_merchant_healer_potions_client_StringID );
    local iPrice = ::rumGetDataTableRow( merchant_magic_DataTableID, iRow )[2];
    g_ciUI.m_ciGameListView.SetEntry( iIndex, strAlpha + "|" + strEntry + "|" + iPrice );

    local strDesc = ::rumGetString( merchant_magic_prompt_client_StringID );
    ShowString( strDesc, g_strColorTagArray.Cyan );

    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

    g_ciUI.m_ciGameListView.m_funcAccept = BuySelectCallback.bindenv( this );
    g_ciUI.m_ciGameListView.m_funcCancel = PurchaseCanceled.bindenv( this );
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();
  }


  function BuySelectCallback()
  {
    // Push the player's selection to output
    local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
    g_ciUI.m_ciGameInputTextBox.Clear();

    local strArray = split( strDesc, "|" );
    ShowString( strArray[1] + "<b>" );

    local eSpellArray = null;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1: eSpellArray = g_eU1SpellArray; break;
      case GameType.Ultima2: eSpellArray = g_eU2SpellArray; break;
    }

    local eSelection = g_ciUI.m_ciGameListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( eSelection >= eSpellArray.len() )
    {
      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Equipment; break;
        case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Equipment; break;
      }
    }
    else
    {
      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Spells; break;
        case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Spells; break;
      }
    }

    Ultima_Stat_Update();

    // Get how many items the player wants
    strDesc = format( ::rumGetString( merchant_magic_buy_prompt_client_StringID ), strDesc );
    ShowString( strDesc, g_strColorTagArray.Cyan );

    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount,
                                              BuyVerifyCallback.bindenv( this ),
                                              eSelection );
  }


  function BuyVerifyCallback( i_eSelection, i_iAmount )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    if( i_iAmount <= 0 )
    {
      // The player doesn't want any items
      OfferService( merchant_magic_buy_no_client_StringID );
      return;
    }

    local ciPlayer = ::rumGetMainPlayer();

    local eSpellArray = null;
    local fDiscount = ciPlayer.GetDiscountPercent();

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1: eSpellArray = g_eU1SpellArray; break;
      case GameType.Ultima2: eSpellArray = g_eU2SpellArray; break;
    }

    // Push the desired amount to output
    ShowString( format( "%d", i_iAmount ) + "<b>" );

    // Does the player have enough gold to make the purchase?
    local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

    if( i_eSelection >= eSpellArray.len() )
    {
      local ciMap = ciPlayer.GetMap();

      // Determine the data table row
      local eMapArray = ::rumGetDataTableColumn( merchant_magic_DataTableID, 0 );
      local iRow = eMapArray.find( ciMap.GetAssetID() );
      if( null == iRow )
      {
        EndTransaction( true /* inform server */ );
        return;
      }

      local iPrice = ::rumGetDataTableRow( merchant_magic_DataTableID, iRow )[2];
      if( iPlayerGold >= iPrice )
      {
        local ePropertyID = rumInvalidAssetID;

        switch( g_ciCUO.m_eVersion )
        {
          case GameType.Ultima1: ePropertyID = U1_Potions_PropertyID; break;
          case GameType.Ultima2: ePropertyID = U2_Potions_PropertyID; break;
        }

        // Send the buy request to the server
        local ciBroadcast = ::rumCreate( Merchant_Magic_BroadcastID, MerchantMagicTransaction.Purchase, ePropertyID, i_iAmount );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        // Not enough gold!
        OfferService( merchant_magic_cant_pay_client_StringID );
      }
    }
    else
    {
      // Handle spells
      local eSpellID = eSpellArray[i_eSelection];
      local ciSpell = ::rumGetCustomAsset( eSpellID );
      if( ciSpell != null )
      {
        local iPrice = ciSpell.GetProperty( Merchant_Price_PropertyID, 0 ) * i_iAmount;
        iPrice = iPrice - ( iPrice * fDiscount ).tointeger();
        iPrice = max( iPrice, 1 );

        if( iPlayerGold >= iPrice )
        {
          // Send the buy request to the server
          local ciBroadcast = ::rumCreate( Merchant_Magic_BroadcastID, MerchantMagicTransaction.Purchase, eSpellID, i_iAmount );
          ::rumSendBroadcast( ciBroadcast );
        }
        else
        {
          // Not enough gold!
          OfferService( merchant_magic_cant_pay_client_StringID );
        }
      }
      else
      {
        OfferService( merchant_magic_buy_no_client_StringID );
      }
    }
  }


  function ContinueCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      BuySelect();
    }
    else
    {
      EndTransaction( true /* inform server */ );
    }
  }


  function EndTransaction( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local strDesc;

    if( vargv.len() > 1 && vargv[1] )
    {
      strDesc = ::rumGetString( vargv[1] );
    }
    else
    {
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local strDesc = ::rumGetString( merchant_magic_bye_client_StringID );
      ShowString( strDesc, g_strColorTagArray.Cyan );
    }

    local bSendTermination = vargv.len() >= 1 ? vargv[0] : true;
    if( bSendTermination )
    {
      // End transaction on the server
      local ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
      ::rumSendBroadcast( ciBroadcast );
    }

    StopMusic();

    // Show the player's previously selected stat page
    Ultima_Stat_Restore();

    g_ciCUO.m_ciMerchantRef = null;
  }


  function OfferService( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local strDesc;

    if( ( vargv.len() == 1 ) && vargv[0] )
    {
      strDesc = format( "%s<b><b>%s",
                        ::rumGetString( vargv[0] ),
                        ::rumGetString( merchant_magic_continue_client_StringID ) );
    }
    else
    {
      strDesc = ::rumGetString( merchant_magic_continue_client_StringID );
    }

    ShowString( strDesc, g_strColorTagArray.Cyan );

    // See if the player wants anything else
    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = ContinueCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();
  }


  function PurchaseCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    OfferService();
  }


  function TransactionCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    EndTransaction( true /* inform server */ );
  }
}
