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
    double cpu_baseline = collect_baseline_mW(cpu_uJ, std::chrono::milliseconds(5000));
	double gpu_baseline = collect_baseline_mW(gpu_uJ, std::chrono::milliseconds(5000));
	auto prev_cpu = cpu_uJ();
	auto prev_gpu = gpu_uJ();      
    auto start = std::chrono::high_resolution_clock::now();
	// char* args[2] = {NULL};
    // Profiler prof(strtoll(argv[1], NULL, 10));
    // prof.start();   
	// prof.dump(prof.funcs, 0);
    QApplication app(argc, argv);
 
    QWidget widget;
    widget.resize(640, 480);
    widget.setWindowTitle("Hello, world!!!");
    
    QGridLayout *gridLayout = new QGridLayout(&widget);

    // 	// std::ostringstream os;
    // 	// os << baseline;
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

	// uint64_t after = cpu_uJ();
	// std::cout << (after-before)/time - baseline << "mW \n";
	// return app.exec();
	app.exec();
	auto end = std::chrono::high_resolution_clock::now();   
	auto curr_cpu = cpu_uJ();
	auto curr_gpu = gpu_uJ();
	double time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();   
	std::cout << (curr_cpu - prev_cpu)/time - cpu_baseline << " " << (curr_gpu - prev_gpu)/time - gpu_baseline << "\n";	
}
