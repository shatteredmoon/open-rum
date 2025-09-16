enum UltimaInfo
{
  Game,
  PC_File,
  PC_FileHash
}

g_aUltimaInfoArray <-
[
  ["Akalabeth", "", ""],
  ["Ultima I", "ULTIMA.EXE", "ed19976a19a2794b8e3ac528e4f622a5"],
  ["Ultima II", "ULTIMAII.EXE", "d6727dacbe7ef7de20498a32b8f171ea"],
  ["Ultima III", "ULTIMA.COM", "7bea6bf20b7079b30f8ca5336b3805b2"],
  ["Ultima IV", "AVATAR.EXE", "4c23370b7a769bfa4d0667c447d2507f"]
]


function CalculateListViewSize( i_ciListView, i_strFont, i_uiMaxRows )
{
  if( i_ciListView instanceof ListView )
  {
    local uiTotalRows = i_ciListView.GetNumEntries();
    uiTotalRows = max( uiTotalRows, 1 );

    local uiFontHeight = rumGetTextHeight( i_strFont );
    if( uiFontHeight > 0 )
    {
      local uiTotalViewHeight = uiTotalRows * uiFontHeight;
      local uiRestrictedViewHeight = i_uiMaxRows * uiFontHeight;

      if( uiTotalViewHeight > uiRestrictedViewHeight )
      {
        i_ciListView.ShowScrollbar( true );
        i_ciListView.SetHeight( uiRestrictedViewHeight );
      }
      else
      {
        i_ciListView.ShowScrollbar( false );
        i_ciListView.SetHeight( uiTotalViewHeight );
      }
    }
    else
    {
      i_ciListView.SetHeight( g_ciUI.s_iDefaultLabelHeight );
    }
  }
}


function GetDirectionFromInput( i_eKey )
{
  if( ( rumKeypress.KeyLeft() == i_eKey ) || ( rumKeypress.KeyPad4() == i_eKey ) )
  {
    return Direction.West;
  }
  else if( ( rumKeypress.KeyRight() == i_eKey ) || ( rumKeypress.KeyPad6() == i_eKey ) )
  {
    return Direction.East;
  }
  else if( ( rumKeypress.KeyUp() == i_eKey ) || ( rumKeypress.KeyPad8() == i_eKey ) )
  {
    return Direction.North;
  }
  else if( ( rumKeypress.KeyDown() == i_eKey ) || ( rumKeypress.KeyPad2() == i_eKey ) )
  {
    return Direction.South;
  }
  else if( rumKeypress.KeyPad7() == i_eKey )
  {
    return Direction.Northwest;
  }
  else if( rumKeypress.KeyPad9() == i_eKey )
  {
    return Direction.Northeast;
  }
  else if( rumKeypress.KeyPad3() == i_eKey )
  {
    return Direction.Southeast;
  }
  else if( rumKeypress.KeyPad1() == i_eKey )
  {
    return Direction.Southwest;
  }

  return Direction.None;
}


function GetDirectionString( i_eDir )
{
  return ::rumGetString( g_eDirectionStringArray[i_eDir] );
}


function GetMapPosFromMousePos( i_ciMapPos, i_ciMousePos )
{
  local ciOffsetVector = GetVectorFromMousePos( i_ciMousePos );
  return rumPos( i_ciMapPos.x + ciOffsetVector.x, i_ciMapPos.y + ciOffsetVector.y );
}


function GetSelectedPath()
{
  local ciListView = g_ciUI.m_ciExplorerFileListView;

  local bDriveListFocused = g_ciUI.m_ciExplorerDriveListView.HasInputFocus();
  if( bDriveListFocused )
  {
    ciListView = g_ciUI.m_ciExplorerDriveListView;
  }

  local strDelimiter = ciListView.GetDelimiter();
  local strSelection = ciListView.GetCurrentEntry();
  if( strSelection )
  {
    // Split on the delimiter
    local strTokenArray = split( strSelection, strDelimiter );
    if( strTokenArray.len() > 0 )
    {
      if( bDriveListFocused )
      {
        strSelection = strTokenArray[1];
      }
      else
      {
        local strSeparator = ::rumGetPathSeparator();
        strSelection = g_ciCUO.m_strCurrentDirectory + strSeparator + strTokenArray[1];
      }
    }
  }

  return ::rumGetCleanFilePath( strSelection );
}


