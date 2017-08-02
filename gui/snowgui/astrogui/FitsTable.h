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
public:
	explicit FitsTable(QWidget *parent = 0);
	~FitsTable();

	void	setImage(astro::image::ImagePtr);
};

} // namespace snowgui

#endif /* _FitsTable_h */
