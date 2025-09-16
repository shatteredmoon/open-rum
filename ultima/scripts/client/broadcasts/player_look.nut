// Sent from client when player interacts with a fountain, orb, or altar
// Received from server with results
class Player_Look_Broadcast extends rumBroadcast
{
  var1 = 0; // Object ID or asset type
  var2 = 0; // Var
  var3 = 0; // Var
  var4 = 0; // Var


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0];

      if( vargv.len() > 1 )
      {
        var2 = vargv[1];
      }
    }
  }


  function OnRecv()
  {
    if( U1_Pond_WidgetID == var1 )
    {
      switch( var2 )
      {
        case U1_PondResultType.Hitpoints:
          ShowString( format( "%s %s + %d!",
                              ::rumGetString( msg_shazam_client_StringID ),
                              ::rumGetString( Hitpoints_Property_client_StringID ),
                              var3 ),
                      g_strColorTagArray.Green );
          break;

        case U1_PondResultType.Food:
          ShowString( format( "%s %s + %d!",
                              ::rumGetString( msg_shazam_client_StringID ),
                              ::rumGetString( Food_Property_client_StringID ),
                              var3 ),
                      g_strColorTagArray.Green );
          break;

        case U1_PondResultType.Spell:
          local ciAsset = ::rumGetAsset( var3 );
          if( ciAsset != null )
          {
            ShowString( format( "%s %s: %s!",
                                ::rumGetString( command_look_pond_alakazot_client_StringID ),
                                ::rumGetString( command_look_pond_spell_client_StringID ),
                                ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" ) ),
                        g_strColorTagArray.Green );
          }
          break;

        case U1_PondResultType.Weapon:
          local ciAsset = ::rumGetAsset( var3 );
          if( ciAsset != null )
          {
            ShowString( format( "%s %s: %s!",
                                ::rumGetString( msg_shazam_client_StringID ),
                                ::rumGetString( command_look_pond_weapon_client_StringID ),
                                ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" ) ),
                        g_strColorTagArray.Green );
          }
          break;

        case U1_PondResultType.Stat:
          local ciAsset = ::rumGetAsset( var3 );
          local strStat = ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" );
          ShowString( format( "%s %s + %d!", ::rumGetString( msg_shazam_client_StringID ), strStat, 5 ),
                      g_strColorTagArray.Green );
          break;
      }
    }
    else if( U1_Signpost_WidgetID == var1 )
    {
      switch( var2 )
      {
        case U1_SignPostUsageType.Description:
          local eSignpostType = var3;
          local strDesc = ::rumGetStringByName( format( "u1_sign_%d_client_StringID", eSignpostType ) );
          ShowString( format( "<b>%s<b>", strDesc ), g_strColorTagArray.White );
          break;

        case U1_SignPostUsageType.QuestComplete:
          local strDesc = var3;
          ShowString( format( "%s<b>", var3 ), g_strColorTagArray.White );
          break;

        case U1_SignPostUsageType.RewardProperty:
          local ciAsset = ::rumGetPropertyAsset( var3 );
          if( ciAsset != null )
          {
            local iAmount = var4;
            local strDesc = ::rumGetStringByName( format( "%s_Property_client_StringID", ciAsset.GetName() ) );
            ShowString( format( "%s + %d", strDesc, iAmount ), g_strColorTagArray.Green );
          }
          break;

        case U1_SignPostUsageType.RewardWeapon:
          local ciAsset = ::rumGetAsset( var3 );
          if( ciAsset != null )
          {
            ShowString( format( ::rumGetString( u1_king_quest_find_client_StringID ),
                                ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" ) ),
                        g_strColorTagArray.Green );
          }
          break;
      }
    }
    else if( U2_Shield_WidgetID == var1 )
    {
      local eAssetID = var2;
      local ciAsset = ::rumGetAsset( eAssetID );

      if( ( U2_Reflect_Armour_InventoryID == eAssetID ) || ( U2_Power_Armour_InventoryID == eAssetID ) )
      {
        ShowString( format( "%s %s!",
                            ::rumGetString( msg_item_found_client_StringID ),
                            ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" ) ),
                    g_strColorTagArray.Green );
      }
      else
      {
        ShowString( format( "%s %s %s!",
                            ::rumGetString( msg_item_found_client_StringID ),
                            ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" ),
                            ::rumGetString( token_armour_client_StringID ) ),
                    g_strColorTagArray.Green );
      }
    }
    else if( U2_Sword_WidgetID == var1 )
    {
      local ciAsset = ::rumGetAsset( var2 );

      local strWeaponDescription = ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" );

      local strArticle = "";
      local eArticleType = ciAsset.GetProperty( Description_Article_Type_PropertyID,
                                                ArticleType.article_indef_sing_cons );
      if( eArticleType != ArticleType.none )
      {
        strArticle = ::rumGetString( g_eArticleStringArray[eArticleType] );
        strWeaponDescription = strArticle + " " + strWeaponDescription;
      }

      ShowString( format( "%s %s!", ::rumGetString( msg_item_found_client_StringID ), strWeaponDescription ),
                  g_strColorTagArray.Green );
    }
    else if( U2_Signpost_WidgetID == var1 )
    {
      local eSignpostType = var2;
      local strGraphic = "<g#U2_Time_Periods:vcenter:w16:h16:f" + eSignpostType + ">";
      ShowString( format( "<b>%s<b>%s %s<b>",
                          ::rumGetString( msg_sign_desc_client_StringID ),
                          ::rumGetStringByName( format( "u2_sign_%d_client_StringID", eSignpostType ) ),
                          strGraphic ),
                  g_strColorTagArray.White );
    }
    else if( U3_Signpost_WidgetID == var1 )
    {
      ShowString( format( "<b>%s<b>%s<b>", ::rumGetString( msg_sign_desc_client_StringID ), var2 ),
                  g_strColorTagArray.White );
    }
    else if( U4_Orb_WidgetID == var1 )
    {
      local eOrbType = var2;
      local strStatDesc = null;

      switch( eOrbType )
      {
        case OrbType.Strength:     strStatDesc = ::rumGetString( msg_strength_increased_client_StringID );     break;
        case OrbType.Dexterity:    strStatDesc = ::rumGetString( msg_dexterity_increased_client_StringID );    break;
        case OrbType.Intelligence: strStatDesc = ::rumGetString( msg_intelligence_increased_client_StringID ); break;
      }

      if( strStatDesc != null )
      {
        ShowString( format( strStatDesc, 5 ), g_strColorTagArray.Green );
      }
    }
  }
}