function GetVectorFromMousePos( i_ciMousePos )
{
  // The vector returned is relative to the player's position
  local ciPlayer = ::rumGetMainPlayer();
  if( null == ciPlayer )
  {
    return rumVector();
  }

  local ciMap = ciPlayer.GetMap();
  if( null == ciMap )
  {
    return rumVector();
  }

  local ciPlayerPos = ciPlayer.GetPosition();
  local ciMouseOffset = rumVector();

  if( g_ciCUO.m_bPeering || ciMap.GetProperty( Map_Always_Peer_PropertyID, false ) )
  {
    ciMouseOffset.x = ( i_ciMousePos.x - g_ciUI.s_iBorderPixelWidth ) /
                      g_ciUI.s_iTileHalfPixelWidth - g_ciUI.s_ciCenterTilePeeringPos.x;
    ciMouseOffset.y = ( i_ciMousePos.y - g_ciUI.s_iBorderPixelWidth ) /
                      g_ciUI.s_iTileHalfPixelWidth - g_ciUI.s_ciCenterTilePeeringPos.y;
  }
  else
  {
    ciMouseOffset.x = ( i_ciMousePos.x - g_ciUI.s_iBorderPixelWidth ) /
                      g_ciUI.s_iTilePixelWidth - g_ciUI.s_ciCenterTilePos.x;
    ciMouseOffset.y = ( i_ciMousePos.y - g_ciUI.s_iBorderPixelWidth ) /
                      g_ciUI.s_iTilePixelWidth - g_ciUI.s_ciCenterTilePos.y;
  }

  return ciMouseOffset;
}


function OnDriveListMenuAccepted()
{
  g_ciUI.m_ciInfoLabel.SetText( "" );
  g_ciUI.m_ciGameInputTextBox.SetText( "" );

  UpdateDriveSelection();

  g_ciUI.m_ciExplorerFileListView.Focus();
}


function OnDriveListMenuCanceled()
{
  g_ciUI.m_ciInfoLabel.SetText( "" );
  g_ciUI.m_ciGameInputTextBox.SetText( "" );

  g_ciUI.m_ciExplorerDriveListView.ClearHandlers();
}


function OnDriveListMenuIndexChanged( i_iIndex )
{
  local strPath = GetSelectedPath();
  g_ciUI.m_ciInfoLabel.SetText( strPath );
  g_ciUI.m_ciGameInputTextBox.SetText( strPath );
}


function OnDriveListMenuKeyPressed( i_ciKeyInput )
{
  g_ciUI.m_ciInfoLabel.SetText( "" );

  local eKey = i_ciKeyInput.GetKey();

  if( ( rumKeypress.KeyRight() == eKey ) || ( rumKeypress.KeyTab() == eKey ) )
  {
    g_ciUI.m_ciExplorerFileListView.Focus();
  }
}


function OnDriveListMenuMouseMoved( i_ciPoint )
{
  local strPath = GetSelectedPath();
  g_ciUI.m_ciInfoLabel.SetText( strPath );
  g_ciUI.m_ciGameInputTextBox.SetText( strPath );
}


function OnFileListMenuAccepted()
{
  g_ciUI.m_ciInfoLabel.SetText( "" );
  g_ciUI.m_ciGameInputTextBox.SetText( "" );

  UpdateFileSelection();
}


function OnFileListMenuCanceled()
{
  g_ciUI.m_ciInfoLabel.SetText( "" );
  g_ciUI.m_ciGameInputTextBox.SetText( "" );

  g_ciUI.m_ciExplorerFileListView.ClearHandlers();
}


function OnFileListMenuIndexChanged( i_iIndex )
{
  local strPath = GetSelectedPath();
  g_ciUI.m_ciInfoLabel.SetText( strPath );
  g_ciUI.m_ciGameInputTextBox.SetText( strPath );
}


function OnFileListMenuKeyPressed( i_ciKeyInput )
{
  g_ciUI.m_ciInfoLabel.SetText( "" );

  local eKey = i_ciKeyInput.GetKey();

  if( ( rumKeypress.KeyLeft() == eKey ) || ( rumKeypress.KeyTab() == eKey ) )
  {
    g_ciUI.m_ciExplorerDriveListView.Focus();
  }
  else if( rumKeypress.KeySpace() == eKey )
  {
    m_ciSettingsTable.bNeedsRenderInit = true;
  }
}


function OnFileListMenuMouseMoved( i_ciPoint )
{
  local strPath = GetSelectedPath();
  g_ciUI.m_ciInfoLabel.SetText( strPath );
  g_ciUI.m_ciGameInputTextBox.SetText( strPath );
}


// Actual work is done here so that other function can reuse printing
function ShowString( i_strText, ... )
{
  // Print the result to the client's game messages
  g_ciUI.m_ciGameTextView.PushText( vargv.len() > 0 ? vargv[0] + i_strText : i_strText  );
}


function UpdateDriveListView()
{
  g_ciUI.m_ciExplorerDriveListView.Clear();
  g_ciUI.m_ciExplorerDriveListView.m_funcAccept = OnDriveListMenuAccepted;
  g_ciUI.m_ciExplorerDriveListView.m_funcCancel = OnDriveListMenuCanceled;
  g_ciUI.m_ciExplorerDriveListView.m_funcKeyPress = OnDriveListMenuKeyPressed;
  g_ciUI.m_ciExplorerDriveListView.m_funcIndexChanged = OnDriveListMenuIndexChanged;
  g_ciUI.m_ciExplorerDriveListView.m_funcMouseMoved = OnDriveListMenuMouseMoved;

  local ciGraphic = ::rumGetGraphic( Volume_GraphicID );

  local ciDriveArray = ::rumGetDriveArray();
  g_ciUI.m_ciExplorerDriveListView.SetMaxEntries( ciDriveArray.len() );
  foreach( iIndex, strDrive in ciDriveArray )
  {
    local strEntry = format( "<G#%s:w16:h16>|%s", ciGraphic.GetName(), strDrive );
    g_ciUI.m_ciExplorerDriveListView.SetEntry( iIndex, strEntry );
  }
}


