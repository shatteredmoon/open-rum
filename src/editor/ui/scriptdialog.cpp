#include <scriptdialog.h>
#include <ui_scriptdialog.h>

#include <mainwindow.h>

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QSortFilterProxyModel>


ScriptDialog::ScriptDialog( Sqrat::Object i_sqArray, QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::ScriptDialog )
{
  m_pcUI->setupUi( this );

  Q_ASSERT( i_sqArray.GetType() == OT_ARRAY );
  Sqrat::Array sqOuterArray( i_sqArray );

  // Although specified in designer, it seems that alignment doesn't work unless explicitly set here
  m_pcUI->verticalLayout_3->setAlignment( Qt::AlignTop | Qt::AlignLeft );

  for( int32_t i = 0; i < sqOuterArray.GetSize(); ++i )
  {
    auto spValue{ sqOuterArray.GetValue<Sqrat::Object>( i ) };
    Sqrat::Array sqInnerArray( !spValue ? Sqrat::Array() : *spValue );

    // User-provided label
    spValue = sqInnerArray.GetValue<Sqrat::Object>( i );
    Sqrat::Object sqLabel{ !spValue? Sqrat::Object() : *spValue };
    QString strLabel;
    if( sqLabel.GetType() == OT_STRING )
    {
      strLabel = sqLabel.Cast<std::string>().c_str();
      QLabel* pcLabel{ new QLabel( strLabel, this ) };
      m_pcUI->verticalLayout_3->addWidget( pcLabel );
    }

    // User-provided current value
    spValue = sqInnerArray.GetValue<Sqrat::Object>( ++i );
    Sqrat::Object sqCurrentVal{ !spValue ? Sqrat::Object() : *spValue };
    switch( sqCurrentVal.GetType() )
    {
      case OT_INTEGER:
      {
        // Handle optional datasource
        spValue = sqInnerArray.GetValue<Sqrat::Object>( ++i );
        Sqrat::Object sqData{ !spValue ? Sqrat::Object() : *spValue };
        if( sqData.GetType() == OT_TABLE )
        {
          // Is the user requesting single selection or multi-selection?
          spValue = sqInnerArray.GetValue<Sqrat::Object>( ++i );
          Sqrat::Object sqMulti{ !spValue ? Sqrat::Object() : *spValue };
          if( sqMulti.GetType() == OT_BOOL )
          {
            const bool bMulti{ sqMulti.Cast<bool>() };
            if( bMulti )
            {
              // The user has specified multi-selection
              QListWidget* pcListWidget{ new QListWidget( this ) };
              pcListWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
              pcListWidget->setMaximumHeight( 500 );

              Sqrat::Object::iterator iter;
              while( sqData.Next( iter ) )
              {
                Sqrat::Object sqKey{ iter.getKey() };
                Sqrat::Object sqValue{ iter.getValue() };

                QListWidgetItem* pcItem{ new QListWidgetItem( sqKey.Cast<std::string>().c_str() ) };
                pcItem->setData( Qt::UserRole, sqValue.Cast<int32_t>() );
                pcListWidget->addItem( pcItem );
                if( ( ( sqCurrentVal.Cast<int32_t>() & sqValue.Cast<int32_t>() ) == sqValue.Cast<int32_t>() ) &&
                    sqValue.Cast<int32_t>() != 0 )
                {
                  // Bitwise AND shows that this item should be selected
                  pcListWidget->setCurrentItem( pcItem, QItemSelectionModel::Select );
                }
              }

              m_pcUI->verticalLayout_3->addWidget( pcListWidget );
              m_cInputList.append( pcListWidget );
              m_cInputTypeList.append( INTEGER_LIST_WIDGET );
            }
            else
            {
              QString strMap;

              // The user has specified single-selection
              QComboBox* pcComboBox{ new QComboBox( this ) };

              Sqrat::Object::iterator iter;
              while( sqData.Next( iter ) )
              {
                Sqrat::Object sqKey{ iter.getKey() };
                Sqrat::Object sqValue{ iter.getValue() };

                // Add the key as variant information
                pcComboBox->addItem( sqKey.Cast<std::string>().c_str(), sqValue.Cast<int32_t>() );
                if( sqCurrentVal.Cast<int32_t>() == sqValue.Cast<int32_t>() )
                {
                  // The key matches the current value, so select this item
                  strMap = sqKey.Cast<std::string>().c_str();
                }
              }

              // Sort the combo box
              QSortFilterProxyModel* pcProxy{ new QSortFilterProxyModel( pcComboBox ) };
              pcProxy->setSourceModel( pcComboBox->model() );
              pcComboBox->model()->setParent( pcProxy );
              pcComboBox->setModel( pcProxy );
              pcComboBox->model()->sort( 0 );

              // Re-select the key since sorting likely changed the index
              const int32_t iIndex{ pcComboBox->findText( strMap, Qt::MatchExactly | Qt::MatchCaseSensitive ) };
              pcComboBox->setCurrentIndex( iIndex );

              m_pcUI->verticalLayout_3->addWidget( pcComboBox );
              m_cInputList.append( pcComboBox );
              m_cInputTypeList.append( INTEGER_COMBO_BOX );
            }
          }
          else
          {
            QMessageBox::critical( this, tr( "Error" ),
                                   "Expected boolean value after table data source for label '" + strLabel +
                                   "'\ntrue = multi-selection\nfalse = single selection" );
            reject();
            return;
          }
        }
        else
        {
          // No data source
          QLineEdit* pcLineEdit{ new QLineEdit( this ) };
          pcLineEdit->setText( QString( rumScript::ToString( sqCurrentVal ).c_str() ) );
          const QValidator* pcValidator{ new QIntValidator( this ) };
          pcLineEdit->setValidator( pcValidator );
          m_pcUI->verticalLayout_3->addWidget( pcLineEdit );
          m_cInputList.append( pcLineEdit );
          m_cInputTypeList.append( OT_INTEGER );
        }
      }
      break;

      case OT_FLOAT:
      {
        QLineEdit* pcLineEdit{ new QLineEdit( this ) };
        pcLineEdit->setText( QString::number( sqCurrentVal.Cast<float>() ) );
        const QValidator* pcValidator{ new QDoubleValidator( this ) };
        pcLineEdit->setValidator( pcValidator );
        m_pcUI->verticalLayout_3->addWidget( pcLineEdit );
        m_cInputList.append( pcLineEdit );
        m_cInputTypeList.append( OT_FLOAT );

        // Ignore float datasources
        ++i;
      }
      break;

      case OT_STRING:
      {
        QLineEdit* pcLineEdit{ new QLineEdit( this ) };
        pcLineEdit->setText( QString( sqCurrentVal.Cast<std::string>().c_str() ) );
        m_pcUI->verticalLayout_3->addWidget( pcLineEdit );
        m_cInputList.append( pcLineEdit );
        m_cInputTypeList.append( OT_STRING );

        // Ignore string datasources
        ++i;
      }
      break;

      case OT_BOOL:
      {
        QComboBox* pcComboBox{ new QComboBox( this ) };
        pcComboBox->addItem( "False" );
        pcComboBox->addItem( "True" );
        m_pcUI->verticalLayout_3->addWidget( pcComboBox );

        if( sqCurrentVal.Cast<bool>() )
        {
          // True
          pcComboBox->setCurrentIndex( 1 );
        }
        else
        {
          // False
          pcComboBox->setCurrentIndex( 0 );
        }

        m_cInputList.append( pcComboBox );
        m_cInputTypeList.append( OT_BOOL );

        // Ignore boolean datasources
        ++i;
      }
      break;

      default:
        QMessageBox::critical( this, tr( "Error" ),
                               "Unsupported script value type '" +
                               QString( rumScript::GetObjectTypeName( sqCurrentVal ).c_str() ) +
                               "' for label '" + strLabel + "'" );
        reject();
        return;
    }
  }
}


