#include "backgrounddialog.h"
#include "settings.h"

BackgroundDialog::BackgroundDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	initUI();
}

BackgroundDialog::~BackgroundDialog()
{

}

void BackgroundDialog::initUI(){
	connect(ui.buttonBoxBkg, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.buttonBoxBkg, SIGNAL(rejected()), this, SLOT(reject()));

	group = new QButtonGroup(this);
	group->addButton(ui.radioButtonBkg0, 0);
	group->addButton(ui.radioButtonBkg1, 1);
	group->addButton(ui.radioButtonBkg2, 2);
	group->addButton(ui.radioButtonBkg3, 3);
	group->addButton(ui.radioButtonBkg4, 4);

	QList<QAbstractButton *> list = group->buttons();
	QStringList strList;
	for(int i = 0; i < list.size(); ++i)
		strList << list[i]->text();

	Settings::instance()->initBkg(strList, group->checkedId());
}

void BackgroundDialog::accept(){
	// tu sprawdzenie ktory jest wybrany
	Settings::instance()->setBkg(group->checkedId());
	QDialog::accept();
}

void BackgroundDialog::reject(){
	// tu odrzucenie zmian
	QAbstractButton * button = group->button(Settings::instance()->getBkg());
	button->setChecked(true);
	QDialog::reject();
}