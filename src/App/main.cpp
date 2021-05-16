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
    Profiler prof(strtoll(argv[1], NULL, 10));
    prof.start();   
	// prof.dump(prof.funcs, 0);
    auto expensive = prof.expensive_funcs({"Eigen", "std"});
	for (Func* f : expensive) {
		// std::cout << f->name << "\n";
	}
    QApplication app(argc, argv);   
    QWidget widget;
    widget.resize(640, 480);
    widget.setWindowTitle("Hello, world!!!");

    QGridLayout *gridLayout = new QGridLayout(&widget);

    QLabel* label = new QLabel(QString::fromStdString("envopt"));

    QPushButton* close = new QPushButton("Close");
	QPushButton* open = new QPushButton("Open Log");
	QPushButton* retry = new QPushButton("Re-record");	

	QObject::connect(close, &QPushButton::released, &QApplication::quit);
    QObject::connect(open, &QPushButton::released, []{system("xdg-open ./latest.log");});

	// QObject::connect(btn1, &QPushButton::released, &widget,
	//     []()
	//     {
	//         app::Window1* dialog = new app::Window1();
	//         dialog->setModal(true);
	//         dialog->setWindowModality(Qt::ApplicationModal);
	//         dialog->exec();
	//         delete dialog;
	//     });
	
	label->setAlignment(Qt::AlignVCenter);

	QGroupBox* groupbox = new QGroupBox("envopt: indev 0.0");
	groupbox->setFlat(true);
	QVBoxLayout* vbox = new QVBoxLayout;
	QGroupBox* targets = new QGroupBox("Primary Offenders");
	QGroupBox* opt = new QGroupBox("Optimization Strategies");
	QGroupBox* env = new QGroupBox("Environmental Impact");
	vbox->addWidget(targets);
	vbox->addWidget(opt);
	vbox->addWidget(env);
	groupbox->setLayout(vbox);
    gridLayout->addWidget(groupbox, 0, 0, 1, 3);
    gridLayout->addWidget(close, 1, 0);
	gridLayout->addWidget(open, 1, 1);
	gridLayout->addWidget(retry, 1, 2);
 
    widget.show();

	// uint64_t after = cpu_uJ();
	// std::cout << (after-before)/time - baseline << "mW \n";
//	return app.exec();
}
