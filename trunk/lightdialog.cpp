#include "lightdialog.h"
#include "settings.h"

LightDialog::LightDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	initUI();
}

LightDialog::~LightDialog()
{

}

void LightDialog::initUI(){
	connect(ui.buttonBoxLight, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.buttonBoxLight, SIGNAL(rejected()), this, SLOT(reject()));

	group = new QButtonGroup(this);
	group->addButton(ui.radioButtonLight0, 0);
	group->addButton(ui.radioButtonLight1, 1);
	group->addButton(ui.radioButtonLight2, 2);
	group->addButton(ui.radioButtonLight3, 3);

	QList<QAbstractButton *> list = group->buttons();
	QStringList strList;
	for(int i = 0; i < list.size(); ++i)
		strList << list[i]->text();

	Settings::instance()->initLight(strList, group->checkedId());
}

void LightDialog::accept(){
	// tu sprawdzenie ktory jest wybrany
	Settings::instance()->setLight(group->checkedId());
	QDialog::accept();
}

void LightDialog::reject(){
	// tu odrzucenie zmian
	QAbstractButton * button = group->button(Settings::instance()->getLight());
	button->setChecked(true);
	QDialog::reject();
}