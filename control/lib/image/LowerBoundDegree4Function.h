/*
 * LowerBoundDegree4Function.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */

/**
 * \brief Optimization problem for symmetric quadratic functions
 */
template<>
FunctionPtr	LowerBound<Degree4Function>::symmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "symmetric quadratic problem");
	// create a problem
	glp_prob	*lp = glp_create_prob();
	glp_set_obj_dir(lp, GLP_MAX);

	// we have three columns for the three coefficients of the linear
	// function, and one constraint for each tile
	glp_add_cols(lp, 3);
	glp_set_col_name(lp, 1, "minimum");
	glp_set_col_bnds(lp, 1, GLP_LO, 0, 0);
	glp_set_col_name(lp, 2, "q0");
	glp_set_col_bnds(lp, 2, GLP_DB, -10, 10);
	glp_set_col_name(lp, 3, "m");
	glp_set_col_bnds(lp, 3, GLP_DB, -10, 10);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "columns set up");

	// got through all the rows
	glp_add_rows(lp, values.size());
	tilevaluevector::const_iterator	vp;
	unsigned int	row = 1;
	int	ind[4] = { 0, 1, 2, 3 };
	double	obj[3] = { 0., 0., 0. };
	for (vp = values.begin(); vp != values.end(); vp++, row++) {
		// row name
		char	rowname[10];
		snprintf(rowname, sizeof(rowname), "s[%d]", row);
		glp_set_row_name(lp, row, rowname);

		// row coefficients and bounds
		glp_set_row_bnds(lp, row, GLP_UP, 0, vp->second);
		double	val[4];
		double	deltax = vp->first.x() - center.x();
		double	deltay = vp->first.y() - center.y();
		double	a = sqr(deltax) + sqr(deltay);
		val[1] = 1;
		val[2] = a;
		val[3] = sqr(a);
		glp_set_mat_row(lp, row, 3, ind, val);

		// objective function
		obj[0] += 1;
		obj[1] += a;
		obj[2] += sqr(a);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rows set up");

	// objective function
	glp_set_obj_coef(lp, 1, obj[0]);
	glp_set_obj_coef(lp, 2, obj[1]);
	glp_set_obj_coef(lp, 3, obj[2]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "objective function set up");

	// solve the linear problem
	glp_simplex(lp, NULL);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "simplex solution found");

	// cleanup the problem
	glp_delete_prob(lp);

	// return the result
	Degree4Function	*q = new Degree4Function(center, true);
debug(LOG_DEBUG, DEBUG_LOG, 0, "const term: %f", glp_get_col_prim(lp, 1));
	(*q)[2] = glp_get_col_prim(lp, 1);
	(*q)[3] = glp_get_col_prim(lp, 2);
	(*q)[6] = glp_get_col_prim(lp, 3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "degree4 function: %s",
		q->toString().c_str());
	return FunctionPtr(q);
}

/**
 * \brief Optimization problem for asymmetric quadratic functions
 */
template<>
FunctionPtr	LowerBound<Degree4Function>::asymmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "symmetric quadratic problem");
	// create a problem
	glp_prob	*lp = glp_create_prob();
	glp_set_obj_dir(lp, GLP_MAX);

	// we have three columns for the three coefficients of the linear
	// function, and one constraint for each tile
	glp_add_cols(lp, 7);
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
	glp_set_col_name(lp, 7, "m");
	glp_set_col_bnds(lp, 7, GLP_DB, -10, 10);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "columns set up");

	// got through all the rows
	glp_add_rows(lp, values.size());
	tilevaluevector::const_iterator	vp;
	unsigned int	row = 1;
	int	ind[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	double	obj[7] = { 0., 0., 0., 0., 0., 0., 0. };
	for (vp = values.begin(); vp != values.end(); vp++, row++) {
		// row name
		char	rowname[10];
		snprintf(rowname, sizeof(rowname), "s[%d]", row);
		glp_set_row_name(lp, row, rowname);

		// row coefficients and bounds
		glp_set_row_bnds(lp, row, GLP_UP, 0, vp->second);
		double	deltax = vp->first.x() - center.x();
		double	deltay = vp->first.y() - center.y();
		double	val[8];
		val[1] = deltax;
		val[2] = deltay;
		val[3] = 1;
		val[4] = sqr(deltax) + sqr(deltay);
		val[5] = deltax * deltay;
		val[6] = sqr(deltax) - sqr(deltay);
		val[7] = sqr(val[4]);
		glp_set_mat_row(lp, row, 7, ind, val);

		// objective function
		for (unsigned int i = 0; i < 7; i++) {
			obj[i] += val[i + 1];
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rows set up");

	// objective function
	for (unsigned int i = 0; i < 7; i++) {
		glp_set_obj_coef(lp, i + 1, obj[i]);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "objective function set up");

	// solve the linear problem
	glp_simplex(lp, NULL);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "simplex solution found");

	// cleanup the problem
	glp_delete_prob(lp);

	// return the result
	Degree4Function	*q = new Degree4Function(center, false);
	for (unsigned int i = 0; i < 7; i++) {
		(*q)[i] = glp_get_col_prim(lp, i + 1);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "degree4 function: %s",
		q->toString().c_str());
	return FunctionPtr(q);
}
