#ifndef SM_TEXTEDIT_H
#define SM_TEXTEDIT_H

#include <QTextEdit>

class smTextEdit : public QTextEdit
{
  Q_OBJECT

public:

  smTextEdit( QWidget* i_pcParent = 0 );
  smTextEdit( const QString& i_strText, QWidget* i_pcParent = 0 );

  // Returns true if the document has been modified
  bool IsDirty() const
  {
    return m_bDirty;
  }

public slots:

  void onCursorPositionChanged();
  void onGotoCursorPosition( int32_t i_iLine, int32_t i_iColumn );
  void onModificationChanged( bool i_bChanged );

private:

  void Init();

  // True when the document is in a modified state
  bool m_bDirty;
};

#endif // SM_TEXTEDIT_H
