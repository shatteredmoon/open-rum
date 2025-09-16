#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>

class FindDialog;
class FindReplaceDialog;

namespace Ui
{
  class ScriptEditor;
}

class ScriptEditor : public QWidget
{
  Q_OBJECT

public:

  ScriptEditor( QWidget* i_pcParent = 0 );
  ScriptEditor( const QString& i_strFilePath, QWidget* i_pcParent = 0 );
  ~ScriptEditor();

  // Forces dirty state on the underlying document
  void ForceModification();

  // Returns true if the script has been modified
  bool IsDirty() const;

  // If dirty, this gives the user a chance to save or cancel
  // Returns true unless the user presses cancel
  bool RequestClose();

private slots:

  void on_actionFind_triggered();
  void on_actionFindReplace_triggered();
  void on_actionGoto_Pos_triggered();
  void on_actionSave_Script_triggered();

private:

  void Init();
  bool Open( const QString& i_strFilePath );
  void Save();

  QString m_strFilePath;

  FindDialog* m_pcFindDialog;
  FindReplaceDialog* m_pcFindReplaceDialog;

  Ui::ScriptEditor* m_pcUI;
};

#endif // SCRIPTEDITOR_H
