#ifndef NEWPROJECT_H
#define NEWPROJECT_H

#include <QDialog>

namespace Ui
{
  class NewProject;
}


class NewProject : public QDialog
{
  Q_OBJECT

public:

  explicit NewProject( bool i_bEditMode = false, QWidget* i_pcParent = 0 );
  ~NewProject();

  bool IsAdding() const
  {
    return !m_bEditMode;
  }

  bool IsEditing() const
  {
    return m_bEditMode;
  }

private slots:

  void on_buttonBox_accepted();
  void on_lineEdit_Project_Folder_textEdited( const QString& i_strText );
  void on_lineEdit_Title_textEdited( const QString& i_strText );
  void on_lineEdit_Uuid_textEdited( const QString& i_strText );
  void on_pushButton_Audio_Path_Hint_clicked();
  void on_pushButton_Graphics_Path_Hint_clicked();
  void on_pushButton_Uuid_clicked();

private:

  bool UpdateDatabase();
  bool ValidateInput();

  Ui::NewProject* m_pcUI;

  bool m_bEditMode;
};

#endif // NEWPROJECT_H
