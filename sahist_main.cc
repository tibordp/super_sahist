#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

#include "sahist.h"

#include <vector>

using namespace std;
using namespace boost::python;
using namespace Sahist;

namespace py = boost::python;

template<class T>
py::list std_vector_to_py_list(const std::vector<T>& v)
{
    py::object get_iter = py::iterator<std::vector<T> >();
    py::object iter = get_iter(v);
    py::list l(iter);
    return l;
}

py::list mozne_poteze(Sahovnica& sah, Barva barva) 
{
	py::list poteze;

	vector<Poteza> pot;
	sah.mozne_poteze(barva, pot);
	
	for (auto i : pot)
		poteze.append(i);
	
	return poteze;
}

// Verjetno najenostavnejši način kopiranja šahovnice

Sahovnica kopija(Sahovnica& sah) 
{
	return sah;
}

class SprostiGIL
{
public:
    inline SprostiGIL()
    {
        stanje = PyEval_SaveThread();
    }

    inline ~SprostiGIL()
    {
        PyEval_RestoreThread(stanje);
        stanje = NULL;
    }

private:
    PyThreadState * stanje;
};

// Wrapper okoli Sahovnica::predlagaj, ki nam sprosti Global Interpreter Lock, medtem ko računamo,
// zato da lahko Tkinter laufa event loop.

Poteza predlagaj_gil(Sahovnica& sah, Barva barva) {
    SprostiGIL sprosti_gil;
    return sah.predlagaj(barva);
}

BOOST_PYTHON_MODULE(sahist)
{
	enum_<Figure>("Figure")
		.value("nic", Figure::nic)
		.value("kralj", Figure::kralj)
		.value("kraljica", Figure::kraljica)
		.value("trdnjava",Figure::trdnjava)
		.value("tekac", Figure::tekac)
		.value("konj", Figure::konj)
		.value("kmet", Figure::kmet)
    ;

	enum_<Barva>("Barva")
		.value("nobena", Barva::nobena)
		.value("bela", Barva::bela)
		.value("crna", Barva::crna)
    ;    

	enum_<Poteze>("Poteze")
		.value("konec", Poteze::konec)
		.value("standardna", Poteze::standardna)
		.value("velika_rosada", Poteze::velika_rosada)
		.value("mala_rosada", Poteze::mala_rosada)
		.value("promocija",Poteze::promocija)
		.value("mimohod", Poteze::mimohod)
    ;

	class_<Koordinata>("Koordinata", init<int, int>())
    	.def_readwrite("stolpec", &Koordinata::stolpec)
    	.def_readwrite("vrstica", &Koordinata::vrstica)
    ;

	class_<Figura>("Figura")
		.def_readwrite("tip", &Figura::tip)
    	.def_readwrite("barva", &Figura::barva)
    ;

	class_<Poteza>("Poteza")
		.def_readwrite("tip", &Poteza::tip)
    	.def_readwrite("od_koordinata", &Poteza::od_koordinata)
    	.def_readwrite("do_koordinata", &Poteza::do_koordinata)
    	.def_readwrite("tip_figure", &Poteza::tip_figure)
    ;

    class_<Sahovnica>("Sahovnica", init<bool>())
    	.def("polje", &Sahovnica::polje, return_value_policy<reference_existing_object>())
    	.def("mozne_poteze", &mozne_poteze)
    	.def("zasedeno", &Sahovnica::zasedeno)
    	.def("premakni", &Sahovnica::premakni)
    	.def("sah", &Sahovnica::sah)
    	.def("predlagaj", &predlagaj_gil)
    	.def("kopija", &kopija)
    ;
}