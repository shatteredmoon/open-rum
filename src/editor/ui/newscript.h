#ifndef NEWSCRIPT_H
#define NEWSCRIPT_H

#include <QDialog>

namespace Ui
{
  class NewScript;
}

class NewScript : public QDialog
{
  Q_OBJECT

public:

  explicit NewScript( QWidget* i_pcParent = 0 );
  ~NewScript();

signals:

  void scriptCreated( const QString& );

private slots:

  void on_buttonBox_accepted();
  void on_lineEditFile_textEdited( const QString& i_strText );
  void on_pushButtonFile_clicked();

private:

  bool ValidateInput();

  Ui::NewScript* m_pcUI;

  QString m_strAbsoluteFilePath;
};

#endif // NEWSCRIPT_H
