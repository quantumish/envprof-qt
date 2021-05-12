#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <iostream>
#include <../MyLib/measure.hpp>
#include <../MyLib/profiler.hpp>

#include "Window1.h"
 
int main(int argc, char *argv[]) 
{
    // double baseline = cpu_avg_baseline_mW(std::chrono::milliseconds(1000000));
    // uint64_t before = cpu_uJ();
    // auto start = std::chrono::high_resolution_clock::now();
    Profiler prof(152712);
    prof.start();
    prof.capture_and_freeze();
    prof.capture_and_freeze();	
    //    prof.capture_and_freeze();	
    prof.dump(prof.funcs, 0);
    QApplication app(argc, argv);
 
    QWidget widget;
    widget.resize(640, 480);
    widget.setWindowTitle("Hello, world!!!");
    
    QGridLayout *gridLayout = new QGridLayout(&widget);

    	// std::ostringstream os;
    	// os << baseline;
    QLabel* label = new QLabel(QString::fromStdString("Baseline mW: "));

    QPushButton* btn1 = new QPushButton("Push Me");
    QObject::connect(btn1, &QPushButton::released, &widget,
        []() 
        { 
            app::Window1* dialog = new app::Window1();
            dialog->setModal(true);
            dialog->setWindowModality(Qt::ApplicationModal);
            dialog->exec();
            delete dialog;
        });

    label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    gridLayout->addWidget(label);
    gridLayout->addWidget(btn1);
 
    widget.show();

	// auto end = std::chrono::high_resolution_clock::now();
	// uint64_t after = cpu_uJ();
	// double time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();   
	// std::cout << (after-before)/time - baseline << "mW \n";
	// return app.exec();	
}
