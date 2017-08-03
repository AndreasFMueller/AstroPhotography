/*
 * FitsTable.h -- table of fits keywords and values
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapeprswil
 */
#ifndef _FitsTable_h
#define _FitsTable_h

#include <QTableWidget>
#include <AstroImage.h>

namespace snowgui {

class FitsTable : public QTableWidget {
	bool	_synthetic;
public:
	explicit FitsTable(QWidget *parent = 0);
	~FitsTable();

	bool	synthetic() const { return _synthetic; }
	void	synthetic(bool s) { _synthetic = s; }

	void	setImage(astro::image::ImagePtr);
};

} // namespace snowgui

#endif /* _FitsTable_h */
