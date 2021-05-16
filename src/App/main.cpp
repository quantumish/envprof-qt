#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <iostream>
#include <../MyLib/measure.hpp>
#include <../MyLib/profiler.hpp>

#include "Window1.h"
 
int main(int argc, char *argv[]) 
{
	setenv("XDG_RUNTIME_DIR", "/tmp/runtime-root", 0);
	setenv("FONTCONFIG_PATH", "/etc/fonts", 0);	
    Profiler prof(strtoll(argv[1], NULL, 10));
    prof.start();   
	// prof.dump(prof.funcs, 0);
    auto expensive = prof.expensive_funcs({"Eigen", "std"});

    QApplication app(argc, argv);   
    QWidget widget;
    widget.resize(640, 480);
    widget.setWindowTitle("Hello, world!!!");

    QGridLayout *gridLayout = new QGridLayout(&widget);

    QPushButton* close = new QPushButton("&Close");
	QPushButton* open = new QPushButton("Open Log");
	QPushButton* retry = new QPushButton("Re-record");	

	QObject::connect(close, &QPushButton::released, &QApplication::quit);
    QObject::connect(open, &QPushButton::released, []{system("xdg-open ./latest.log");});
	

	QGroupBox* groupbox = new QGroupBox("envopt: indev 0.0");
	groupbox->setFlat(true);
	QVBoxLayout* vbox = new QVBoxLayout;
	QGroupBox* targets = new QGroupBox("Primary Offenders");
	QGroupBox* opt = new QGroupBox("Optimization Strategies");
	QGroupBox* env = new QGroupBox("Environmental Impact");
    QGridLayout* targets_layout = new QGridLayout;
	targets->setLayout(targets_layout);
	QVBoxLayout* names = new QVBoxLayout;
	QVBoxLayout* percentages = new QVBoxLayout;
	targets_layout->addLayout(names, 0, 0);
	targets_layout->addLayout(percentages, 0, 1);   
	for (Func* f : expensive) {
	    names->addWidget(new QLabel(QString::fromStdString(f->name)));
		QString str;		
		double percent = f->energy/static_cast<double>(prof.total);
		str.setNum(percent*100, 'g', 4);
		QLabel* label = new QLabel(str);
		percentages->addWidget(label);
	    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		label->setMargin(3);
		label->setStyleSheet("QLabel { background-color : rgba(255,0,0,"+QString::fromStdString(std::to_string(percent))+"); }");
	}
	vbox->addWidget(targets);
	vbox->addWidget(opt);
	vbox->addWidget(env);
	groupbox->setLayout(vbox);
    gridLayout->addWidget(groupbox, 0, 0, 1, 3);
    gridLayout->addWidget(close, 1, 0);
	gridLayout->addWidget(open, 1, 1);
	gridLayout->addWidget(retry, 1, 2);
 
    widget.show();

	return app.exec();
}
