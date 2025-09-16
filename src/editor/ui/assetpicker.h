#ifndef ASSETPICKER_H
#define ASSETPICKER_H

#include <QDialog>
#include <QSet>

#include <u_enum.h>

namespace Ui
{
  class AssetPicker;
}


class AssetPicker : public QDialog
{
  Q_OBJECT

public:

  explicit AssetPicker( AssetType i_eAssetType,
                        bool i_bPersistentOnly,
                        const QSet<rumAssetID>& i_rcIgnoredAssetsSet = QSet<rumAssetID>(),
                        QWidget* i_pcParent = 0 );
  ~AssetPicker();

signals:

  void NewPropertySelected( rumAssetID i_ePropertyID );

private slots:

  void on_buttonBox_accepted();
  void on_lineEdit_Filter_textEdited( const QString& i_strText );
  void on_pushButton_Clear_clicked();

private:

  void UpdateFilter();

  Ui::AssetPicker* m_pcUI;
};

#endif // ASSETPICKER_H
