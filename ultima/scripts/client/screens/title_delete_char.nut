class DeleteCharacterTitleStage
{
  m_ciSettingsTable = { bNeedsRenderInit = true };


  function BeginStage()
  {
    g_ciUI.m_ciInfoLabel.SetActive( true );
    g_ciUI.m_ciInfoLabel.SetText( "" );

    // Explain that the character's name must be retyped in order to delete the character
    g_ciUI.m_ciTitleTextView.SetActive( true );
    local strText = ::rumGetString( chargen_delete_confirm_client_StringID );
    strText = format( strText, g_ciCUO.m_strCharacterDeleteName );
    g_ciUI.m_ciTitleTextView.PushText( strText );

    g_ciUI.m_ciTextBoxArray[0].Focus();
    g_ciUI.m_ciTextBoxArray[0].SetActive( true );
    g_ciUI.m_ciTextBoxArray[0].SetText( "" );
    g_ciUI.m_ciTextBoxArray[0].ObscureText( false );
    g_ciUI.m_ciTextBoxArray[0].SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
    g_ciUI.m_ciTextBoxArray[0].ShowPrompt( true );
    g_ciUI.m_ciTextBoxArray[0].ShowCursor( true );
    g_ciUI.m_ciTextBoxArray[0].m_funcAccept = OnTextBoxAccepted.bindenv( DeleteCharacterTitleStage );
    g_ciUI.m_ciTextBoxArray[0].m_funcCancel = OnTextBoxCanceled.bindenv( DeleteCharacterTitleStage );
  }


  function EndStage()
  {
    m_ciSettingsTable.bNeedsRenderInit = true;

    g_ciUI.m_ciInfoLabel.SetActive( false );
    g_ciUI.m_ciInfoLabel.SetText( "" );
    g_ciUI.m_ciTextBoxArray[0].SetActive( false );
    g_ciUI.m_ciTextBoxArray[0].ClearHandlers();
    g_ciUI.m_ciTitleTextView.SetActive( false );
    g_ciUI.m_ciTitleTextView.Clear();

    InitTitleStage( TitleStages.MainMenu );
  }


  function OnTextBoxAccepted()
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );

    local strCharacterName = g_ciUI.m_ciTextBoxArray[0].GetText();
    if( strCharacterName == g_ciCUO.m_strCharacterDeleteName )
    {
      ::rumDeleteCharacter( strCharacterName );
      EndStage();
    }
    else
    {
      g_ciUI.m_ciInfoLabel.SetText( ::rumGetString( chargen_delete_mismatch_client_StringID ) );
    }
  }


  function OnTextBoxCanceled()
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );
    EndStage();
  }


  function Render( i_iOffsetY )
  {
    if( !m_ciSettingsTable.bNeedsRenderInit )
    {
      return;
    }

    local iScreenWidth = ::rumGetScreenWidth();
    local iScreenCenterX = iScreenWidth / 2;

    i_iOffsetY += g_ciUI.s_iDefaultLabelHeight * 2;

    // Explain that the character's name must be retyped in order to delete the character
    g_ciUI.m_ciTitleTextView.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciTitleTextView.GetWidth() / 2 ),
                                               i_iOffsetY ) );
    i_iOffsetY += g_ciUI.m_ciTitleTextView.GetHeight() + g_ciUI.s_iDefaultLabelHeight;

    // Text box for input
    g_ciUI.m_ciTextBoxArray[0].SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciTextBoxArray[0].GetWidth() / 2 ),
                                                 i_iOffsetY ) );
    i_iOffsetY += g_ciUI.m_ciTextBoxArray[0].GetHeight() + ( g_ciUI.s_iDefaultLabelHeight * 2 );

    g_ciUI.m_ciInfoLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciInfoLabel.GetWidth() / 2 ), i_iOffsetY ) );

    m_ciSettingsTable.bNeedsRenderInit = false;
  }
}