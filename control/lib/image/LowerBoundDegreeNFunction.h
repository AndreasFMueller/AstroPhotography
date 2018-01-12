/*
 * LowerBoundDegree4Function.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */

/**
 * \brief Optimization problem for symmetric quadratic functions
 */
template<>
FunctionPtr	LowerBound<DegreeNFunction>::symmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "symmetric degree n problem");
	// we first need to know the degree 
	int	degree = 1;
	if (end() != find(std::string("degree"))) {
		degree = at(std::string("degree"));
		if ((degree < 1) || (degree > 10)) {
			std::string	msg = stringprintf("invalid degree %d",
				degree);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "degree: %d", degree);

	// create a problem
	glp_prob	*lp = glp_create_prob();
	glp_set_obj_dir(lp, GLP_MAX);

	// we have three columns for the three coefficients of the linear
	// function, and one constraint for each tile
	glp_add_cols(lp, 2 + degree);
	glp_set_col_name(lp, 1, "minimum");
	glp_set_col_bnds(lp, 1, GLP_LO, 0, 0);
	glp_set_col_name(lp, 2, "q0");
	glp_set_col_bnds(lp, 2, GLP_DB, -10, 10);
	for (int i = 3; i < 3 + degree; i++) {
		glp_set_col_name(lp, i, stringprintf("m%d", i - 3).c_str());
		glp_set_col_bnds(lp, i, GLP_DB, -10, 10);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "columns set up");

	// got through all the rows
	glp_add_rows(lp, values.size());
	tilevaluevector::const_iterator	vp;
	unsigned int	row = 1;
	int	ind[3 + degree];
	for (int i = 0; i < 3 + degree; i++) {
		ind[i] = i;
	}
	double	obj[2 + degree];
	for (int i = 0; i < 2 + degree; i++) {
		obj[i] = 0.;
	}
	for (vp = values.begin(); vp != values.end(); vp++, row++) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "row %d", row);
		// row name
		char	rowname[10];
		snprintf(rowname, sizeof(rowname), "s[%d]", row);
		glp_set_row_name(lp, row, rowname);

		// row coefficients and bounds
		glp_set_row_bnds(lp, row, GLP_UP, 0, vp->second);
		double	val[3 + degree];
		double	deltax = vp->first.x() - center.x();
		double	deltay = vp->first.y() - center.y();
		double	a = sqr(deltax) + sqr(deltay);
		val[1] = 1;
		for (int i = 2; i < 3 + degree; i++) {
			val[i] = a * val[i - 1];
		}
		glp_set_mat_row(lp, row, 2 + degree, ind, val);

		// objective function
		for (int i = 0; i < 2 + degree; i++) {
			obj[i] += val[i + 1];
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rows set up");

	// objective function
	for (int i = 0; i < 2 + degree; i++) {
		glp_set_obj_coef(lp, i + 1, obj[i]);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "objective function set up");

	// solve the linear problem
	glp_simplex(lp, NULL);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "simplex solution found");

	// cleanup the problem
	glp_delete_prob(lp);

	// return the result
	DegreeNFunction	*q = new DegreeNFunction(center, true, degree);
	(*q)[2] = glp_get_col_prim(lp, 1);
	(*q)[3] = glp_get_col_prim(lp, 2);
	for (int i = 0; i < degree; i++) {
		(*q)[6 + i] = glp_get_col_prim(lp, 3 + i);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "degreeN function: %s",
		q->toString().c_str());
	return FunctionPtr(q);
}

/**
 * \brief Optimization problem for asymmetric quadratic functions
 */
template<>
FunctionPtr	LowerBound<DegreeNFunction>::asymmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "asymmetric degree n problem");
	// first find out the degree
	// we first need to know the degree 
	int	degree = 1;
	if (end() != find(std::string("degree"))) {
		degree = at(std::string("degree"));
		if ((degree < 1) || (degree > 10)) {
			std::string	msg = stringprintf("invalid degree %d",
				degree);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "degree: %d", degree);

	// create a problem
	glp_prob	*lp = glp_create_prob();
	glp_set_obj_dir(lp, GLP_MAX);

	// we have three columns for the three coefficients of the linear
	// function, and one constraint for each tile
	glp_add_cols(lp, 6 + degree);
	glp_set_col_name(lp, 1, "alpha");
	glp_set_col_bnds(lp, 1, GLP_DB, -10, 10);
	glp_set_col_name(lp, 2, "beta");
	glp_set_col_bnds(lp, 2, GLP_DB, -10, 10);
	glp_set_col_name(lp, 3, "gamma");
	glp_set_col_bnds(lp, 3, GLP_LO, 0, 0);
	glp_set_col_name(lp, 4, "qsymmetric");
	glp_set_col_bnds(lp, 4, GLP_DB, -10, 10);
	glp_set_col_name(lp, 5, "qmixed");
	glp_set_col_bnds(lp, 5, GLP_DB, -10, 10);
	glp_set_col_name(lp, 6, "qhyperbolic");
	glp_set_col_bnds(lp, 6, GLP_DB, -10, 10);
	for (int i = 0; i < degree; i++) {
		glp_set_col_name(lp, 7 + i, stringprintf("m%d", i).c_str());
		glp_set_col_bnds(lp, 7 + i, GLP_DB, -10, 10);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "columns set up");

	// got through all the rows
	glp_add_rows(lp, values.size());
	tilevaluevector::const_iterator	vp;
	unsigned int	row = 1;
	int	ind[7 + degree];
	for (int i = 0; i < 7 + degree; i++) {
		ind[i] = i;
	}
	double	obj[6 + degree];
	for (int i = 0; i < 6 + degree; i++) {
		obj[i] = 0.;
	}
	for (vp = values.begin(); vp != values.end(); vp++, row++) {
		// row name
		char	rowname[10];
		snprintf(rowname, sizeof(rowname), "s[%d]", row);
		glp_set_row_name(lp, row, rowname);

		// row coefficients and bounds
		glp_set_row_bnds(lp, row, GLP_UP, 0, vp->second);
		double	deltax = vp->first.x() - center.x();
		double	deltay = vp->first.y() - center.y();
		double	val[7 + degree];
		val[1] = deltax;
		val[2] = deltay;
		val[3] = 1;
		val[4] = sqr(deltax) + sqr(deltay);
		val[5] = deltax * deltay;
		val[6] = sqr(deltax) - sqr(deltay);
		val[7] = val[4] * val[4];
		for (int i = 8; i < 7 + degree; i++) {
			val[i] = val[4] * val[i - 1];
		}
		glp_set_mat_row(lp, row, 6 + degree, ind, val);

		// objective function
		for (int i = 0; i < 6 + degree; i++) {
			obj[i] += val[i + 1];
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rows set up");

	// objective function
	for (int i = 0; i < 6 + degree; i++) {
		glp_set_obj_coef(lp, i + 1, obj[i]);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "objective function set up");

	// solve the linear problem
	glp_simplex(lp, NULL);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "simplex solution found");

	// cleanup the problem
	glp_delete_prob(lp);

	// return the result
	DegreeNFunction	*q = new DegreeNFunction(center, false);
	for (int i = 0; i < 6 + degree; i++) {
		(*q)[i] = glp_get_col_prim(lp, i + 1);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "degreeN function: %s",
		q->toString().c_str());
	return FunctionPtr(q);
}
