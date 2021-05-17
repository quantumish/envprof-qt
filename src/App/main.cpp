#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <../envprof/measure.hpp>
#include <../envprof/profiler.hpp>

#include "Window1.h"

using json = nlohmann::json;

void fill_env_box(QGroupBox* box, Func* main)
{
	curlpp::Cleanup clean;
	const std::vector<std::string> api_codes {"CL", "PA", "WS", "WY", "SO", "WD", "NU", "OJ", "NG", "GE", "HV"};
	const std::vector<std::string> api_names {"Coal", "Petroleum", "Waste", "Wind", "Solar", "Wood",
		                                      "Nuclear", "Other Gases", "Natural Gas", "Geothermal", "Hydro"};
	std::ifstream api_file("./api_file");
	std::string api_key((std::istreambuf_iterator<char>(api_file)),  std::istreambuf_iterator<char>());
	api_key.pop_back(); // HACK
	for (int i = 0; i < api_codes.size(); i++) {
		std::ostringstream os;
		os << curlpp::options::Url("https://api.eia.gov/series/?api_key="+api_key+"&series_id=TOTAL."+api_codes[i]+"ETPUS.M"); // TODO Cache me.
	    json raw_json = json::parse(os.str());
		std::cout << api_names[i] << ": " << raw_json["series"][0]["data"][0][1] << "\n";
	} 
}

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
		str.setNum(f->energy/1e6, 'g', 4);
		QLabel* label = new QLabel(str+QString(" J"));
		percentages->addWidget(label);
	    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		label->setMargin(3);
		label->setStyleSheet("QLabel { background-color : rgba(255,0,0,"+QString::fromStdString(std::to_string(percent))+"); }");
	}
	// fill_env_box(env, expensive[0]);
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
