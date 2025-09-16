#ifndef _C_LISTVIEW_H_
#define _C_LISTVIEW_H_

#include <u_structs.h>
#include <controls/c_control.h>
#include <controls/c_scrollbuffer.h>

#include <vector>


class rumClientListView : public rumClientScrollBuffer
{
  // Represents a single item in the list
  struct ListItem
  {
    int64_t m_iKey{ 0 };
    std::string m_strEntry;
    int32_t m_keyShortcut{ -1 };
    bool m_bSelected{ false };
  };

  // Used to save parsed format settings for column spacing
  struct OffsetEntry
  {
    uint32_t m_uiOffset{ 0 };
    AlignType m_eAlignment{ ALIGN_LEFT };
  };

public:

  void CalcHeight()
  {
    SetHeight( GetNumEntries() * GetFontHeight() );
  }

  void Clear() override;

  void DisableMultiSelect();
  void EnableMultiSelect( uint32_t i_uiMin, uint32_t i_uiMax );

  void Focus() override;

  int32_t GetCurrentIndex() const
  {
    return m_iCurrentIndex;
  }

  void SetCurrentIndex( int32_t i_uiIndex );

  int32_t GetIndexAtScreenPosition( const rumPoint& i_rcPoint );

  int64_t GetSelectedKey() const;

  // Returns the screen position of the selected item
  rumPoint GetSelectedPos() const;

  // List views always set their own buffer height
  void ScriptSetBufferPixelHeight( uint32_t i_uiHeight ) override
  {
    return;
  }

  void MoveToIndex( int32_t i_uiIndex );

  void MoveToFirst()
  {
    MoveToIndex( 0 );
  }

  void MoveToLast()
  {
    if( !m_vEntries.empty() )
    {
      MoveToIndex( static_cast<int32_t>( m_vEntries.size() - 1 ) );
    }
  }

  void MoveToNext()
  {
    MoveToIndex( m_iCurrentIndex + 1 );
  }

  void MoveToPrev()
  {
    if( m_iCurrentIndex > 0 )
    {
      MoveToIndex( m_iCurrentIndex - 1 );
    }
  }

  void MovePageUp();
  void MovePageDown();

  ListItem* ListItemFetchIndex( uint32_t i_uiIndex );
  ListItem* ListItemFetchKey( int64_t i_iKey );

  uint32_t GetNumSelectedEntries() const
  {
    return m_uiNumSelectedEntries;
  }

  void HighlightCurrentIndex( bool i_bHighlight )
  {
    m_bHighlightCurrent = i_bHighlight;
  }

  void RemoveEntry( int64_t i_iKey );

  // Selects or deselects all items
  void SelectAll( bool i_bSelected );

  // Deselect all items
  void SelectNone()
  {
    SelectionReset();
  }

  void SelectionReset()
  {
    SelectAll( false );
  }

  // Toggles the current item
  bool SelectCurrent();

  // Toggles item selection by key
  bool SelectItem( int32_t i_iKey );

  // Toggles item selection by shortcut key
  bool SelectItemByShortcut( int32_t i_iKey );

  bool SetEntry( int64_t i_iKey, const std::string& i_strText )
  {
    return SetEntry( i_iKey, i_strText, RUM_INVALID_NATIVEHANDLE );
  }

  bool SetEntry( int64_t i_iKey, const std::string& i_strText, NativeHandle i_eShortcutKey = RUM_INVALID_NATIVEHANDLE );

  std::string GetCurrentEntry();
  std::string GetEntry( int64_t i_iKey );

  std::string ScriptGetCurrentEntry();
  std::string ScriptGetEntry( int64_t i_iKey );

  // A string of characters that will be used to detect separate columns in list entries, example: ",:|"
  std::string GetDelimiter() const
  {
    return m_strDelimiter;
  }

  void SetDelimiter( const std::string& i_strDelimiter )
  {
    m_strDelimiter = i_strDelimiter;
    Invalidate();
  }

  uint32_t GetMaxSelectableEntries() const
  {
    return m_uiMaxSelectableEntries;
  }

  void SetMaxSelectableEntries( uint32_t i_uiMax );

  uint32_t GetMinSelectableEntries() const
  {
    return m_uiMinSelectableEntries;
  }

  void SetMinSelectableEntries( uint32_t i_uiMin )
  {
    m_uiMinSelectableEntries = i_uiMin;
  }

  uint32_t GetNumEntries() const
  {
    return static_cast<uint32_t>( m_vEntries.size() );
  }

  // A string that specifies how to space out list entries into columns
  // using the delimiter. Valid ranges are from [0.0 to 1.0). (ex. Example:
  // "0.1|0.5|0.8" defines 3 columns, starting at 10% from the left of the
  // total available output area. The second column begins at the midpoint
  // and the 3rd column begins at 80% of the way
  // Justification works this way:
  //  0.1 (leading zero = left justification)
  //  1.1 (leading one = center justification)
  //  2.1 (leading two = right justification)
  void SetFormat( const std::string& i_strFormat )
  {
    m_strFormat = i_strFormat;
    Invalidate();
  }

  // Returns an array of each key that's currently selected
  Sqrat::Array ScriptGetSelectedKeys();

  uint32_t GetMaxEntries() const
  {
    return m_uiMaxEntries;
  }
  void SetMaxEntries( uint32_t i_uiMax )
  {
    m_uiMaxEntries = i_uiMax;
  }

  bool SupportsMultiSelect() const
  {
    return m_bMultiSelect;
  }

  // Static interface
  static void ScriptBind();

private:

  int32_t DrawEntry( const int32_t i_iIndex, const ListItem& i_rcItem );

  void OnIndexChanged();

  bool Validate() override;

  typedef std::vector<ListItem> Container;
  Container m_vEntries;

  static const rumColor s_cDefaultSelectionActiveColor;
  static const rumColor s_cDefaultSelectionInactiveColor;

  static constexpr uint32_t DEFAULT_MAX_ENTRIES{ 32 };
  uint32_t m_uiMaxEntries{ DEFAULT_MAX_ENTRIES };

  typedef std::vector<OffsetEntry> OffsetContainer;
  OffsetContainer m_vColumnOffsets;

  std::string m_strFormat{ "0.0" };
  std::string m_strDelimiter{ "|" };

  int32_t m_iCurrentIndex{ -1 };

  uint32_t m_uiMaxSelectableEntries{ 1 };
  uint32_t m_uiMinSelectableEntries{ 1 };
  uint32_t m_uiNumSelectedEntries{ 0 };

  rumColor m_cSelectionActiveColor{ s_cDefaultSelectionActiveColor };
  rumColor m_cSelectionInactiveColor{ s_cDefaultSelectionInactiveColor };

  bool m_bHighlightCurrent{ true };
  bool m_bMultiSelect{ false };

  typedef rumClientScrollBuffer super;
};

#endif // _C_LISTVIEW_H_
