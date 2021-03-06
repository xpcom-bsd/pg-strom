/*
 * matrix.c
 *
 * Matrix data type support
 * ----
 * Copyright 2011-2018 (C) KaiGai Kohei <kaigai@kaigai.gr.jp>
 * Copyright 2014-2018 (C) The PG-Strom Development Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "pg_strom.h"
#include "cuda_matrix.h"
#include <math.h>

/* function declarations */
extern Datum array_matrix_accum(PG_FUNCTION_ARGS);
extern Datum array_matrix_accum_varbit(PG_FUNCTION_ARGS);
extern Datum varbit_to_int4_array(PG_FUNCTION_ARGS);
extern Datum int4_array_to_varbit(PG_FUNCTION_ARGS);
extern Datum array_matrix_final_bool(PG_FUNCTION_ARGS);
extern Datum array_matrix_final_int2(PG_FUNCTION_ARGS);
extern Datum array_matrix_final_int4(PG_FUNCTION_ARGS);
extern Datum array_matrix_final_int8(PG_FUNCTION_ARGS);
extern Datum array_matrix_final_float4(PG_FUNCTION_ARGS);
extern Datum array_matrix_final_float8(PG_FUNCTION_ARGS);
extern Datum array_matrix_unnest(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_bool(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_int2(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_int4(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_int8(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_float4(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_float8(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_boolt(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_boolb(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_int2t(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_int2b(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_int4t(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_int4b(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_int8t(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_int8b(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_float4t(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_float4b(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_float8t(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_scalar_float8b(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_bool(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_int2(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_int4(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_int8(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_float4(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_float8(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_booll(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_boolr(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_int2l(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_int2r(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_int4l(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_int4r(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_int8l(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_int8r(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_float4l(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_float4r(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_float8l(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_scalar_float8r(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_accum(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_final_bool(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_final_int2(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_final_int4(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_final_int8(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_final_float4(PG_FUNCTION_ARGS);
extern Datum array_matrix_rbind_final_float8(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_accum(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_final_bool(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_final_int2(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_final_int4(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_final_int8(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_final_float4(PG_FUNCTION_ARGS);
extern Datum array_matrix_cbind_final_float8(PG_FUNCTION_ARGS);
extern Datum array_matrix_transpose_bool(PG_FUNCTION_ARGS);
extern Datum array_matrix_transpose_int2(PG_FUNCTION_ARGS);
extern Datum array_matrix_transpose_int4(PG_FUNCTION_ARGS);
extern Datum array_matrix_transpose_int8(PG_FUNCTION_ARGS);
extern Datum array_matrix_transpose_float4(PG_FUNCTION_ARGS);
extern Datum array_matrix_transpose_float8(PG_FUNCTION_ARGS);
extern Datum float4_as_int4(PG_FUNCTION_ARGS);
extern Datum int4_as_float4(PG_FUNCTION_ARGS);
extern Datum float8_as_int8(PG_FUNCTION_ARGS);
extern Datum int8_as_float8(PG_FUNCTION_ARGS);
extern Datum array_matrix_validation(PG_FUNCTION_ARGS);
extern Datum array_matrix_height(PG_FUNCTION_ARGS);
extern Datum array_matrix_width(PG_FUNCTION_ARGS);
extern Datum array_matrix_rawsize(PG_FUNCTION_ARGS);

extern Datum postgresql_type_rawsize(PG_FUNCTION_ARGS);
extern Datum composite_type_rawsize(PG_FUNCTION_ARGS);

/* fmgr macros for regular varlena matrix  objects */
#define DatumGetMatrixTypeP(X)					\
	((MatrixType *) PG_DETOAST_DATUM(X))
#define DatumGetMatrixTypePCopy(X)				\
	((MatrixType *) PG_DETOAST_DATUM_COPY(X))
#define PG_GETARG_MATRIXTYPE_P(n)				\
	DatumGetMatrixTypeP(PG_GETARG_DATUM(n))
#define PG_GETARG_MATRIXTYPE_P_COPY(n)			\
	DatumGetMatrixTypePCopy(PG_GETARG_DATUM(n))
#define PG_RETURN_MATRIXTYPE_P(x)		PG_RETURN_POINTER(x)

/*
 * create_empty_matrix
 */
static MatrixType *
create_empty_matrix(Oid type_oid, cl_uint width, cl_uint height)
{
	Size		type_len;
	MatrixType *M;

	Assert(width > 0);
	switch (type_oid)
	{
		case BOOLOID:	type_len = sizeof(bool);	break;
		case INT2OID:	type_len = sizeof(int16);	break;
		case INT4OID:	type_len = sizeof(int32);	break;
		case INT8OID:	type_len = sizeof(int64);	break;
		case FLOAT4OID:	type_len = sizeof(float);	break;
		case FLOAT8OID:	type_len = sizeof(double);	break;
		default:
			elog(ERROR, "array-matrix does not support: %s",
				 format_type_be(type_oid));
	}
	M = palloc(ARRAY_MATRIX_RAWSIZE(type_len,height,width));
	if (width == 1)
		INIT_ARRAY_VECTOR(M,type_oid,type_len,height);
	else
		INIT_ARRAY_MATRIX(M,type_oid,type_len,height,width);
	return M;
}

/*
 * Constructor of Matrix-like Array
 */
typedef struct
{
	Oid			elemtype;	/* element type of the input array */
	cl_uint		width;		/* max width of the input vector */
	List	   *rows;		/* list of the supplied vector */
} array_matrix_state;

Datum
array_matrix_accum(PG_FUNCTION_ARGS)
{
	array_matrix_state *amstate;
	MemoryContext	aggcxt;
	MemoryContext	oldcxt;
	ArrayType	   *array;
	cl_uint			width;

	if (!AggCheckCallContext(fcinfo, &aggcxt))
		elog(ERROR, "aggregate function called in non-aggregate context");

	if (PG_ARGISNULL(1))
		elog(ERROR, "null-array was supplied");

	oldcxt = MemoryContextSwitchTo(aggcxt);
	array = PG_GETARG_ARRAYTYPE_P_COPY(1);

	/* sanity check */
	if (ARR_NDIM(array) != 1)
		elog(ERROR, "input array was not 1-dimension array");
	if (ARR_ELEMTYPE(array) != BOOLOID &&
		ARR_ELEMTYPE(array) != INT2OID &&
		ARR_ELEMTYPE(array) != INT4OID &&
		ARR_ELEMTYPE(array) != INT8OID &&
		ARR_ELEMTYPE(array) != FLOAT4OID &&
		ARR_ELEMTYPE(array) != FLOAT8OID)
		elog(ERROR, "unsupported element type: %s",
			 format_type_be(ARR_ELEMTYPE(array)));

	width = ARR_LBOUND(array)[0] + ARR_DIMS(array)[0] - 1;

	if (PG_ARGISNULL(0))
	{
		amstate = palloc0(sizeof(array_matrix_state));
		amstate->elemtype = array->elemtype;
	}
	else
		amstate = (array_matrix_state *)PG_GETARG_POINTER(0);

	amstate->width = Max(amstate->width, width);
	amstate->rows = lappend(amstate->rows, array);

	MemoryContextSwitchTo(oldcxt);

	PG_RETURN_POINTER(amstate);
}
PG_FUNCTION_INFO_V1(array_matrix_accum);

static MatrixType *
__varbit_to_int_vector(VarBit *varbit)
{
	MatrixType *matrix;
	Size		nitems;
	Size		len;

	if (!varbit)
	{
		len = ARRAY_VECTOR_RAWSIZE(sizeof(cl_int), 0);
		matrix = palloc0(len);
		INIT_ARRAY_VECTOR(matrix, INT4OID, sizeof(cl_int), 0);
	}
	else
	{
		nitems = ((varbit->bit_len + sizeof(cl_int) * BITS_PER_BYTE - 1)
				  / (sizeof(cl_int) * BITS_PER_BYTE));

		len = ARRAY_VECTOR_RAWSIZE(sizeof(cl_int), nitems);
		matrix = palloc0(len);
		INIT_ARRAY_VECTOR(matrix, INT4OID, sizeof(cl_int), nitems);
		memcpy(matrix->u.d1.values, varbit->bit_dat,
			   (varbit->bit_len + BITS_PER_BYTE - 1) / BITS_PER_BYTE);
	}
	return matrix;
}

Datum
varbit_to_int4_array(PG_FUNCTION_ARGS)
{
	VarBit	   *varbit = PG_GETARG_VARBIT_P(0);

	PG_RETURN_POINTER(__varbit_to_int_vector(varbit));
}
PG_FUNCTION_INFO_V1(varbit_to_int4_array);

Datum
int4_array_to_varbit(PG_FUNCTION_ARGS)
{
	AnyArrayType   *array = PG_GETARG_ANY_ARRAY_P(0);
	VarBit		   *varbit;
	Size			len;
	cl_int			i, nitems;
	Datum			datum;
	bool			isnull;
	array_iter		iter;

	/* sanity check - only vector like array is valid */
	if (AARR_NDIM(array) != 1)
		elog(ERROR, "Only 1D array is supported");
	nitems = AARR_DIMS(array)[0];

	len = MAXALIGN(offsetof(VarBit, bit_dat[sizeof(cl_int) * nitems]));
	varbit = palloc0(len);
	SET_VARSIZE(varbit, len);
	varbit->bit_len = sizeof(cl_int) * BITS_PER_BYTE * nitems;

	array_iter_setup(&iter, array);
	for (i=0; i < nitems; i++)
	{
		datum = array_iter_next(&iter, &isnull, i,
								sizeof(cl_int), true, 'i');
		if (isnull)
			continue;
		((cl_int *)varbit->bit_dat)[i] =  DatumGetInt32(datum);
	}
	PG_RETURN_POINTER(varbit);
}
PG_FUNCTION_INFO_V1(int4_array_to_varbit);

Datum
array_matrix_accum_varbit(PG_FUNCTION_ARGS)
{
	array_matrix_state *amstate;
	MemoryContext	aggcxt;
	MemoryContext	oldcxt;
	VarBit		   *varbit = NULL;
	MatrixType	   *matrix;

	if (!AggCheckCallContext(fcinfo, &aggcxt))
		elog(ERROR, "aggregate function called in non-aggregate context");

	oldcxt = MemoryContextSwitchTo(aggcxt);
	if (!PG_ARGISNULL(1))
		varbit = PG_GETARG_VARBIT_P(1);
	matrix = __varbit_to_int_vector(varbit);

	if (PG_ARGISNULL(0))
	{
		amstate = palloc0(sizeof(array_matrix_state));
		amstate->elemtype = INT4OID;
	}
	else
		amstate = (array_matrix_state *)PG_GETARG_POINTER(0);

	amstate->width = Max(amstate->width, ARRAY_MATRIX_HEIGHT(matrix));
	amstate->rows = lappend(amstate->rows, matrix);

	MemoryContextSwitchTo(oldcxt);

	PG_RETURN_POINTER(amstate);
}
PG_FUNCTION_INFO_V1(array_matrix_accum_varbit);

#define ARRAY_MATRIX_FINAL_TEMPLATE(R,amstate,BASETYPE)					\
	do {																\
		Size		width = (amstate)->width;							\
		Size		height = list_length((amstate)->rows);				\
		Size		length;												\
		Size		row_index;											\
		int16		typlen;												\
		bool		typbyval;											\
		char		typalign;											\
		ListCell   *lc;													\
																		\
		get_typlenbyvalalign((amstate)->elemtype,						\
							 &typlen, &typbyval, &typalign);			\
		Assert(typlen == sizeof(BASETYPE));								\
		length = ARRAY_MATRIX_RAWSIZE(typlen, height, width);			\
		if (!AllocSizeIsValid(length))									\
			elog(ERROR, "supplied array-matrix is too big");			\
		R = palloc(length);												\
		INIT_ARRAY_MATRIX(R, (amstate)->elemtype, typlen, height, width); \
																		\
		row_index = 0;													\
		foreach (lc, (amstate)->rows)									\
		{																\
			ArrayType  *array = lfirst(lc);								\
			Size		offset = ARR_LBOUND(array)[0] - 1;				\
			Size		i, nitems = ARR_DIMS(array)[0];					\
			BASETYPE   *dest;											\
			array_iter	iter;											\
			Datum		datum;											\
			Datum		mask;											\
			bool		isnull;											\
																		\
			/* sanity checks */											\
			Assert(ARR_ELEMTYPE(array) == (amstate)->elemtype &&		\
				   ARR_NDIM(array) == 1);								\
			dest = ((BASETYPE *)ARRAY_MATRIX_DATAPTR(R)) + row_index;	\
			mask = (sizeof(BASETYPE) < sizeof(Datum)					\
					? ((1UL << (sizeof(BASETYPE) * 8)) - 1)				\
					: ~0UL);											\
			for (i=0; i < offset; i++, dest += height)					\
				*dest = 0;												\
			array_iter_setup(&iter, (AnyArrayType *)array);				\
			for (i=0; i < nitems; i++, dest += height)					\
			{															\
				datum = array_iter_next(&iter, &isnull, i,				\
										typlen, typbyval, typalign);	\
				if (isnull)												\
					datum = 0;											\
				*dest = (BASETYPE)(datum & mask);						\
			}															\
																		\
			for (i = offset + nitems; i < width; i++, dest += height)	\
				*dest = 0;												\
																		\
			row_index++;												\
		}																\
	} while(0)

Datum
array_matrix_final_bool(PG_FUNCTION_ARGS)
{
	array_matrix_state *amstate;
	MatrixType *R;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	amstate = (array_matrix_state *)PG_GETARG_POINTER(0);
	Assert(amstate->elemtype == BOOLOID);
	ARRAY_MATRIX_FINAL_TEMPLATE(R,amstate,cl_uchar);
	PG_RETURN_POINTER(R);
}
PG_FUNCTION_INFO_V1(array_matrix_final_bool);

Datum
array_matrix_final_int2(PG_FUNCTION_ARGS)
{
	array_matrix_state *amstate;
	MatrixType *R;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	amstate = (array_matrix_state *)PG_GETARG_POINTER(0);
	Assert(amstate->elemtype == INT2OID);
	ARRAY_MATRIX_FINAL_TEMPLATE(R,amstate,cl_ushort);
	PG_RETURN_POINTER(R);
}
PG_FUNCTION_INFO_V1(array_matrix_final_int2);

Datum
array_matrix_final_int4(PG_FUNCTION_ARGS)
{
	array_matrix_state *amstate;
	MatrixType *R;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	amstate = (array_matrix_state *)PG_GETARG_POINTER(0);
	Assert(amstate->elemtype == INT4OID);
	ARRAY_MATRIX_FINAL_TEMPLATE(R,amstate,cl_uint);
	PG_RETURN_POINTER(R);
}
PG_FUNCTION_INFO_V1(array_matrix_final_int4);

Datum
array_matrix_final_int8(PG_FUNCTION_ARGS)
{
	array_matrix_state *amstate;
	MatrixType *R;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	amstate = (array_matrix_state *)PG_GETARG_POINTER(0);
	Assert(amstate->elemtype == INT8OID);
	ARRAY_MATRIX_FINAL_TEMPLATE(R,amstate,cl_ulong);
	PG_RETURN_POINTER(R);
}
PG_FUNCTION_INFO_V1(array_matrix_final_int8);

Datum
array_matrix_final_float4(PG_FUNCTION_ARGS)
{
	array_matrix_state *amstate;
	MatrixType *R;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	amstate = (array_matrix_state *)PG_GETARG_POINTER(0);
	Assert(amstate->elemtype == FLOAT4OID);
	ARRAY_MATRIX_FINAL_TEMPLATE(R,amstate,cl_uint);
	PG_RETURN_POINTER(R);
}
PG_FUNCTION_INFO_V1(array_matrix_final_float4);

Datum
array_matrix_final_float8(PG_FUNCTION_ARGS)
{
	array_matrix_state *amstate;
	MatrixType *R;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	amstate = (array_matrix_state *)PG_GETARG_POINTER(0);
	Assert(amstate->elemtype == FLOAT8OID);
	ARRAY_MATRIX_FINAL_TEMPLATE(R,amstate,cl_ulong);
	PG_RETURN_POINTER(R);
}
PG_FUNCTION_INFO_V1(array_matrix_final_float8);

/*
 * Validator of matrix-like array
 */
Datum
array_matrix_validation(PG_FUNCTION_ARGS)
{
	MatrixType *matrix = PG_GETARG_MATRIXTYPE_P(0);

	PG_RETURN_BOOL(VALIDATE_ARRAY_MATRIX(matrix));
}
PG_FUNCTION_INFO_V1(array_matrix_validation);

/*
 * Size estimator
 */
Datum
array_matrix_rawsize(PG_FUNCTION_ARGS)
{
	Oid		elemtype = PG_GETARG_OID(0);
	int32	height = (PG_NARGS() > 1 ? PG_GETARG_INT32(1) : 1);
	int32	width = (PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 1);
	int32	depth = (PG_NARGS() > 3 ? PG_GETARG_INT32(3) : 1);
	int16	typlen;

	switch (elemtype)
	{
		case BOOLOID:
		case INT2OID:
		case INT4OID:
		case INT8OID:
		case FLOAT4OID:
		case FLOAT8OID:
			typlen = get_typlen(elemtype);
			break;
		default:
			elog(ERROR, "unable to make array-matrix with '%s' type",
				 format_type_be(elemtype));
	}
	PG_RETURN_INT64(ARRAY_CUBE_RAWSIZE(typlen, depth, height, width));
}
PG_FUNCTION_INFO_V1(array_matrix_rawsize);

Datum
array_matrix_height(PG_FUNCTION_ARGS)
{
	if (!PG_ARGISNULL(0))
	{
		MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);

		if (VARATT_IS_EXPANDED_HEADER(M) ||
			!VALIDATE_ARRAY_MATRIX(M))
			elog(ERROR, "not a matrix-like array");
		PG_RETURN_INT32(ARRAY_MATRIX_HEIGHT(M));
	}
	PG_RETURN_INT32(0);
}
PG_FUNCTION_INFO_V1(array_matrix_height);

Datum
array_matrix_width(PG_FUNCTION_ARGS)
{
	if (!PG_ARGISNULL(0))
	{
		MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);

		if (VARATT_IS_EXPANDED_HEADER(M) ||
			!VALIDATE_ARRAY_MATRIX(M))
			elog(ERROR, "not a matrix-like array");
		PG_RETURN_INT32(ARRAY_MATRIX_WIDTH(M));
	}
	PG_RETURN_INT32(0);
}
PG_FUNCTION_INFO_V1(array_matrix_width);

Datum
array_matrix_unnest(PG_FUNCTION_ARGS)
{
	struct {
		MatrixType	   *matrix;
		TupleTableSlot *slot;
		int16			typlen;
		bool			typbyval;
		char			typalign;
	}				   *state;
	FuncCallContext	   *fncxt;
	MatrixType		   *matrix;
	TupleTableSlot	   *slot;
	HeapTuple			tuple;
	cl_int				height;
	cl_int				width;
	cl_int				i;
	char			   *source;

	if (SRF_IS_FIRSTCALL())
	{
		TupleDesc		tupdesc;
		MemoryContext	oldcxt;
		MatrixType	   *matrix;

		fncxt = SRF_FIRSTCALL_INIT();
		oldcxt = MemoryContextSwitchTo(fncxt->multi_call_memory_ctx);
		state = palloc0(sizeof(*state));
		matrix = PG_GETARG_MATRIXTYPE_P(0);

		/*
		 * TODO: Allow general 1D/2D array to unnest
		 */
		if (VARATT_IS_EXPANDED_HEADER(matrix))
			elog(ERROR, "ExpandedArrayHeader is not supported");
		if (!VALIDATE_ARRAY_MATRIX(matrix))
			elog(ERROR, "Not a matrix-like array");

		get_typlenbyvalalign(matrix->elemtype,
							 &state->typlen,
							 &state->typbyval,
							 &state->typalign);
		width = ARRAY_MATRIX_WIDTH(matrix);
		tupdesc = CreateTemplateTupleDesc(width, false);
		for (i=0; i < width; i++)
		{
			TupleDescInitEntry(tupdesc,
							   (AttrNumber) i+1,
							   psprintf("c%u", i+1),
							   ARRAY_MATRIX_ELEMTYPE(matrix), -1, 0);
		}
		fncxt->tuple_desc = BlessTupleDesc(tupdesc);

		state->matrix = matrix;
		state->slot = MakeSingleTupleTableSlot(fncxt->tuple_desc);
		fncxt->user_fctx = state;

		MemoryContextSwitchTo(oldcxt);
	}
	fncxt = SRF_PERCALL_SETUP();
	state = fncxt->user_fctx;
	matrix = state->matrix;
	width = ARRAY_MATRIX_WIDTH(matrix);
	height = ARRAY_MATRIX_HEIGHT(matrix);
	slot = state->slot;

	if (fncxt->call_cntr >= height)
		SRF_RETURN_DONE(fncxt);
	ExecClearTuple(slot);
	source = ARRAY_MATRIX_DATAPTR(matrix) + state->typlen * fncxt->call_cntr;
	memset(slot->tts_isnull, 0, sizeof(bool) * width);
	for (i=0; i < width; i++)
	{
		switch (state->typlen)
		{
			case sizeof(cl_ushort):
				slot->tts_values[i] = *((cl_ushort *)source);
				source += sizeof(cl_ushort) * height;
				break;
			case sizeof(cl_uint):
				slot->tts_values[i] = *((cl_uint *)source);
				source += sizeof(cl_uint) * height;
				break;
			case sizeof(cl_ulong):
				slot->tts_values[i] = *((cl_ulong *)source);
				source += sizeof(cl_ulong) * height;
				break;
			default:
				elog(ERROR, "unexpecter type length: %d", state->typlen);
		}
	}
	ExecStoreVirtualTuple(slot);
	tuple = ExecMaterializeSlot(slot);
	SRF_RETURN_NEXT(fncxt, HeapTupleGetDatum(tuple));
}
PG_FUNCTION_INFO_V1(array_matrix_unnest);

/*
 * rbind that takes two arrays
 */
static MatrixType *
array_martix_rbind(Oid elemtype, MatrixType *X, MatrixType *Y)
{
	MatrixType *R;
	cl_int		r_width, x_width, y_width;
	cl_int		r_height, x_height, y_height;
	int			typlen;
	Size		length;
	int			i;
	char	   *src, *dst;

	/* sanity checks */
	if (VARATT_IS_EXPANDED_HEADER(X) || VARATT_IS_EXPANDED_HEADER(Y))
		elog(ERROR, "ExpandedArrayHeader is not supported");
	if (!VALIDATE_ARRAY_MATRIX(X) || !VALIDATE_ARRAY_MATRIX(Y))
		elog(ERROR, "Not a matrix-like array");
	if (elemtype != ARRAY_MATRIX_ELEMTYPE(X) ||
		elemtype != ARRAY_MATRIX_ELEMTYPE(Y))
		elog(ERROR, "Bug? not expected type");
	typlen = get_typlen(elemtype);

	x_width = ARRAY_MATRIX_WIDTH(X);
	y_width = ARRAY_MATRIX_WIDTH(Y);
	r_width = Max(x_width, y_width);
	x_height = ARRAY_MATRIX_HEIGHT(X);
	y_height = ARRAY_MATRIX_HEIGHT(Y);
	r_height = x_height + y_height;

	length = ARRAY_MATRIX_RAWSIZE(typlen, r_width, r_height);
	R = palloc(length);
	SET_VARSIZE(R, length);
	INIT_ARRAY_MATRIX(R, elemtype, typlen, r_height, r_width);

	/* copy from the top-matrix */
	src = ARRAY_MATRIX_DATAPTR(X);
	dst = ARRAY_MATRIX_DATAPTR(R);
	for (i=0; i < r_width; i++)
	{
		if (i < x_width)
			memcpy(dst, src, typlen * x_height);
		else
			memset(dst, 0, typlen * x_height);
		dst += typlen * r_height;
		src += typlen * x_height;
	}

	/* copy from the bottom-matrix */
	src = ARRAY_MATRIX_DATAPTR(Y);
	dst = ARRAY_MATRIX_DATAPTR(R) + typlen * x_height;
	for (i=0; i < r_width; i++)
	{
		if (i < y_width)
			memcpy(dst, src, typlen * y_height);
		else
			memset(dst, 0, typlen * y_height);
		dst += typlen * r_height;
		src += typlen * y_height;
	}
	return R;
}

Datum
array_matrix_rbind_bool(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_rbind(BOOLOID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_bool);

Datum
array_matrix_rbind_int2(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_rbind(INT2OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_int2);

Datum
array_matrix_rbind_int4(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_rbind(INT4OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_int4);

Datum
array_matrix_rbind_int8(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_rbind(INT8OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_int8);

Datum
array_matrix_rbind_float4(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_rbind(FLOAT4OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_float4);

Datum
array_matrix_rbind_float8(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_rbind(FLOAT8OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_float8);

Datum
array_matrix_rbind_scalar_boolt(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	bool	   *v, scalar = PG_GETARG_INT16(0);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(BOOLOID, width, 1);
	v = (bool *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(BOOLOID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_boolt);

Datum
array_matrix_rbind_scalar_boolb(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	bool	   *v, scalar = PG_GETARG_BOOL(1);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(BOOLOID, width, 1);
	v = (bool *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(BOOLOID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_boolb);

Datum
array_matrix_rbind_scalar_int2t(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	int16	   *v, scalar = PG_GETARG_INT16(0);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(INT2OID, width, 1);
	v = (int16 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(INT2OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_int2t);

Datum
array_matrix_rbind_scalar_int2b(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	int16	   *v, scalar = PG_GETARG_INT16(1);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(INT2OID, width, 1);
	v = (int16 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(INT2OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_int2b);

Datum
array_matrix_rbind_scalar_int4t(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	int32	   *v, scalar = PG_GETARG_INT32(0);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(INT4OID, width, 1);
	v = (int32 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(INT4OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_int4t);

Datum
array_matrix_rbind_scalar_int4b(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	int32	   *v, scalar = PG_GETARG_INT32(1);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(INT4OID, width, 1);
	v = (int32 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(INT4OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_int4b);

Datum
array_matrix_rbind_scalar_int8t(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	int64	   *v, scalar = PG_GETARG_INT64(0);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(INT8OID, width, 1);
	v = (int64 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(INT8OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_int8t);

Datum
array_matrix_rbind_scalar_int8b(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	int64	   *v, scalar = PG_GETARG_INT64(1);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(INT8OID, width, 1);
	v = (int64 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(INT8OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_int8b);

Datum
array_matrix_rbind_scalar_float4t(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	float4	   *v, scalar = PG_GETARG_FLOAT4(0);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(FLOAT4OID, width, 1);
	v = (float4 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(FLOAT4OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_float4t);

Datum
array_matrix_rbind_scalar_float4b(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	float4	   *v, scalar = PG_GETARG_FLOAT4(1);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(FLOAT4OID, width, 1);
	v = (float4 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(FLOAT4OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_float4b);

Datum
array_matrix_rbind_scalar_float8t(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	float8	   *v, scalar = PG_GETARG_FLOAT8(0);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(FLOAT8OID, width, 1);
	v = (float8 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(FLOAT8OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_float8t);

Datum
array_matrix_rbind_scalar_float8b(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	float8	   *v, scalar = PG_GETARG_FLOAT8(1);
	int			i, width = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(FLOAT8OID, width, 1);
	v = (float8 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < width; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_rbind(FLOAT8OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_scalar_float8b);

/*
 * cbind that takes two arrays
 */
static MatrixType *
array_martix_cbind(Oid elemtype, MatrixType *X, MatrixType *Y)
{
	MatrixType *R;
	cl_uint		r_height, x_height, y_height;
	cl_uint		r_width, x_width, y_width;
	int			typlen;
	Size		length;
	int			i;
	char	   *src, *dst;

	/* sanity checks */
	if (VARATT_IS_EXPANDED_HEADER(X) || VARATT_IS_EXPANDED_HEADER(Y))
		elog(ERROR, "ExpandedArrayHeader is not supported");
	if (!VALIDATE_ARRAY_MATRIX(X) || !VALIDATE_ARRAY_MATRIX(Y))
		elog(ERROR, "Not a matrix-like array");
	if (elemtype != ARRAY_MATRIX_ELEMTYPE(X) ||
		elemtype != ARRAY_MATRIX_ELEMTYPE(Y))
		elog(ERROR, "Bug? not expected type");
	typlen = get_typlen(elemtype);

	x_width = ARRAY_MATRIX_WIDTH(X);
	y_width = ARRAY_MATRIX_WIDTH(Y);
	r_width = x_width + y_width;
	x_height = ARRAY_MATRIX_HEIGHT(X);
	y_height = ARRAY_MATRIX_HEIGHT(Y);
	r_height = Max(x_height, y_height);
	length = ARRAY_MATRIX_RAWSIZE(typlen, r_height, r_width);
	R = palloc(length);
	INIT_ARRAY_MATRIX(R, elemtype, typlen, r_height, r_width);

	src = ARRAY_MATRIX_DATAPTR(X);
	dst = ARRAY_MATRIX_DATAPTR(R);
	for (i=0; i < x_width; i++)
	{
		memcpy(dst, src, typlen * x_height);
		if (x_height < r_height)
			memset(dst + typlen * x_height, 0, typlen * (r_height - x_height));
		src += typlen * x_height;
		dst += typlen * r_height;
	}

	src = ARRAY_MATRIX_DATAPTR(Y);
	dst = ARRAY_MATRIX_DATAPTR(R) + typlen * x_width * r_height;
	for (i=0; i < y_width; i++)
	{
		memcpy(dst, src, typlen * y_height);
		if (y_height < r_height)
			memset(dst + typlen * y_height, 0, typlen * (r_height - y_height));
		src += typlen * y_height;
		dst += typlen * r_height;
	}
	return R;
}

Datum
array_matrix_cbind_bool(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_cbind(BOOLOID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_bool);

Datum
array_matrix_cbind_int2(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT2OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_int2);

Datum
array_matrix_cbind_int4(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT4OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_int4);

Datum
array_matrix_cbind_int8(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT8OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_int8);

Datum
array_matrix_cbind_float4(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_cbind(FLOAT4OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_float4);

Datum
array_matrix_cbind_float8(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_DATUM(PG_GETARG_DATUM(1));
	else if (PG_ARGISNULL(1))
		PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	else
	{
		MatrixType	   *X = PG_GETARG_MATRIXTYPE_P(0);
		MatrixType	   *Y = PG_GETARG_MATRIXTYPE_P(1);
		PG_RETURN_MATRIXTYPE_P(array_martix_cbind(FLOAT8OID, X, Y));
	}
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_float8);

Datum
array_matrix_cbind_scalar_booll(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	bool	   *v, scalar = PG_GETARG_BOOL(0);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(BOOLOID, 1, height);
	v = (bool *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(BOOLOID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_booll);

Datum
array_matrix_cbind_scalar_boolr(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	bool	   *v, scalar = PG_GETARG_BOOL(1);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(BOOLOID, 1, height);
	v = (bool *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(BOOLOID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_boolr);

Datum
array_matrix_cbind_scalar_int2l(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	int16	   *v, scalar = PG_GETARG_INT16(0);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(INT2OID, 1, height);
	v = (int16 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT2OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_int2l);

Datum
array_matrix_cbind_scalar_int2r(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	int16	   *v, scalar = PG_GETARG_INT16(1);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(INT2OID, 1, height);
	v = (int16 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT2OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_int2r);

Datum
array_matrix_cbind_scalar_int4l(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	int32	   *v, scalar = PG_GETARG_INT32(0);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(INT4OID, 1, height);
	v = (int32 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT4OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_int4l);

Datum
array_matrix_cbind_scalar_int4r(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	int32	   *v, scalar = PG_GETARG_INT32(1);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(INT4OID, 1, height);
	v = (int32 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT4OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_int4r);

Datum
array_matrix_cbind_scalar_int8l(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	int64	   *v, scalar = PG_GETARG_INT64(0);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(INT8OID, 1, height);
	v = (int64 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT8OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_int8l);

Datum
array_matrix_cbind_scalar_int8r(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	int64	   *v, scalar = PG_GETARG_INT64(1);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(INT8OID, 1, height);
	v = (int64 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT8OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_int8r);

Datum
array_matrix_cbind_scalar_float4l(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	float4	   *v, scalar = PG_GETARG_FLOAT4(0);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(FLOAT4OID, 1, height);
	v = (float4 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(FLOAT4OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_float4l);

Datum
array_matrix_cbind_scalar_float4r(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	float4	   *v, scalar = PG_GETARG_FLOAT4(1);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(FLOAT4OID, 1, height);
	v = (float4 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(FLOAT4OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_float4r);

Datum
array_matrix_cbind_scalar_float8l(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(1);
	MatrixType *S;
	float8	   *v, scalar = PG_GETARG_FLOAT8(0);
	int			i, height = ARRAY_MATRIX_WIDTH(M);

	S = create_empty_matrix(FLOAT8OID, 1, height);
	v = (float8 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(FLOAT8OID,S,M));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_float8l);

Datum
array_matrix_cbind_scalar_float8r(PG_FUNCTION_ARGS)
{
	MatrixType *M = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *S;
	float8	   *v, scalar = PG_GETARG_FLOAT8(1);
	int			i, height = ARRAY_MATRIX_HEIGHT(M);

	S = create_empty_matrix(INT8OID, 1, height);
	v = (float8 *)ARRAY_MATRIX_DATAPTR(S);
	for (i=0; i < height; i++)
		v[i] = scalar;
	PG_RETURN_MATRIXTYPE_P(array_martix_cbind(INT8OID,M,S));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_scalar_float8r);

/*
 * rbind as aggregate function
 */
typedef struct
{
	Oid		elemtype;
	Size	width;
	Size	height;
	List   *matrix_list;
} matrix_rbind_state;

Datum
array_matrix_rbind_accum(PG_FUNCTION_ARGS)
{
	matrix_rbind_state *mrstate;
	MemoryContext	aggcxt;
	MemoryContext	oldcxt;
	MatrixType	   *X;

	if (!AggCheckCallContext(fcinfo, &aggcxt))
		elog(ERROR, "aggregate function called in non-aggregate context");

	if (PG_ARGISNULL(1))
		elog(ERROR, "null-array was supplied");

	oldcxt = MemoryContextSwitchTo(aggcxt);
    X = PG_GETARG_MATRIXTYPE_P_COPY(1);
	if (!VALIDATE_ARRAY_MATRIX(X))
		elog(ERROR, "input array is not a valid matrix-like array");

	if (PG_ARGISNULL(0))
	{
		mrstate = palloc0(sizeof(matrix_rbind_state));
		mrstate->elemtype = ARRAY_MATRIX_ELEMTYPE(X);
	}
	else
	{
		mrstate = (matrix_rbind_state *)PG_GETARG_POINTER(0);
		if (mrstate->elemtype != ARRAY_MATRIX_ELEMTYPE(X))
			elog(ERROR, "element type of input array mismatch '%s' for '%s'",
				 format_type_be(ARRAY_MATRIX_ELEMTYPE(X)),
				 format_type_be(mrstate->elemtype));
	}

	mrstate->width = Max(mrstate->width, ARRAY_MATRIX_WIDTH(X));
	mrstate->height += ARRAY_MATRIX_HEIGHT(X);
	mrstate->matrix_list = lappend(mrstate->matrix_list, X);

	MemoryContextSwitchTo(oldcxt);

	PG_RETURN_POINTER(mrstate);
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_accum);

static MatrixType *
array_matrix_rbind_final(matrix_rbind_state *mrstate)
{
	MatrixType *R;
	int			typlen;
	Size		height = mrstate->height;
	Size		width = mrstate->width;
	Size		length;
	Size		row_index;
	char	   *src, *dst;
	ListCell   *lc;

	switch (mrstate->elemtype)
	{
		case BOOLOID:
			typlen = sizeof(cl_char);
			break;
		case INT2OID:
			typlen = sizeof(cl_short);
			break;
		case INT4OID:
		case FLOAT4OID:
			typlen = sizeof(cl_int);
			break;
		case INT8OID:
		case FLOAT8OID:
			typlen = sizeof(cl_long);
			break;
		default:
			elog(ERROR, "unsupported element type: %s",
				 format_type_be(mrstate->elemtype));
	}
	length = ARRAY_MATRIX_RAWSIZE(typlen, height, width);
	if (!AllocSizeIsValid(length))
		elog(ERROR, "supplied array-matrix is too big");
	R = palloc(length);
	INIT_ARRAY_MATRIX(R, mrstate->elemtype, typlen, height, width);

	row_index = 0;
	foreach (lc, mrstate->matrix_list)
	{
		MatrixType *X = lfirst(lc);
		cl_int		x_width = ARRAY_MATRIX_WIDTH(X);
		cl_int		x_height = ARRAY_MATRIX_HEIGHT(X);
		cl_int		i;

		Assert(VALIDATE_ARRAY_MATRIX(X));
		src = ARRAY_MATRIX_DATAPTR(X);
		dst = ARRAY_MATRIX_DATAPTR(R) + typlen * row_index;
		for (i=0; i < width; i++)
		{
			if (i < x_width)
				memcpy(dst, src, typlen * x_height);
			else
				memset(dst, 0, typlen * x_height);
			src += typlen * x_height;
			dst += typlen * height;
		}
		row_index += x_height;
	}
	return R;
}

Datum
array_matrix_rbind_final_bool(PG_FUNCTION_ARGS)
{
	matrix_rbind_state *mrstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mrstate = (matrix_rbind_state *)PG_GETARG_POINTER(0);
	Assert(mrstate->elemtype == BOOLOID);
	PG_RETURN_POINTER(array_matrix_rbind_final(mrstate));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_final_bool);

Datum
array_matrix_rbind_final_int2(PG_FUNCTION_ARGS)
{
	matrix_rbind_state *mrstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mrstate = (matrix_rbind_state *)PG_GETARG_POINTER(0);
	Assert(mrstate->elemtype == INT2OID);
	PG_RETURN_POINTER(array_matrix_rbind_final(mrstate));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_final_int2);

Datum
array_matrix_rbind_final_int4(PG_FUNCTION_ARGS)
{
	matrix_rbind_state *mrstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mrstate = (matrix_rbind_state *)PG_GETARG_POINTER(0);
	Assert(mrstate->elemtype == INT4OID);
	PG_RETURN_POINTER(array_matrix_rbind_final(mrstate));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_final_int4);

Datum
array_matrix_rbind_final_int8(PG_FUNCTION_ARGS)
{
	matrix_rbind_state *mrstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mrstate = (matrix_rbind_state *)PG_GETARG_POINTER(0);
	Assert(mrstate->elemtype == INT8OID);
	PG_RETURN_POINTER(array_matrix_rbind_final(mrstate));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_final_int8);

Datum
array_matrix_rbind_final_float4(PG_FUNCTION_ARGS)
{
	matrix_rbind_state *mrstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mrstate = (matrix_rbind_state *)PG_GETARG_POINTER(0);
	Assert(mrstate->elemtype == FLOAT4OID);
	PG_RETURN_POINTER(array_matrix_rbind_final(mrstate));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_final_float4);

Datum
array_matrix_rbind_final_float8(PG_FUNCTION_ARGS)
{
	matrix_rbind_state *mrstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mrstate = (matrix_rbind_state *)PG_GETARG_POINTER(0);
	Assert(mrstate->elemtype == FLOAT8OID);
	PG_RETURN_POINTER(array_matrix_rbind_final(mrstate));
}
PG_FUNCTION_INFO_V1(array_matrix_rbind_final_float8);

/*
 * cbind as aggregate function
 */
typedef struct
{
	Oid		elemtype;
	Size	width;
	Size	height;
	List   *matrix_list;
} matrix_cbind_state;

Datum
array_matrix_cbind_accum(PG_FUNCTION_ARGS)
{
	matrix_cbind_state *mcstate;
	MemoryContext	aggcxt;
	MemoryContext	oldcxt;
	MatrixType	   *X;

	if (!AggCheckCallContext(fcinfo, &aggcxt))
		elog(ERROR, "aggregate function called in non-aggregate context");

	if (PG_ARGISNULL(1))
		elog(ERROR, "null-array was supplied");

	oldcxt = MemoryContextSwitchTo(aggcxt);
	X = PG_GETARG_MATRIXTYPE_P_COPY(1);
	if (!VALIDATE_ARRAY_MATRIX(X))
		elog(ERROR, "input array is not a valid matrix-like array");

	if (PG_ARGISNULL(0))
	{
		mcstate = palloc0(sizeof(matrix_cbind_state));
		mcstate->elemtype = ARRAY_MATRIX_ELEMTYPE(X);
	}
	else
	{
		mcstate = (matrix_cbind_state *)PG_GETARG_POINTER(0);
		if (mcstate->elemtype != ARRAY_MATRIX_ELEMTYPE(X))
			elog(ERROR, "element type of input array mismatch '%s' for '%s'",
				 format_type_be(ARRAY_MATRIX_ELEMTYPE(X)),
				 format_type_be(mcstate->elemtype));
	}
	mcstate->width += ARRAY_MATRIX_WIDTH(X);
	mcstate->height = Max(mcstate->height, ARRAY_MATRIX_HEIGHT(X));
	mcstate->matrix_list = lappend(mcstate->matrix_list, X);

	MemoryContextSwitchTo(oldcxt);

	PG_RETURN_POINTER(mcstate);
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_accum);

static MatrixType *
array_matrix_cbind_final(matrix_cbind_state *mcstate)
{
	MatrixType *R;
	int			typlen;
	Size		height = mcstate->height;
	Size		width = mcstate->width;
	Size		length;
	ListCell   *lc;
	char	   *src, *dst;

	switch (mcstate->elemtype)
	{
		case BOOLOID:
			typlen = sizeof(cl_char);
			break;
		case INT2OID:
			typlen = sizeof(cl_short);
			break;
		case INT4OID:
		case FLOAT4OID:
			typlen = sizeof(cl_int);
			break;
		case INT8OID:
		case FLOAT8OID:
			typlen = sizeof(cl_long);
			break;
		default:
			elog(ERROR, "unsupported element type: %s",
				 format_type_be(mcstate->elemtype));
    }
	length = ARRAY_MATRIX_RAWSIZE(typlen, height, width);
	if (!AllocSizeIsValid(length))
		elog(ERROR, "supplied array-matrix is too big");
	R = palloc(length);
	INIT_ARRAY_MATRIX(R, mcstate->elemtype, typlen, height, width);

	dst = ARRAY_MATRIX_DATAPTR(R);
	foreach (lc, mcstate->matrix_list)
	{
		MatrixType *X = lfirst(lc);
		cl_int		x_width = ARRAY_MATRIX_WIDTH(X);
		cl_int		x_height = ARRAY_MATRIX_HEIGHT(X);
		cl_uint		i;

		Assert(VALIDATE_ARRAY_MATRIX(X));
		src = ARRAY_MATRIX_DATAPTR(X);
		for (i=0; i < x_width; i++)
		{
			memcpy(dst, src, typlen * x_height);
			if (x_height < height)
				memset(dst + typlen * x_height, 0,
					   typlen * (height - x_height));
			src += typlen * x_height;
			dst += typlen * height;
		}
	}
	return R;
}

Datum
array_matrix_cbind_final_bool(PG_FUNCTION_ARGS)
{
	matrix_cbind_state *mcstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mcstate = (matrix_cbind_state *)PG_GETARG_POINTER(0);
	Assert(mcstate->elemtype == BOOLOID);
	PG_RETURN_POINTER(array_matrix_cbind_final(mcstate));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_final_bool);

Datum
array_matrix_cbind_final_int2(PG_FUNCTION_ARGS)
{
	matrix_cbind_state *mcstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mcstate = (matrix_cbind_state *)PG_GETARG_POINTER(0);
	Assert(mcstate->elemtype == INT2OID);
	PG_RETURN_POINTER(array_matrix_cbind_final(mcstate));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_final_int2);

Datum
array_matrix_cbind_final_int4(PG_FUNCTION_ARGS)
{
	matrix_cbind_state *mcstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mcstate = (matrix_cbind_state *)PG_GETARG_POINTER(0);
	Assert(mcstate->elemtype == INT4OID);
	PG_RETURN_POINTER(array_matrix_cbind_final(mcstate));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_final_int4);

Datum
array_matrix_cbind_final_int8(PG_FUNCTION_ARGS)
{
	matrix_cbind_state *mcstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mcstate = (matrix_cbind_state *)PG_GETARG_POINTER(0);
	Assert(mcstate->elemtype == INT8OID);
	PG_RETURN_POINTER(array_matrix_cbind_final(mcstate));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_final_int8);

Datum
array_matrix_cbind_final_float4(PG_FUNCTION_ARGS)
{
	matrix_cbind_state *mcstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mcstate = (matrix_cbind_state *)PG_GETARG_POINTER(0);
	Assert(mcstate->elemtype == FLOAT4OID);
	PG_RETURN_POINTER(array_matrix_cbind_final(mcstate));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_final_float4);

Datum
array_matrix_cbind_final_float8(PG_FUNCTION_ARGS)
{
	matrix_cbind_state *mcstate;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	mcstate = (matrix_cbind_state *)PG_GETARG_POINTER(0);
	Assert(mcstate->elemtype == FLOAT8OID);
	PG_RETURN_POINTER(array_matrix_cbind_final(mcstate));
}
PG_FUNCTION_INFO_V1(array_matrix_cbind_final_float8);

/*
 * matrix_transpose
 */
#define ARRAY_MATRIX_TRANSPOSE_TEMPLATE(T,M,BASETYPE)					\
	do {																\
		Size	height = ARRAY_MATRIX_HEIGHT(M);						\
		Size	width = ARRAY_MATRIX_WIDTH(M);							\
		Size	i, nitems = width * height;								\
		Size	length;													\
		char   *T_values;												\
		char   *M_values = ARRAY_MATRIX_DATAPTR(M);						\
																		\
		length = ARRAY_MATRIX_RAWSIZE(sizeof(BASETYPE), height, width);	\
		if (!AllocSizeIsValid(length))									\
			elog(ERROR, "matrix array size too large");					\
		T = palloc(length);												\
		INIT_ARRAY_MATRIX(T, ARRAY_MATRIX_ELEMTYPE(M),					\
						  sizeof(BASETYPE), width, height);				\
		T_values = ARRAY_MATRIX_DATAPTR(T);								\
		for (i=0; i < nitems; i++)										\
		{																\
			*((BASETYPE *)(T_values + sizeof(BASETYPE) *				\
						   ((i % height) * width + (i / height)))) =	\
				*((BASETYPE *)(M_values + sizeof(BASETYPE) * i));		\
		}																\
	} while(0)

Datum
array_matrix_transpose_bool(PG_FUNCTION_ARGS)
{
	MatrixType *matrix = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *result;

	if (VARATT_IS_EXPANDED_HEADER(matrix) ||
		!VALIDATE_ARRAY_MATRIX(matrix))
		elog(ERROR, "Array is not like Matrix");
	Assert(matrix->elemtype == BOOLOID);
	ARRAY_MATRIX_TRANSPOSE_TEMPLATE(result,matrix,cl_uchar);
	PG_RETURN_POINTER(result);
}
PG_FUNCTION_INFO_V1(array_matrix_transpose_bool);

Datum
array_matrix_transpose_int2(PG_FUNCTION_ARGS)
{
	MatrixType *matrix = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *result;

	if (VARATT_IS_EXPANDED_HEADER(matrix) ||
		!VALIDATE_ARRAY_MATRIX(matrix))
		elog(ERROR, "Array is not like Matrix");
	Assert(matrix->elemtype == INT2OID);
	ARRAY_MATRIX_TRANSPOSE_TEMPLATE(result,matrix,cl_ushort);
	PG_RETURN_POINTER(result);
}
PG_FUNCTION_INFO_V1(array_matrix_transpose_int2);

Datum
array_matrix_transpose_int4(PG_FUNCTION_ARGS)
{
	MatrixType *matrix = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *result;

	if (VARATT_IS_EXPANDED_HEADER(matrix) ||
		!VALIDATE_ARRAY_MATRIX(matrix))
		elog(ERROR, "Array is not like Matrix");
	Assert(matrix->elemtype == INT4OID);
	ARRAY_MATRIX_TRANSPOSE_TEMPLATE(result,matrix,cl_ushort);
	PG_RETURN_POINTER(result);
}
PG_FUNCTION_INFO_V1(array_matrix_transpose_int4);

Datum
array_matrix_transpose_int8(PG_FUNCTION_ARGS)
{
	MatrixType *matrix = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *result;

	if (VARATT_IS_EXPANDED_HEADER(matrix) ||
		!VALIDATE_ARRAY_MATRIX(matrix))
		elog(ERROR, "Array is not like Matrix");
	Assert(matrix->elemtype == INT8OID);
	ARRAY_MATRIX_TRANSPOSE_TEMPLATE(result,matrix,cl_ushort);
	PG_RETURN_POINTER(result);
}
PG_FUNCTION_INFO_V1(array_matrix_transpose_int8);

Datum
array_matrix_transpose_float4(PG_FUNCTION_ARGS)
{
	MatrixType *matrix = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *result;

	if (VARATT_IS_EXPANDED_HEADER(matrix) ||
		!VALIDATE_ARRAY_MATRIX(matrix))
		elog(ERROR, "Array is not like Matrix");
	Assert(matrix->elemtype == FLOAT4OID);
	ARRAY_MATRIX_TRANSPOSE_TEMPLATE(result,matrix,cl_ushort);
	PG_RETURN_POINTER(result);
}
PG_FUNCTION_INFO_V1(array_matrix_transpose_float4);

Datum
array_matrix_transpose_float8(PG_FUNCTION_ARGS)
{
	MatrixType *matrix = PG_GETARG_MATRIXTYPE_P(0);
	MatrixType *result;

	if (VARATT_IS_EXPANDED_HEADER(matrix) ||
		!VALIDATE_ARRAY_MATRIX(matrix))
		elog(ERROR, "Array is not like Matrix");
	Assert(matrix->elemtype == FLOAT8OID);
	ARRAY_MATRIX_TRANSPOSE_TEMPLATE(result,matrix,cl_ushort);
	PG_RETURN_POINTER(result);
}
PG_FUNCTION_INFO_V1(array_matrix_transpose_float8);

/*
 * postgresql_type_rawsize on behalf of type_len(regtype)
 */
Datum
postgresql_type_rawsize(PG_FUNCTION_ARGS)
{
	Oid			type_oid = PG_GETARG_OID(0);
	HeapTuple	tup;
	int64		type_len;

	tup = SearchSysCache1(TYPEOID, ObjectIdGetDatum(type_oid));
	if (!HeapTupleIsValid(tup))
		elog(ERROR, "cache lookup failed for type %u", type_oid);
	type_len = ((Form_pg_type) GETSTRUCT(tup))->typlen;
	ReleaseSysCache(tup);

	PG_RETURN_INT64(type_len);
}
PG_FUNCTION_INFO_V1(postgresql_type_rawsize);

/*
 * composite_type_rawsize - estimator of composite data type
 *
 * note: it returns raw-size with safety margin due to lack of alignment
 * information, so actual composite record may be smaller than estimation.
 */
Datum
composite_type_rawsize(PG_FUNCTION_ARGS)
{
	ArrayType	   *a = PG_GETARG_ARRAYTYPE_P(0);
	ArrayIterator	aiter;
	Size			t_hoff;
	ssize_t			attlen;
	cl_int			attalign;
	Datum			datum;
	bool			isnull;

	if ((ARR_ELEMTYPE(a) != INT4OID && ARR_ELEMTYPE(a) != INT8OID) ||
		ARR_NDIM(a) != 1 ||
		ARR_LBOUND(a)[0] != 1)
		elog(ERROR, "array of sub-attributes size is not valid");

	t_hoff = offsetof(HeapTupleHeaderData, t_bits);
	if (ARR_HASNULL(a))
		t_hoff += BITMAPLEN(ARR_DIMS(a)[0]);
	/* NOTE: composite type should never have OID */
	t_hoff = MAXALIGN(t_hoff);

	aiter = array_create_iterator(a, 0, NULL);
	while (array_iterate(aiter, &datum, &isnull))
    {
		if (isnull)
			continue;
		attlen = (Size)(ARR_ELEMTYPE(a) == INT4OID
						? DatumGetInt32(datum)
						: DatumGetInt64(datum));
		if (attlen < 0)
			elog(ERROR, "negative type length is not valid - actual size should be supplied for varlena");
		if (attlen <= sizeof(cl_char))
			attalign = sizeof(cl_char);
		else if (attlen <= sizeof(cl_short))
			attalign = sizeof(cl_short);
		else if (attlen <= sizeof(cl_int))
			attalign = sizeof(cl_int);
		else
			attalign = sizeof(cl_long);

		t_hoff = TYPEALIGN(attalign, t_hoff);
		t_hoff += attlen;
	}
	array_free_iterator(aiter);

	PG_RETURN_INT64(MAXALIGN(t_hoff));
}
PG_FUNCTION_INFO_V1(composite_type_rawsize);

/*
 * float4_as_int4, int4_as_float4
 * float8_as_int8, int8_as_float8
 *
 * Re-interpretation of integer/floating-point values.
 * When we want to back a pair of integer + floating-point, because of the
 * characteristict of matrix, either of them have to be packed to others.
 * However, simple cast may make problem because FP32 has only 22-bit for
 * mantissa; it is not sufficient to pack ID value more than 4M.
 * Usual type cast makes problem for these values. So, we provide several
 * type re-interpretation routines as CUDA doing.
 */
Datum
float4_as_int4(PG_FUNCTION_ARGS)
{
	PG_RETURN_DATUM(PG_GETARG_DATUM(0) & 0xffffffffU);
}
PG_FUNCTION_INFO_V1(float4_as_int4);

Datum
int4_as_float4(PG_FUNCTION_ARGS)
{
	PG_RETURN_DATUM(PG_GETARG_DATUM(0) & 0xffffffffU);
}
PG_FUNCTION_INFO_V1(int4_as_float4);

Datum
float8_as_int8(PG_FUNCTION_ARGS)
{
	PG_RETURN_DATUM(PG_GETARG_DATUM(0));
}
PG_FUNCTION_INFO_V1(float8_as_int8);

Datum
int8_as_float8(PG_FUNCTION_ARGS)
{
	PG_RETURN_DATUM(PG_GETARG_DATUM(0));
}
PG_FUNCTION_INFO_V1(int8_as_float8);
