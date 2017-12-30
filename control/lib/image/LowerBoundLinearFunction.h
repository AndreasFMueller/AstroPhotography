/*
 * LowerBoundLinearRunction.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */

/**
 * \brief problem for symmetric linear functions
 *
 * A symmetric linear function is simply a constant, which is the minimum
 * of all values in the tiles.
 */
template<>
FunctionPtr	LowerBound<LinearFunction>::symmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	double	minimum = std::numeric_limits<double>::infinity();
	tilevaluevector::const_iterator	i;
	for (i = values.begin(); i != values.end(); i++) {
		if (i->second < minimum) {
			minimum = i->second;
		}
	}
	LinearFunction	*result = new LinearFunction(center, true);
	(*result)[2] = minimum;
	return FunctionPtr(result);
}

/**
 * \brief Optimization problem for asymmetric linear functions
 */
template<>
FunctionPtr	LowerBound<LinearFunction>::asymmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "asymmetric linear problem");
	// create a problem
	glp_prob	*lp = glp_create_prob();
	glp_set_obj_dir(lp, GLP_MAX);

	// we have three columns for the three coefficients of the linear
	// function, and one constraint for each tile
	glp_add_cols(lp, 3);
	glp_set_col_name(lp, 1, "alpha");
	glp_set_col_bnds(lp, 1, GLP_DB, -10, 10);
	glp_set_col_name(lp, 2, "beta");
	glp_set_col_bnds(lp, 2, GLP_DB, -10, 10);
	glp_set_col_name(lp, 3, "gamma");
	glp_set_col_bnds(lp, 3, GLP_LO, 0, 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "columns set up");

	// got through all the rows
	glp_add_rows(lp, values.size());
	tilevaluevector::const_iterator	vp;
	unsigned int	row = 1;
	int	ind[4] = { 0, 1, 2, 3 };
	double	obj[3] = { 0., 0., (double)values.size() };
	for (vp = values.begin(); vp != values.end(); vp++, row++) {
		// row name
		char	rowname[10];
		snprintf(rowname, sizeof(rowname), "s[%d]", row);
		glp_set_row_name(lp, row, rowname);

		// row coefficients and bounds
		glp_set_row_bnds(lp, row, GLP_UP, 0, vp->second);
		double	deltax = vp->first.x() - center.x();
		double	deltay = vp->first.y() - center.y();
		double	val[4];
		val[1] = deltax;
		val[2] = deltay;
		val[3] = 1;
		glp_set_mat_row(lp, row, 3, ind, val);

		// objective function
		obj[0] += deltax;
		obj[1] += deltay;
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
	LinearFunction	*lb = new LinearFunction(center, false);
	(*lb)[0] = glp_get_col_prim(lp, 1);
	(*lb)[1] = glp_get_col_prim(lp, 2);
	(*lb)[2] = glp_get_col_prim(lp, 3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "linear function: %s",
		lb->toString().c_str());
	return FunctionPtr(lb);
}