ScriptDialog::~ScriptDialog()
{
  delete m_pcUI;
}


void ScriptDialog::on_buttonBox_accepted()
{
  Sqrat::Array sqArray( Sqrat::DefaultVM::Get(), m_cInputList.size() );

  // Visit each widget in the list and copy its value into the array
  for( int32_t i = 0; i < m_cInputList.size(); ++i )
  {
    switch( m_cInputTypeList[i] )
    {
      case OT_INTEGER:
      {
        const QLineEdit* pcLineEdit{ qobject_cast<QLineEdit*>( m_cInputList[i] ) };
        Q_ASSERT( pcLineEdit );
        const int32_t iValue{ pcLineEdit->text().toInt() };
        sqArray.SetValue( i, iValue );
      }
      break;

      case INTEGER_COMBO_BOX:
      {
        // Data is expected to be a single integer value
        const QComboBox* pcComboBox{ qobject_cast<QComboBox*>( m_cInputList[i] ) };
        Q_ASSERT( pcComboBox );
        const QVariant cVariant{ pcComboBox->itemData( pcComboBox->currentIndex() ) };
        sqArray.SetValue( i, cVariant.toInt() );
      }
      break;

      case INTEGER_LIST_WIDGET:
      {
        // Data is expected to be a bitfield
        int32_t iValue{ 0 };
        const QListWidget* pcListWidget{ qobject_cast<QListWidget*>( m_cInputList[i] ) };
        Q_ASSERT( pcListWidget );
        const QList<QListWidgetItem*> pcSelectedList{ pcListWidget->selectedItems() };
        for( int32_t j = 0; j < pcSelectedList.size(); ++j )
        {
          const QListWidgetItem* pcItem{ pcSelectedList.at( j ) };
          Q_ASSERT( pcItem );
          const QVariant cVariant{ pcItem->data( Qt::UserRole ) };
          iValue |= cVariant.toInt();
        }
        sqArray.SetValue( i, iValue );
      }
      break;

      case OT_FLOAT:
      {
        const QLineEdit* pcLineEdit{ qobject_cast<QLineEdit*>( m_cInputList[i] ) };
        Q_ASSERT( pcLineEdit );
        const float fValue{ pcLineEdit->text().toFloat() };
        sqArray.SetValue( i, fValue );
      }
      break;

      case OT_STRING:
      {
        const QLineEdit* pcLineEdit{ qobject_cast<QLineEdit*>( m_cInputList[i] ) };
        Q_ASSERT( pcLineEdit );
        sqArray.SetValue( i, qPrintable( pcLineEdit->text() ) );
      }
      break;

      case OT_BOOL:
      {
        const QComboBox* pcComboBox{ qobject_cast<QComboBox*>( m_cInputList[i] ) };
        Q_ASSERT( pcComboBox );
        const bool bValue{ ( pcComboBox->currentText().compare( "True", Qt::CaseInsensitive ) == 0 ) };
        sqArray.SetValue( i, bValue );
      }
      break;

      default:
        // This should never happen, but if it does, an unsupported type somehow snuck through
        Q_ASSERT( false );
        QMessageBox::critical( this, tr( "Error" ), "Unsupported script value type" );
        reject();
        return;
    }
  }

  // Store the results of this dialog's changes
  MainWindow::SetScriptModalDialogResult( sqArray );
  accept();
}