function UpdateDriveSelection()
{
  local strSelection = GetSelectedPath();
  if( ::rumIsFolder( strSelection ) )
  {
    g_ciCUO.m_strCurrentDirectory = ::rumGetCleanFilePath( strSelection );
    UpdateFileListView();
  }
}


function UpdateFileListView()
{
  local bFound = false;

  g_ciUI.m_ciExplorerFileListView.Clear();
  g_ciUI.m_ciExplorerFileListView.m_funcAccept = OnFileListMenuAccepted;
  g_ciUI.m_ciExplorerFileListView.m_funcCancel = OnFileListMenuCanceled;
  g_ciUI.m_ciExplorerFileListView.m_funcKeyPress = OnFileListMenuKeyPressed;
  g_ciUI.m_ciExplorerFileListView.m_funcIndexChanged = OnFileListMenuIndexChanged;
  g_ciUI.m_ciExplorerFileListView.m_funcMouseMoved = OnFileListMenuMouseMoved;

  local ciFolderGraphic = ::rumGetGraphic( Folder_GraphicID );
  local ciFileGraphic = ::rumGetGraphic( File_GraphicID );

  local iLastIndex = 0;

  if( ::rumHasParentPath( g_ciCUO.m_strCurrentDirectory ) )
  {
    local strEntry = format( "<G#%s:w16:h16>|%s", ciFolderGraphic.GetName(), ".." );
    g_ciUI.m_ciExplorerFileListView.SetEntry( 0, strEntry );
    iLastIndex = 1;
  }

  local ciFolderArray = ::rumGetFoldersFromPath( g_ciCUO.m_strCurrentDirectory );
  g_ciUI.m_ciExplorerFileListView.SetMaxEntries( ciFolderArray.len() + 1 );
  foreach( iIndex, strFolder in ciFolderArray )
  {
    iLastIndex += iIndex + 1;
    local strEntry = format( "<G#%s:w16:h16>|%s", ciFolderGraphic.GetName(), strFolder );
    g_ciUI.m_ciExplorerFileListView.SetEntry( iLastIndex, strEntry );
  }

  local strSeparator = ::rumGetPathSeparator();

  local ciFileArray = ::rumGetFilesFromPath( g_ciCUO.m_strCurrentDirectory );
  g_ciUI.m_ciExplorerFileListView.SetMaxEntries( ciFileArray.len() + ciFolderArray.len() + 1 );
  foreach( iIndex, strFile in ciFileArray )
  {
    // Check for possible matches to the expected file while we're iterating
    if( strFile == g_aUltimaInfoArray[g_ciCUO.m_eVersion][UltimaInfo.PC_File] )
    {
      local strFilePath = g_ciCUO.m_strCurrentDirectory + strSeparator + strFile;
      if( ::rumVerifyFile( strFilePath, g_aUltimaInfoArray[g_ciCUO.m_eVersion][UltimaInfo.PC_FileHash] ) )
      {
        bFound = true;
        break;
      }
    }

    iLastIndex += iIndex + 1;
    local strEntry = format( "<G#%s:w16:h16>|%s", ciFileGraphic.GetName(), strFile );
    g_ciUI.m_ciExplorerFileListView.SetEntry( iLastIndex, strEntry );
  }

  if( bFound )
  {
    SetUltimaPath( g_ciUI.m_eVerificationVersion, g_ciCUO.m_strCurrentDirectory );
  }
}


function UpdateFileSelection()
{
  local strSelection = GetSelectedPath();
  g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].SetText( strSelection );

  if( ::rumIsFolder( strSelection ) )
  {
    g_ciCUO.m_strCurrentDirectory = ::rumGetCleanFilePath( strSelection );
    UpdateFileListView();
  }
  else
  {
    SetUltimaPath( g_ciUI.m_eVerificationVersion, g_ciCUO.m_strCurrentDirectory );
  }
}


function SetUltimaPath( i_eVersion, i_strPath )
{
  g_ciUI.m_ciExplorerDriveListView.SetActive( false );
  g_ciUI.m_ciExplorerFileListView.SetActive( false );

  local strAdjustedPath = ::rumAppendPathSeparator( i_strPath );
  local strUltima = format( "cuo:ultima%d", i_eVersion );
  g_ciCUO.m_ciGameConfigTable[strUltima] = strAdjustedPath;

  // Save the new path
  ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

  if( GameMode.Game == g_ciCUO.m_eCurrentGameMode )
  {
    g_ciUI.m_ciGameInputTextBox.ResetInputMode( true, strAdjustedPath );
  }
  else if( GameMode.Title == g_ciCUO.m_eCurrentGameMode )
  {
    VerifyUltimaTitleStage.EndStage( TitleStages.VerifyUltimaInstall );
  }
}
